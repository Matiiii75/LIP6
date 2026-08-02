#!/usr/bin/env python3
"""
gen_graph.py
============

Adaptation Python de gen_graph.cpp : génère un DAG à `n` sommets AU TOTAL
dont le graphe de co-comparabilité H_G (sur les noeuds internes) a une 
dégénérescence d(H_G) proche de `k`. Le graphe final inclut un sommet source 
(prédécesseur de tout sommet à degré entrant nul) et un sommet puits 
(successeur de tout sommet à degré sortant nul), puis renvoie / écrit le résultat.

Contrairement au binaire C++ (qui prend directement une probabilité p),
ce script prend en entrée la dégénérescence CIBLE k et calcule p via le
modèle appris (régression logit-polynomiale, mêmes coefficients que dans
generate_dag.py / le dépôt du rapport LU3IN400).

Convention de numérotation des sommets (identique au .cpp) :
    0          -> source
    1 .. n-2   -> sommets du DAG "original" généré aléatoirement
    n - 1      -> puits
    (n sommets au total)

Utilisation en ligne de commande, comme le binaire C++ :

    ./gen_graph.py n k ID [max_attempts] [tolerance]

    n             : nombre total de sommets (source et puits INCLUS, doit être >= 3)
    k             : dégénérescence ciblée d(H_G)
    ID            : identifiant de l'instance (utilisé dans le nom de
                    fichier ET pour dériver une graine aléatoire
                    reproductible : même (k, n, ID) => même instance)
    max_attempts  : optionnel, défaut 30 (voir generate_dag)
    tolerance     : optionnel, défaut 0.0 (0 = on vise l'égalité exacte)

Utilisation en tant que module :

    from gen_graph import generate_instance
    adj, d_obtenue = generate_instance(n=1000, k=100, ID="1")
    # adj est une list[list[int]] : adj[i] = successeurs de i
"""

from __future__ import annotations

import hashlib
import math
import os
import random
import sys

import networkx as nx


# ---------------------------------------------------------------------------
# 1. Génération du DAG interne + co-comparabilité + dégénérescence
#    (équivalent des méthodes Dag::gen_graph / compute_HG / compute_degenerascy,
#     mais sur le DAG "original", sans source ni puits)
# ---------------------------------------------------------------------------

def dag_generator(n: int, p: float, rng: random.Random | None = None) -> nx.DiGraph:
    """DAG aléatoire à n sommets (0..n-1) : arc (i, j) pour i < j avec proba p."""
    if n <= 0:
        raise ValueError("Le nombre de sommets internes doit être strictement positif.")
    if not (0.0 <= p <= 1.0):
        raise ValueError("p doit être entre 0 et 1.")
    rng = rng or random
    G = nx.DiGraph()
    G.add_nodes_from(range(n))
    for i in range(n):
        for j in range(i + 1, n):
            if rng.random() < p:
                G.add_edge(i, j)
    return G


def get_co_comp(G: nx.DiGraph) -> nx.Graph:
    """H_G : deux sommets reliés s'ils sont incomparables dans G."""
    TC = nx.transitive_closure_dag(G)
    H = nx.Graph()
    H.add_nodes_from(G.nodes())
    nodes = list(G.nodes())
    for i in range(len(nodes)):
        a = nodes[i]
        for j in range(i + 1, len(nodes)):
            b = nodes[j]
            if not TC.has_edge(a, b) and not TC.has_edge(b, a):
                H.add_edge(a, b)
    return H


def degeneracy(H: nx.Graph) -> int:
    """d(H) via core_number (équivalent Dag::compute_degenerascy, même algo de peeling)."""
    core = nx.core_number(H)
    return max(core.values(), default=0)


def measure_degeneracy(G: nx.DiGraph) -> int:
    return degeneracy(get_co_comp(G))


# ---------------------------------------------------------------------------
# 2. Modèle p -> d(H_G)  (coefficients repris du dépôt GitHub du rapport,
#    src/graph_lib.py -- absents du PDF, c'est la pièce qui manquait côté C++)
#    Domaine validé par les auteurs : n dans [~10, 2000], p dans [0.01, 0.9]
# ---------------------------------------------------------------------------

DEGENERACY_MODEL = {
    "intercept": -18.54393130910309,
    "terms": [
        ("log_n", -2.808983437617303), ("log_p", -0.35778675067172344),
        ("sqrt_p", -1.6139494087235673), ("log_n^2", 1.0032858583798732),
        ("log_n log_p", -0.24447736952782995), ("log_n sqrt_p", -6.153019126599521),
        ("log_p^2", -0.0267841698293633), ("log_p sqrt_p", 6.6576050858431035),
        ("sqrt_p^2", -0.02907046692824014), ("log_n^3", -0.19840665461576187),
        ("log_n^2 log_p", -0.14335549595212982), ("log_n^2 sqrt_p", 0.28223638994903916),
        ("log_n log_p^2", -0.09011916495932704), ("log_n log_p sqrt_p", 1.816122959282859),
        ("log_n sqrt_p^2", -8.028373370697166), ("log_p^3", 0.07728246976505715),
        ("log_p^2 sqrt_p", 0.9695459032896236), ("log_p sqrt_p^2", 27.83989664726159),
        ("sqrt_p^3", 1.6662223358452755), ("log_n^4", -0.00795150061928675),
        ("log_n^3 log_p", -0.10952008402316046), ("log_n^3 sqrt_p", 0.44518542314483506),
        ("log_n^2 log_p^2", -0.2691329506586916), ("log_n^2 log_p sqrt_p", 1.8100423984538652),
        ("log_n^2 sqrt_p^2", -4.05322181422922), ("log_n log_p^3", -0.26270872624811586),
        ("log_n log_p^2 sqrt_p", 2.2419145794440336), ("log_n log_p sqrt_p^2", -19.26181026094775),
        ("log_n sqrt_p^3", 29.632826973831325), ("log_p^4", -0.07863279621903092),
        ("log_p^3 sqrt_p", -1.0898642436042683), ("log_p^2 sqrt_p^2", 32.29655789069866),
        ("log_p sqrt_p^3", -64.31508740156623), ("sqrt_p^4", -6.770719774137262),
    ],
}


def _term_value(term: str, values: dict) -> float:
    result = 1.0
    for factor in term.split(" "):
        if "^" in factor:
            name, power = factor.split("^")
            result *= values[name] ** int(power)
        else:
            result *= values[factor]
    return result


def _sigmoid(x: float) -> float:
    if x >= 0:
        return 1.0 / (1.0 + math.exp(-x))
    ex = math.exp(x)
    return ex / (1.0 + ex)


def predict_degeneracy(n: int, p: float) -> float:
    values = {"log_n": math.log(n), "log_p": math.log(p), "sqrt_p": math.sqrt(p)}
    z = DEGENERACY_MODEL["intercept"]
    for term, coef in DEGENERACY_MODEL["terms"]:
        z += coef * _term_value(term, values)
    return n * _sigmoid(z)


_P_MIN, _P_MAX = 0.01, 0.9  # bornes du domaine sur lequel le modèle a été ajusté


def choose_p_for_degeneracy(k: int, n: int, verbose: bool = True) -> tuple[float, float]:
    """Cherche p tel que predict_degeneracy(n, p) ~= k (recherche ternaire sur log p,
    valide car d(H_G) est monotone décroissante en p)."""
    lo, hi = math.log(_P_MIN), math.log(_P_MAX)
    for _ in range(80):
        m1 = lo + (hi - lo) / 3
        m2 = hi - (hi - lo) / 3
        e1 = abs(predict_degeneracy(n, math.exp(m1)) - k)
        e2 = abs(predict_degeneracy(n, math.exp(m2)) - k)
        if e1 <= e2:
            hi = m2
        else:
            lo = m1
    p = math.exp((lo + hi) / 2)
    predicted = predict_degeneracy(n, p)

    if verbose and (p <= _P_MIN * 1.01 or p >= _P_MAX * 0.99):
        print(
            f"[avertissement] p={p:.4f} est sur la borne du domaine validé "
            f"[{_P_MIN}, {_P_MAX}]. k={k} est probablement hors de la zone "
            f"où le modèle a été ajusté (n~[10,2000], p~[0.01,0.9]) : "
            f"la précision n'est plus garantie.",
            file=sys.stderr,
        )
    return p, predicted


# ---------------------------------------------------------------------------
# 3. generate_dag(n, k) -> DAG interne (nx.DiGraph, sommets 0..n-1)
#    équivalent Dag::gen_graph, mais avec p choisi automatiquement pour viser k
# ---------------------------------------------------------------------------

def generate_dag(
    n_internal: int,
    k: int,
    *,
    max_attempts: int = 30,
    tolerance: float = 0.0,
    seed: int | None = None,
    verbose: bool = True,
) -> nx.DiGraph:
    """DAG à n_internal sommets (0..n_internal-1) dont d(H_G) est aussi proche que possible de k."""
    if n_internal <= 0:
        raise ValueError("n_internal doit être strictement positif.")
    if k <= 0:
        raise ValueError("k doit être strictement positif.")
    if k >= n_internal:
        raise ValueError(f"k={k} >= n_internal={n_internal} est impossible (un sommet a au plus n-1 voisins).")

    p, predicted = choose_p_for_degeneracy(k, n_internal, verbose=verbose)
    rng = random.Random(seed) if seed is not None else random

    best_G, best_value, best_error = None, None, None
    attempt = 0
    for attempt in range(1, max_attempts + 1):
        G = dag_generator(n_internal, p, rng=rng)
        value = measure_degeneracy(G)
        error = abs(value - k) / k
        if best_error is None or error < best_error:
            best_G, best_value, best_error = G, value, error
        if error <= tolerance:
            break

    if verbose:
        print(
            f"[interne] cible k={k} | obtenu d(H_G)={best_value} | "
            f"prédiction modèle={predicted:.2f} | p={p:.5f} | "
            f"essais={attempt}/{max_attempts} | erreur relative={best_error:.1%}",
            file=sys.stderr,
        )
    return best_G


# ---------------------------------------------------------------------------
# 4. Ajout source / puits + conversion en liste d'adjacence
#    (équivalent de la 2e partie de Dag::gen_graph dans le .cpp)
# ---------------------------------------------------------------------------

def add_source_sink(G: nx.DiGraph) -> list[list[int]]:
    """
    G : DAG interne, sommets 0..n_internal-1.

    Construit le DAG final à n_internal+2 sommets :
      0              -> source, prédécesseur de tout sommet de G à in-degré nul
      1..n_internal  -> sommets de G, décalés de +1
      n_internal+1   -> puits, successeur de tout sommet de G à out-degré nul

    Renvoie une liste d'adjacence adj (list[list[int]]) : adj[i] = liste
    triée des successeurs de i dans le DAG final.
    """
    n_internal = G.number_of_nodes()
    total = n_internal + 2
    source, puit = 0, n_internal + 1
    adj: list[list[int]] = [[] for _ in range(total)]

    for u, v in G.edges():
        adj[u + 1].append(v + 1)

    for u in range(n_internal):
        node = u + 1
        if G.in_degree(u) == 0:
            adj[source].append(node)
        if G.out_degree(u) == 0:
            adj[node].append(puit)

    for row in adj:
        row.sort()

    return adj


# ---------------------------------------------------------------------------
# 5. API principale : instance complète (source + puits inclus)
# ---------------------------------------------------------------------------

def generate_instance(
    n: int,
    k: int,
    *,
    ID: str | int | None = None,
    max_attempts: int = 30,
    tolerance: float = 0.0,
    seed: int | None = None,
    verbose: bool = True,
) -> tuple[list[list[int]], int]:
    """
    Génère une instance complète : DAG à n sommets AU TOTAL (dont une source (0) 
    et un puits (n-1)), dont H_G a une dégénérescence ~= k.

    Si `seed` n'est pas fourni et que `ID` l'est, une graine déterministe
    est dérivée de (k, n, ID) : le même triplet (k, n, ID) reproduit
    toujours exactement la même instance.

    Renvoie (adj, d_obtenue) :
      adj        : list[list[int]] de taille n, liste d'adjacence du DAG
                   final (source=0, ..., puits=n-1)
      d_obtenue  : dégénérescence effectivement obtenue (== k dans la
                   quasi-totalité des cas grâce aux essais répétés)
    """
    n_internal = n - 2
    if n_internal <= 0:
        raise ValueError(f"Le graphe doit contenir au moins 3 sommets (source, puits + 1 interne). Reçu n={n}.")

    if seed is None and ID is not None:
        seed = int(hashlib.sha256(f"{k}_{n}_{ID}".encode()).hexdigest()[:8], 16)

    G = generate_dag(n_internal, k, max_attempts=max_attempts, tolerance=tolerance,
                      seed=seed, verbose=verbose)
    d_obtenue = measure_degeneracy(G)
    adj = add_source_sink(G)
    return adj, d_obtenue


# ---------------------------------------------------------------------------
# 6. Écriture fichier (mêmes conventions que Dag::write_in_file)
# ---------------------------------------------------------------------------

def write_instance(adj: list[list[int]], d_obtenue: int, ID: str, output_dir: str = "../instances/test_gen_exact") -> str:
    """
    Écrit l'instance au format :
        nb_nodes nb_arcs degenerescence
        i j        (une ligne par arc i -> j)
    Nom de fichier : "{nb_nodes}_n_{d_obtenue}_k_{ID}_ID.txt" (nb_nodes = total,
    source+puits inclus -- identique à la convention du .cpp).
    """
    n_total = len(adj)
    nb_arcs = sum(len(row) for row in adj)
    filename = f"{n_total}_n_{d_obtenue}_k_{ID}_ID.txt"
    os.makedirs(output_dir, exist_ok=True)
    path = os.path.join(output_dir, filename)
    with open(path, "w") as f:
        f.write(f"{n_total} {nb_arcs} {d_obtenue}\n")
        for i, row in enumerate(adj):
            for j in row:
                f.write(f"{i} {j}\n")
    return path


# ---------------------------------------------------------------------------
# 7. CLI : ./gen_graph.py k n ID [max_attempts] [tolerance]
# ---------------------------------------------------------------------------

def genere_cluster_instances(): 
    """
    cette fonction permet de générer des lots d'instances. 
    on définit un ensemble de tailles du dag, un ensemble de degeneracy visées 
    ainsi qu'un ensemble d'ID (0 à max(ID)) et cela va permettre de générer plusieurs instances
    de même paramètres avec des ID différents. 
    """

    tailles = [50,100,200,500]
    k_degens = [5,10,15,20,25,30,35,40]
    IDs = [i for i in range(1,11)]

    max_attempts = 30
    tolerance = 0.0

    for id in IDs: 
        for n in tailles: 
            for k in k_degens: 

                if k >= n-2: 
                    continue 

                dag, k_obtenu = generate_instance(n, k, ID= id, 
                                           max_attempts=max_attempts, 
                                           tolerance=tolerance, 
                                           verbose=False
                                           )
                path = write_instance(dag, k_obtenu, id)
                print(f"instance {path} générée avec succès !")



def main() -> None:
    if len(sys.argv) < 4:
        print("Usage : ./gen_graph.py k n ID [max_attempts] [tolerance]", file=sys.stderr)
        sys.exit(1)

    n = int(sys.argv[1])
    k = int(sys.argv[2]) 
    ID = sys.argv[3]
    max_attempts = int(sys.argv[4]) if len(sys.argv) > 4 else 30
    tolerance = float(sys.argv[5]) if len(sys.argv) > 5 else 0.0

    adj, d_obtenue = generate_instance(
        n, k, ID=ID, max_attempts=max_attempts, tolerance=tolerance, verbose=True
    )
    path = write_instance(adj, d_obtenue, ID)

    nb_arcs = sum(len(row) for row in adj)
    print("=== instance générée ===")
    print(f"n total (source+puits inclus) : {n}")
    print(f"n (sommets internes)          : {n - 2}")
    print(f"k ciblé                       : {k}")
    print(f"k obtenu                      : {d_obtenue}")
    print(f"nb arcs                       : {nb_arcs}")
    print(f"fichier                       : {path}")
    print("adjacence (list[list[int]])   :")
    print(adj)


if __name__ == "__main__":
    # main() # décommenter si on veut générer une seule instance (avec la commande python3 Dag_generator.py n k ID)
    genere_cluster_instances() # décommenter si on veut générer un lot d'instance dont les paramètres sont définits dans la fonction ci-contre appelée 

