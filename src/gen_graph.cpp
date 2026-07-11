#include "gen_graph.hpp"

Dag::Dag(int size) {
    dag.resize(size); 
    nb_nodes = size; 
}

void Dag::compute_transitive_closure() { 

    // redimensionne le vecteur pr avoir nb_nodes lignes 
    // chaque bitset vaut 0 au début 
    TC.assign(nb_nodes, std::bitset<5001>()); 
    
    // remplissage initial
    // les diagonales valent 1 
    // et on copie la matrice d'adjacence du graphe
    for(int i = 0; i < nb_nodes; ++i) {
        TC[i].set(i); // TC[i][i] = 1
        for(int j : dag[i]) {
            TC[i].set(j);  // TC[i][j] = 1
        }
    }

    for(int k = 0; k < nb_nodes; ++k) {
        for(int i = 0; i < nb_nodes; ++i) {

            // test(k) vérifie si le bit k vaut 1 (TC[i][k] = 1)
            if(TC[i].test(k)) TC[i] |= TC[k]; 
            
            // si TC[i][k] vaut 1, on veut regarder tous les j tels que 
            // TC[k][j] = 1 et ainsi mettre TC[i][j] = 1 par transitivité. 
            // le bitset est pratique il permet de faire ça rapidement grâce au OU logique. 
            // si on considère un certain indice j, TC[i][j] OU TC[k][j] vaut : 

            // -> 0 OU 0 = 0  (si TCij = 0 et TCkj = 0 alors TC[i][j] = 0)
            // -> 1 OU 0 = 1  (si TCij = 1 et TCkj = 0 alors TC[i][j] = 1)
            // -> 0 OU 1 = 1  (si TCij = 0 et TCkj = 1 alors TC[i][j] = 1)
            // -> 1 OU 1 = 1  (si TCij = 1 et TCkj = 1 alors TC[i][j] = 1)

        }
    }
}

void Dag::write_in_file(int ID) const {
    
    std::string file_name = std::to_string(nb_nodes) + "_n_" 
        + std::to_string(degenerascy) + "_k_" 
        + std::to_string(ID) + "_ID"; 
    
        std::string path_to_file = "../inst_new/" + file_name + ".txt"; 

    std::ofstream file_to_write; 
    file_to_write.open(path_to_file); 

    if(!file_to_write.is_open()) 
        throw std::runtime_error("gen_graph::write_dag_in_file -> impossible d'ouvrir le fichier"); 

    // écriture des données dans le fichier au format : 
    // nb_nodes nb_arcs degenerascy
    // i -> j pr tt i et j concernés 

    file_to_write << nb_nodes << " "; 
    file_to_write << nb_arcs << " "; 
    file_to_write << degenerascy << std::endl;

    for(int i = 0; i < (int)nb_nodes; ++i) {
        for(int neigh : dag[i]) {
            
            file_to_write << i << " " << neigh << std::endl;
            
        }
    }

    file_to_write.close(); 

}

void Dag::compute_HG() {

    HG.resize(nb_nodes); 
    int count_nb_edge = 0; 

    for(int i = 0; i < nb_nodes; ++i) {
        for(int j = i+1; j < nb_nodes; ++j) {

            // TC[i].test(j) vaut vrai si TC[i][j] = 1
            if(!TC[i].test(j) && !TC[j].test(i)) { // si TC[i][j] = 0 ET TC[j][i] == 0 -> ils sont incomparables. 
                HG[i].push_back(j); 
                HG[j].push_back(i); 
                count_nb_edge++; 
            }

        }
    }

    HG_nb_edge = count_nb_edge; 
}

void Dag::compute_degenerascy() {

    // structures pr les degrés 
    std::vector<int> nodes_to_degree(nb_nodes, 0); 
    std::vector<std::vector<int>> degree_to_nodes(nb_nodes + 1); 

    // les remplir 
    for(int u = 0; u < nb_nodes; ++u) 
    {
        int d_u = (int)HG[u].size(); 
        nodes_to_degree[u] = d_u; 
        degree_to_nodes[d_u].push_back(u); 
    }

    int k = 0; 

    for(int i = 0; i < nb_nodes; ++i) { // on répète n fois (autant que de sommets)
        for(int deg = 0; deg < (int)degree_to_nodes.size(); ++deg) {

            if(!degree_to_nodes[deg].empty()) { // dès qu'on en trouve un 

                if(deg > k) k = deg; 
                int node = degree_to_nodes[deg][0]; // arbitrairement, on prend le 1er 
                degree_to_nodes[deg].erase(degree_to_nodes[deg].begin()); // on le suppr
                nodes_to_degree[node] = -1; // -1 pr défaut pr se rappeler qu'on l'a retiré 

                for(int neigh : HG[node]) { // pr chq1 de ses voisins 
                    
                    int neigh_deg = nodes_to_degree[neigh]; 
                    if(neigh_deg == -1) continue; // déjà traité 

                    // retirer neigh de degree_to_nodes[neigh_deg]
                    auto it = std::find(
                        degree_to_nodes[neigh_deg].begin(), 
                        degree_to_nodes[neigh_deg].end(), 
                        neigh
                    ); 
                    if(it != degree_to_nodes[neigh_deg].end()) { 
                        degree_to_nodes[neigh_deg].erase(it); 
                    } else {
                        throw std::runtime_error("Dag::compute_degenerascy -> devrais pas arriver"); 
                    }

                    // l'ajouter a neigh_deg-1
                    degree_to_nodes[neigh_deg-1].push_back(neigh); 

                    // maj son degré 
                    nodes_to_degree[neigh]--; 

                }

                break; // on a modifié les structures, on passe a la prochaine itération 
            }

        }
    }

    degenerascy = k; 
}

void Dag::gen_graph(double p) {

    assert(p < 1.0 && p > 0.0); 

    int source = 0; 
    int puit = nb_nodes - 1; 
    int original_dag_size = nb_nodes - 2; // taill sans compter puit & source 

    std::vector<int> in_degs(nb_nodes, 0); // pour mémoriser l'évolution des degrés entrants
    std::vector<int> out_degs(nb_nodes, 0); // et sortants

    int nb_arc_counter = 0; 

    for(int i = 1; i < original_dag_size; ++i) { 
        for(int j = i+1; j <= original_dag_size; ++j) {

            if(random_double(0.0,1.0) < p) {
                dag[i].push_back(j); 
                nb_arc_counter++; // on mémorise l'ajout de l'arc 
                in_degs[j]++; 
                out_degs[i]++; 
            }

        }
    }

    // ajouter les liaisons entre la source/puits et les sommets du dag oririnal

    for(int i = 1; i <= original_dag_size; ++i) { // pr chq sommet original du dag 

        if(in_degs[i] == 0) {// si degré entrant nul -> liaison source
            dag[source].push_back(i); 
            nb_arc_counter++; 
        }

        if(out_degs[i] == 0) {// si degré sortant nul -> liaison puit 
            dag[i].push_back(puit); 
            nb_arc_counter++; 
        }

    }

    nb_arcs = nb_arc_counter; 
}

void Dag::display_infos() const {

    std::cout << "Affichage des informations sur le dag : " << std::endl;
    std::cout << "Nombre de sommets : " << nb_nodes << " | "; 
    std::cout << "Nombre d'arcs : " << nb_arcs << " | "; 
    std::cout << "Degenerascy du graphe de co-comparabilité : " << degenerascy; 
    std::cout << std::endl;
    std::cout << "Nombre edge HG : " << HG_nb_edge << std::endl;

}

void Dag::display_dag() const {

    std::cout << "Affichage du dag : " << std::endl;
    for(int i = 0; i < nb_nodes; ++i) {
        std::cout << i << " : {"; 
        for(int j = 0; j < (int)dag[i].size(); ++j) {
            
            if(j != (int)dag[i].size()-1) {
                std::cout << dag[i][j] << ","; 
            } else {
                std::cout << dag[i][j]; 
            }
            
        }
        std::cout << "}" << std::endl;
    }

}

void Dag::display_HG() const {

    std::cout << "Affichage graphe de co-comparabilité :" << std::endl;
    for(int i = 0; i < nb_nodes; ++i) {
        std::cout << i << " : {"; 
        for(int j = 0; j < (int)HG[i].size(); ++j) {

            if(j != (int)HG[i].size()-1) {
                std::cout << HG[i][j] << ","; 
            } else {
                std::cout << HG[i][j]; 
            }

        }
        std::cout << "}" << std::endl;
    }

}

double random_double(double a, double b) {

    static std::random_device rd; 
    static std::mt19937 gen(rd()); 
    std::uniform_real_distribution<double> dis(a,b);
    
    return dis(gen);     
}












