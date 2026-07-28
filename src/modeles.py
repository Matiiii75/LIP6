
import pulp 
from itertools import product 
from itertools import combinations
from itertools import permutations 

def read_file(file:str) -> list[list]: 

    with open(file, "r") as f: 

        line = f.readline().strip().split(" ")
        nb_node = int(line[0])
        nb_arc = int(line[1])
        degen = int(line[2])

        dag = [[] for _ in range(nb_node)]

        for _ in range(nb_arc): 

            line = f.readline().strip().split(" ")
            debut = int(line[0])
            fin = int(line[1])
            dag[debut].append(fin)

    return dag 

# implémentation des différents modèles PL 

def modele_positions(dag:list[list]) -> int:

    nb_nodes = len(dag)
    source = 0
    puit = nb_nodes - 1

    model = pulp.LpProblem("test", pulp.LpMinimize) 

    # définitions des ensembles d'indices 

    all_pos = [i for i in range(nb_nodes-2)] # -2 car on enleve le puit et source
    all_nodes = [i for i in range(nb_nodes) if (i != source and i != puit)]
    all_arcs = [(i,j) for i in range(nb_nodes)
                for j in dag[i]
                if i != source and j != puit
                ]
    last_pos = len(all_pos) - 1

    # variables 
    
    indices_x = list(product(all_pos, all_nodes))
    x = pulp.LpVariable.dicts("x",
                              indices_x,
                              lowBound=0,
                              upBound=1,
                              cat="Binary"
                              )

    indices_y = list(product(all_pos, all_nodes))
    y = pulp.LpVariable.dicts("y",
                              indices_y,
                              lowBound=0,
                              upBound=1,
                              cat="Binary"
                              )

    # objectif 

    model += pulp.lpSum(
        y[i,j]
        for i in all_pos
        for j in all_nodes
        )

    # contraintes 

    for i in all_pos: # tt position occupée 
        model += pulp.lpSum(x[i,j] for j in all_nodes) == 1

    for j in all_nodes: # tt noeud placé 
        model += pulp.lpSum(x[i,j] for i in all_pos) == 1

    for i0 in all_pos: # respect de l'ordre imposé par les arcs du dag 
        for arc in all_arcs: 
            j1 = arc[0]
            j2 = arc[1]
            model += pulp.lpSum((x[i,j1] - x[i,j2]) for i in range(i0+1)) >= 0

    for i0 in all_pos: # définition des y 
        if i0 == last_pos: continue 
        for arc in all_arcs: 
            j1 = arc[0]
            j2 = arc[1]
            model += y[i0,j1] +1 >= ( pulp.lpSum(x[i,j1] for i in range(i0+1)) + 
                                    pulp.lpSum(x[i,j2] for i in range(i0+1, last_pos+1)) )


    model.solve() # résolution 

    print(f"Statut : {pulp.LpStatus[model.status]}")
    print(f"opt val : {pulp.value(model.objective)}")


def modele_position_relatives(dag:list[list], relax: bool = False): 

    nb_nodes = len(dag)
    source = 0
    puit = nb_nodes - 1
    all_nodes = [i for i in range(nb_nodes) if i != source and i != puit]
    all_arcs = [(u,v) for u in all_nodes for v in dag[u] if v != puit] 
    z_indices = [(u,v) for u in all_nodes for v in all_nodes if u != v]

    cat_z = "Continuous" if relax else "Binary"

    model = pulp.LpProblem("modele2_bis", pulp.LpMinimize)

    # variables : 

    z = pulp.LpVariable.dicts("z", z_indices, lowBound = 0, upBound = 1, cat=cat_z)
    phi = pulp.LpVariable.dicts("phi", all_nodes, lowBound=0, cat="Continuous")
    mu = pulp.LpVariable.dicts("mu", all_nodes, lowBound=0, cat="Continuous")

    # objectif : 

    model += pulp.lpSum(mu[u] - phi[u] for u in all_nodes)

    # contraintes : 

    for arc in all_arcs: # si (u,v) \in A -> zuv = 1
        u = arc[0]
        v = arc[1]
        model += z[u,v] == 1

    for i, u in enumerate(all_nodes): # antisymétrie 
        for v in all_nodes[i+1:]:
            model += z[u, v] + z[v, u] == 1


    for u, v, w in combinations(all_nodes, 3): # équivalent transitivité (3-cyle inequalities)
        model += z[u,v] + z[v,w] + z[w,u] <= 2
        model += z[u,w] + z[w,v] + z[v,u] <= 2

    for u in all_nodes: # définition de phi 
        model += phi[u] == pulp.lpSum(z[v,u] for v in all_nodes if v != u)

    for u in all_nodes: 
        model += mu[u] >= phi[u]
        for v in dag[u]: 
            if v != puit: 
                model += mu[u] >= phi[v]

    model.solve(pulp.PULP_CBC_CMD(msg=0))

    val = pulp.value(model.objective)
    print("obj val -> ", val)



def modele_with_lazy_cuts(dag:list[list], relax: bool = False): 

    nb_nodes = len(dag)
    source = 0
    puit = nb_nodes - 1
    all_nodes = [i for i in range(nb_nodes) if i != source and i != puit]
    all_arcs = [(u,v) for u in all_nodes for v in dag[u] if v != puit] 
    z_indices = [(u,v) for u in all_nodes for v in all_nodes if u != v]

    cat_z = "Continuous" if relax else "Binary"

    model = pulp.LpProblem("modele2_bis", pulp.LpMinimize)

    # variables : 

    z = pulp.LpVariable.dicts("z", z_indices, lowBound = 0, upBound = 1, cat=cat_z)
    phi = pulp.LpVariable.dicts("phi", all_nodes, lowBound=0, cat="Continuous")
    mu = pulp.LpVariable.dicts("mu", all_nodes, lowBound=0, cat="Continuous")

    # objectif : 

    model += pulp.lpSum(mu[u] - phi[u] for u in all_nodes)

    # contraintes : 

    for arc in all_arcs: # si (u,v) \in A -> zuv = 1
        u = arc[0]
        v = arc[1]
        model += z[u,v] == 1

    for i, u in enumerate(all_nodes): # antisymétrie 
        for v in all_nodes[i+1:]:
            model += z[u, v] + z[v, u] == 1

    for u in all_nodes: # définition de phi 
        model += phi[u] == pulp.lpSum(z[v,u] for v in all_nodes if v != u)

    for u in all_nodes: 
        model += mu[u] >= phi[u]
        for v in dag[u]: 
            if v != puit: 
                model += mu[u] >= phi[v]

    epsilon = 1e-5
    iteration = 0

    while True: # tant qu'on a pas trouvé une solution ne violant pas de contraintes 

        model.solve(pulp.PULP_CBC_CMD(msg=0))

        # récupérer les valeurs des z 
        
        violated_cuts = []

        # on récup tt les valeurs dans un dict avant 
        z_vals = {(u,v): (pulp.value(z[u,v]) or 0.0) for u,v in z_indices}

        for u, v, w in combinations(all_nodes, 3): 

            if z_vals[u,v] + z_vals[v,w] + z_vals[w,u] > 2 + epsilon:
                violated_cuts.append((u,v,w))

            if z_vals[u,w] + z_vals[w,v] + z_vals[v,u] > 2 + epsilon: 
                violated_cuts.append((u,w,v))

        if len(violated_cuts) == 0: 
            print(f"sortie à itération {iteration}") 
            break

        print(f"iteration {iteration} : {len(violated_cuts)} contraintes violées")

        # ajout des contraintes au modele
        for u,v,w in violated_cuts: 
            model += z[u,v] + z[v,w] + z[w,u] <= 2

        iteration += 1

    val = pulp.value(model.objective)
    return val


# dag = read_file("../instances/k_variations/100_n_61_k_3_ID.txt")
# modele_with_lazy_cuts(dag,True)

        
