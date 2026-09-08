# Optimisation d'Ordres Topologiques dans les DAGs (Directed Sum Cut)

## 1. Introduction
Ce dépôt héberge le code source de mon projet de recherche en algorithmique et optimisation, réalisé au laboratoire LIP6. Ce projet se concentre sur l'optimisation d'ordres topologiques dans les Graphes Orientés Acycliques (DAGs), en s'appuyant sur les concepts de la complexité paramétrée. L'objectif principal est de résoudre le problème du *Dag Sum Cut* (DSC) en comparant une modélisation par Programme Linéaire en Nombres Entiers (PLNE) résolue avec Gurobi, face à un algorithme FPT (Fixed-Parameter Tractable) de programmation dynamique.

## 2. Définition Formelle du Problème

Soit un Graphe Orienté Acyclique (DAG) $G = (V, A)$ avec $n = |V|$ sommets. 
Un **ordre topologique** $\varphi$ définit un ordre total sur l'ensemble des sommets $V$ tel que pour tout arc $(u, v) \in A$, on a $\varphi(u) < \varphi(v)$.

Pour un ordre topologique $\varphi$ donné, on définit la **coupe** à l'ordre $i$ (avec $1 \le i \le n$) par l'ensemble des sommets placés avant ou à la position $i$, et ayant au moins un successeur strictement après $i$ :

$$ \Delta(\varphi, i) = \{u \in V : \varphi(u) \le i \wedge (\exists v \in V : \varphi(v) > i \wedge (u,v) \in A)\} $$

Le problème du **Dag Sum Cut (DSC)** consiste à trouver un ordre topologique $\varphi^*$ minimisant la somme de ces coupes sur toutes les positions :

$$ \min_{\varphi} \sum_{i \in [|V|]} |\Delta(\varphi, i)| $$

**Paramètre de Dégénérescence ($k$) :**
La complexité de notre approche exacte s'exprime en fonction de la dégénérescence du graphe de co-comparabilité $\mathcal{H}_G$. Deux sommets sont reliés dans $\mathcal{H}_G$ s'ils sont incomparables dans la clôture transitive de $G$. Le nombre de cliques (et donc d'ensembles candidats) est borné par $\mathcal{O}(n2^k)$, ce qui garantit la complexité FPT de l'algorithme.

## 3. Algorithmes et Complexité

Ce projet oppose deux paradigmes de résolution exacte, couplés à une méthode heuristique d'amorçage :
*   **Approche FPT (Programmation Dynamique) :** L'algorithme explore un graphe d'états ($\mathcal{SG}_G$) où chaque nœud représente un ensemble candidat de sommets pour former une coupe valide. L'implémentation inclut un hachage très performant sur 128 bits (inspiré de Zobrist) pour stocker et identifier les états en évitant les collisions. Nous exploitons également une borne inférieure avancée ($\mathcal{L}_2$) basée sur les ensembles de blocage pour élaguer le graphe d'états à la volée. Un pré-traitement par décomposition en composantes faiblement connexes permet de réduire massivement l'espace de recherche pour les graphes peu denses.
*   **Approche PLNE (Gurobi) :** Deux modèles mathématiques ont été testés. Le plus performant, retenu ici, utilise des variables de précédence relatives ($z_{u,v} = 1$ si $u$ précède $v$) et intègre des contraintes de transitivité (3-dicycles) générées à la volée (*Lazy Cuts*) via des callbacks.
*   **Heuristique (Recuit Simulé / SAA) :** Implémentation d'un algorithme de recherche locale effectuant des échanges (*swaps*) entre sommets adjacents dans l'ordre topologique. L'évaluation du delta d'objectif se fait en $\mathcal{O}(1)$ grâce à une matrice de participation $\mathcal{MP}$. Cette heuristique fournit une borne supérieure très stricte pour amorcer l'élagage de l'algorithme exact.

## 4. Architecture du Projet

Le code source est divisé entre des scripts générateurs et le cœur algorithmique en C++ :

    📂 Racine du projet
    ├── 📄 Dag_generator.py        # Génération de DAGs selon n et k ciblés (via régression logit-polynomiale)
    ├── 📄 Makefile                # Script de compilation automatisée pour les environnements avec Gurobi
    ├── 📄 common.hpp/.cpp         # Outils partagés, structures de parsers et d'exports de logs
    ├── 📄 Data.hpp/.cpp           # Chargement des instances, calcul de la clôture transitive et initialisation des hashs
    ├── 📄 Timer.cpp               # Chronométrage précis des temps d'exécution
    ├── 📄 User_choices.cpp/.hpp   # Configuration utilisateur (activation élagage, pré-traitement, limites)
    ├── 📄 State_graph.hpp/.cpp    # Modélisation du graphe d'états, hachage 128-bit, gestion des identifiants et collisions
    ├── 📄 Heuristics.hpp/.cpp     # Recuit Simulé (température initiale automatique, évaluation des deltas via MP)
    ├── 📄 Master.hpp/.cpp         # Orchestrateur FPT : construction de SG, calcul de L2, élagage, et pré-traitement
    ├── 📄 main.cpp                # Point d'entrée pour l'algorithme FPT, SAA et Pré-traitement
    ├── 📄 gurobi_modeles.hpp/.cpp # Définition des modèles PLNE et des Callbacks (Lazy Constraints)
    └── 📄 main_gurobi_modeles.cpp # Point d'entrée exécutable pour les benchmarks PLNE

## 5. Installation et Exécution

### Prérequis
*   Compilateur **C++17** (`g++` ou `clang++`).
*   Licence et bibliothèque **Gurobi Optimizer** (les chemins doivent être configurés dans le `Makefile`).
*   **Python 3** avec la bibliothèque `networkx` pour la génération des DAGs.

### Compilation
Un fichier `Makefile` est fourni. Pour compiler l'ensemble des exécutables, lancez simplement :

    make

Cela génère deux binaires : `prog` (pour l'algorithme FPT et SAA) et `prog_gurobi` (pour la PLNE).

### Exécution des approches (C++)

**Algorithme de Complexité Paramétrée et Heuristiques (`prog`) :**

    ./prog <chemin_instance.txt> <mode_execution> <elagage_LB2> <ecriture_resultats> [pourcentage_elagage]

*   `<mode_execution>` : `0` = FPT pur, `1` = SAA seul, `2` = FPT + Pré-traitement.
*   `<elagage_LB2>` : `0` (Désactivé) ou `1` (Activé).
*   `<ecriture_resultats>` : `0` (Non) ou `1` (Oui, écriture dans le dossier `results/`).
*   `[pourcentage_elagage]` : (Optionnel, ex: `0.1`) Requis uniquement si `<elagage_LB2>` vaut `1`.

**Modèle Gurobi (`prog_gurobi`) :**

    ./prog_gurobi <chemin_instance.txt> <choix_modele> <lazy_cuts> <sauvegarder_resultats>

*   `<choix_modele>` : `0` (Positions absolues), `1` (Positions relatives).
*   `<lazy_cuts>` : `0` (Désactivé), `1` (Activé pour le modèle 1).
