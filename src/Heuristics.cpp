#include "Heuristics.hpp"


Heuristics::Heuristics(const Data& _data) : data(_data) 
{
    init_ordre_topo(); 
    compute_node_order_matrix();
    obj_val = compute_DSC_obj(); 
}


void Heuristics::init_ordre_topo() {

    ordre_to_node.reserve(data.dag_size);  
    node_to_ordre.resize(data.dag_size); 
    
    std::vector<int> in_degrees(data.dag_size, 0); 
    
    for(int u = 0; u < data.dag_size; ++u) { // pr chq node du dag
        in_degrees[u] = (int)data.reverse_dag[u].size(); 
    }

    std::queue<int> fifo_list; 

    for(int u = 0; u < data.dag_size; ++u) { 
        if(in_degrees[u] == 0) {
            fifo_list.push(u); 
            break; // on break car notre dag doit avoir qu'1 sommet de degré entrant = 0 (s)
        }
    }

    int pos_ordre = 0; 

    while(fifo_list.size() > 0) 
    {

        int curr_node = fifo_list.front(); // récup le sommet de degré 0
        fifo_list.pop(); // le retirer 

        ordre_to_node.push_back(curr_node); // l'ajouter à l'ordre topo
        node_to_ordre[curr_node] = pos_ordre; // ajouter sa position au sommet 
        ++pos_ordre; 

        for(const int neigh : data.dag[curr_node]) {
            if(--in_degrees[neigh] == 0) // si apres décrementation -> degré 0 
                fifo_list.push(neigh); // l'ajouter a la liste
        }

    }

    if((int)ordre_to_node.size() < data.dag_size) 
        throw std::runtime_error("Heuristic::init_ordre_topo() -> ordre topologique trouvé incorrect"); 

        
}


void Heuristics::compute_node_order_matrix() {
    
    std::vector<std::vector<int>> mat(data.dag_size, std::vector<int>(data.dag_size-1, 0));    

    for(int i = 1; i < data.dag_size; ++i) { // pr chq sommet dans l'ordre 

        int curr_node = ordre_to_node[i]; 
        for(int neigh : data.dag[curr_node]) { // pr chq succ direct 

            if(neigh == data.dag_size-1) continue; // si c'est le puit (t)

            int neigh_pos = node_to_ordre[neigh]; // récup sa position dans l'ordre 
            for(int j = i; j <= neigh_pos-1; ++j) {
                mat[curr_node][j]++; 
            } 

        }

    }

    node_order_matrix = mat; // assigner à l'attribut 

}


int Heuristics::compute_DSC_obj() const {

    int obj_val = 0; 
    for(int i = 0; i < data.dag_size; ++i) { // pr chq noeud 
        for(int j = 1; j < data.dag_size-2; ++j) { // pr chq ordre 
            if(node_order_matrix[i][j] > 0) ++obj_val;
        }   
    }

    return obj_val; 
}


void Heuristics::display_node_order_matrix() const {

    for(auto& v : node_order_matrix) {
        for(auto& i : v) {
            std::cout << i << " "; 
        }
        std::cout << std::endl;
    }

}


/* FONCTIONS POUR RECUIT SIMULÉ DSC */


bool Heuristics::is_move_valid(int indx) const {

    int t_index = data.dag_size-1; 

    if(indx <= 0 || indx >= t_index - 1)
        return false; // gestion cas impossibles 

    int u_l = ordre_to_node[indx]; 
    int u_r = ordre_to_node[indx+1]; 

    if(node_order_matrix[u_l][indx] - node_order_matrix[u_l][indx+1])
        return false; // si ça vaut 1, alors u_r succède à u_l 

    return true; 
}


int Heuristics::compute_delta_DSC(int indx) const {
    
    int t_index = data.dag_size-1; 
    int s_node = ordre_to_node[0]; // on stocke s

    if(indx == 0 || indx == t_index) // devrait jamais entrer ici avec indx == 0
        throw std::runtime_error("Heuristics::compute_delta_DSC -> indx == 0"); 

    int delta = 0; 
    
    int u_l = ordre_to_node[indx]; // ui
    int u_r = ordre_to_node[indx+1]; // ui+1

    // impact déplacement u_r à gauche 
    for(int pred : data.reverse_dag[u_r]) { 
        if(pred == s_node) continue; // ignorer s 

        if(node_order_matrix[pred][indx] == 1) {
            if(node_order_matrix[pred][indx-1] == 2) continue; 
            delta--; // si u_r était le seul a droite de indx qui comptait pour pred
        }
    }

    // impact déplacement u_l à droite 
    for(int pred : data.reverse_dag[u_l]) {
        if(pred == s_node) continue; // ignorer s 

        if(node_order_matrix[pred][indx] == 0) 
            delta++; // si aucun de ses succs comptait à droite de indx -> mtn u_l compte 
    }

    // si u_l avait au moins un voisin (forcément à droite de indx)
    if(node_order_matrix[u_l][indx] > 0) 
        delta--; // alors le déplacer à droite lui fait perdre sa participation en indx

    // si u_r avait au moins un voisin (forcément apres indx+1)
    if(node_order_matrix[u_r][indx+1] > 0)
        delta++; // alors le déplacer à gauche étend sa participation en indx

    return delta; 
}


void Heuristics::apply_move(const BestSwap& bs) {

    int indx_l = bs.indx_l; 
    int indx_r = bs.indx_r; 

    int u_l = ordre_to_node[indx_l];
    int u_r = ordre_to_node[indx_r];

    int s_node = ordre_to_node[0]; 

    // modifications node_order_matrix

    //impact déplacement u_r à gauche 
    
    for(int pred : data.reverse_dag[u_r]) {
        
        if(pred == s_node) continue; // ignore s
        --node_order_matrix[pred][indx_l];

    }

    // impact déplacement u_l à droite 
    
    for(int pred : data.reverse_dag[u_l]) {

        if(pred == s_node) continue; // ignorer s 
        ++node_order_matrix[pred][indx_l]; 
    
    }

    // impact sur la ligne de u_r
    node_order_matrix[u_r][indx_l] = node_order_matrix[u_r][indx_r]; 

    // impact sur la ligne de u_l
    node_order_matrix[u_l][indx_l] = 0; 

    // modifications val, et structure d'ordre topologique 

    ordre_to_node[indx_l] = u_r;
    ordre_to_node[indx_r] = u_l; 

    node_to_ordre[u_l] = indx_r; 
    node_to_ordre[u_r] = indx_l; 

    obj_val += bs.delta; 
    
}


bool Heuristics::metropolis(double temp, int delta, std::mt19937& gen) const {

    std::uniform_real_distribution<> distrib(0.0,1.0); 
    double p = distrib(gen); 
    if(p <= exp(-(double)delta/temp)) return true; 
    return false; 
}


void Heuristics::SAA_optimize(double temp, int iter_max) {

    std::random_device rd; // création graine aléatoire 
    std::mt19937 gen(rd()); // moteur aléatoire 

    int t = data.dag_size-1; // sommet puit

    while(temp > 0.01)
    {
        for(int i = 0; i < iter_max; ++i) {

            BestSwap bs; 
            int indx = random_int(1,t-2, gen); // on ignore s et t 

            // indx est l'indice de gauche
            if(is_move_valid(indx)) { 

                int delta = compute_delta_DSC(indx); 

                bs.found_move = true; 
                bs.delta = delta; 
                bs.indx_l = indx; 
                bs.indx_r = indx+1;   

            } 

            // indx est l'indice de droite (donc indx-1 celui de gauche)
            if(is_move_valid(indx-1)) {

                int delta = compute_delta_DSC(indx-1); 
                
                // si on avait déjà trouvé un améliorant et que sa valeur est moins bonne 
                // ou si on avait simplement pas trouvé d'améliorant 
                if((bs.found_move && bs.delta > delta) || !bs.found_move) 
                { 
                    bs.found_move = true; 
                    bs.delta = delta; 
                    bs.indx_l = indx-1; 
                    bs.indx_r = indx; 
                }

            }

            if(bs.found_move) {
                
                if(bs.delta < 0)
                    apply_move(bs); 
                else { // sinon pas améliorant -> métropolis 
                    if(metropolis(temp, bs.delta, gen))
                        apply_move(bs); 
                }

            }

        }

        temp = temp * 0.95; 
    }

}


