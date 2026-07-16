import pulp
from itertools import product 
from collections import defaultdict

def test_lp(blocages:list[list]):

    # pré calculs de données : 
    
    univers = set()
    for b in blocages: 
        for i in b: 
            univers.add(i)
    
    nb_blocages = [i for i in range(len(blocages))]

    model = pulp.LpProblem("test", pulp.LpMaximize) # création modèle 

    # création variables :
    
    indices_x = list(product(univers,nb_blocages)) # tt les paires (i,j)

    x = pulp.LpVariable.dicts("x", 
                              indices_x, 
                              lowBound=0, 
                              upBound=1, 
                              cat="Binary")

    indices_gamma = list(product(nb_blocages, nb_blocages))

    gamma = pulp.LpVariable.dicts("g", 
                                  indices_gamma, 
                                  lowBound=0, 
                                  upBound=1, 
                                  cat="Binary")
    
    z = pulp.LpVariable.dicts("z", 
                              nb_blocages, 
                              lowBound=0, 
                              cat="Integer")
    
    # Objectif 
    k = len(nb_blocages)
    model += pulp.lpSum(z[(k-1)-l] * (l+1) for l in nb_blocages)

    # Contraintes 

    # tout élément de l'univers est assigné exactement une fois 
    for i in univers:
        model += pulp.lpSum(x[i,j] for j in nb_blocages) == 1

    # tout élément de l'univers ne peut être assigné qu'à un ens. blocage 
    # Xj qui le contenait déjà
    for i in univers:
        for j in nb_blocages: 
            
            if i not in blocages[j]: 
                model += x[i,j] == 0

    # tout Xj est placé à également 1 position 
    for j in nb_blocages: 
        model += pulp.lpSum(gamma[j,l] for l in nb_blocages) == 1
    
    # tout position admet exactement un Xj 
    for l in nb_blocages: 
        model += pulp.lpSum(gamma[j,l] for j in nb_blocages) == 1

    # les positions ademettent des Xj dans un ordre croissant des cardinaux 
    for l in nb_blocages:
        if l == 0: continue # ignore le cas 0
        model += z[l] >= z[l-1]

    # liaison z, x et gamma 
    big_m = max(len(X) for X in blocages) # big_m grossier 
    for j in nb_blocages: 
        for l in nb_blocages: 
            model += (pulp.lpSum(x[i,j] for i in univers) + 
                      big_m * (1-gamma[j,l])) >= z[l]


    model.solve()

    # construction des nouveaux ensembles de blocages : 
    nouveaux_blocages = [[] for _ in nb_blocages]
    for i in univers: 
        for j in nb_blocages: 
            val = x[i,j].varValue
            if val == 1: 
                nouveaux_blocages[j].append(i)


    print(f"Statut : {pulp.LpStatus[model.status]}")
    print(f"Coût total optimal : {pulp.value(model.objective)}") 
    print("Ensembles de blocages disjoints : ")
    for j in nb_blocages: 
        print(nouveaux_blocages[j])


    return int(pulp.value(model.objective))


def fast_disjoint_balanced_heuristic(blocages: list[list]) -> int:
    
    k = len(blocages)
    
    # Cartographie des éléments vers les ensembles qui les contiennent
    element_to_sets = {}
    for j, b in enumerate(blocages):
        for e in b:
            if e not in element_to_sets:
                element_to_sets[e] = []
            element_to_sets[e].append(j)
            
    # On trie les éléments : on traite d'abord ceux qui ont le MOINS de choix 
    # (les éléments exclusifs) pour poser la structure forcée du graphe.
    elements_tries = sorted(element_to_sets.keys(), key=lambda e: len(element_to_sets[e]))
    
    nouveaux_blocages = [[] for _ in range(k)]
    tailles_courantes = [0] * k
    
    # Distribution
    for e in elements_tries:
        options = element_to_sets[e]
        # L'ÉQUILIBRAGE : On donne l'élément à l'ensemble candidat 
        # qui a la plus PETITE taille courante à ce moment-là.
        best_set = min(options, key=lambda j: tailles_courantes[j])
        
        nouveaux_blocages[best_set].append(e)
        tailles_courantes[best_set] += 1
        
    # Tri et calcul de la borne
    cardinaux = [len(b) for b in nouveaux_blocages]
    cardinaux.sort() # Ordre croissant
    
    borne = sum(cardinaux[i] * (k - i) for i in range(k))
    return borne




            

