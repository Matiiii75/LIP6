#include "Master.hpp"


Master::Master(const Data& _data, int _s, int _t) : data(_data), SG(_data, _s, _t)
{
    L.push(0); // ajouter l'ID du premier candidat  
    best_dist.push_back(0); // le coût pour aller au premier candidat est nul 
    pred_in_pcc.push_back({-1,-1}); 
}


std::unordered_set<int> Master::compute_cut_set(const std::vector<int>& cand) const {

    std::unordered_set<int> cut_set; // comme on connait k, on peut allouer a l'avance !!
    
    // j'utilise un vecteur de bool où tout est faux au debut. 
    // on va parcourir les N_G^+(cand) et mettre vrai a l'indice de ceux qui y sont
    // ensuite, suffira d'inverser les valeurs de vérité. 
    std::vector<bool> is_in_cut_set(data.dag_size, false); 

    for(const int c : cand) { // pr chq candidat 
        for(int i = 0; i < data.dag_size; ++i) { // pr chq noeud du graphe
            
            if(data.TC[c][i]) is_in_cut_set[i] = true; // si c'est 1 dans la transitive closure -> true 

        }
    }

    for(int i = 0; i < data.dag_size; ++i) {
        if(is_in_cut_set[i] == false) 
            cut_set.insert(i); // si c'est faux, alors il est hors successeur, donc dans le cut set 
    }

    return cut_set; 
}


void Master::build_SG() {

    while(!L.empty()) 
    {  

        int C_ID = L.front(); // on récupère l'ID de l'ens. cand. devans la FIFO
        L.pop(); // on l'efface 

        std::vector<int> C = SG.get_cand(C_ID); 
        std::unordered_set<int> cut_set = compute_cut_set(C); // on calcule le cut_set associé

        std::vector<int> C2; 
        C2.reserve(C.size()-1); // on réserve l'espace pour copier le C 
    
        for(int i = 0; i < (int)C.size(); ++i) { // pr chq candidat de C

            C2 = C; // copier C, mettre le candidat en derniere pos, et le suppr 
            C2[i] = C2.back(); 
            C2.pop_back(); 

            int curr_c = C[i]; // copie c 

            cut_set.insert(curr_c); // on étend le cut set au noeud candidat 

            for(const int u : data.dag[curr_c]) { // (l.7) : pr chq succ u du candidat

                if(is_included(data.reverse_dag[u], cut_set, -1)) // si tt les pred de u sont dans cut-set
                    C2.push_back(u); // -1 signifie qu'on regarde la simple inclusion de r-dag dans e-c-s 

            }

            increase_sort_vector(C2); // tri croissant 
            
            if(C2.size() == 0) continue; // si C2 est vide, inutile de continuer 

            keyHash C2_hash = compute_cand_hash(C2, data.node_to_hash); 
            int C2_ID = SG.is_cand_in_SG(C2, C2_hash); 

            if(C2_ID == -1) { // (l.10) (si il vaut -1 c'est qu'on a pas trouvé d'ID pr ce hash)
                
                SG.add_cand_to_SG(C2, C2_hash); 
                C2_ID = (int)SG.ID_to_cands.size()-1; // on vient de l'ajouter, son ID est le dernier index 
                L.push(C2_ID); // ajout à FIFO
                int C2_weight = SG.compute_weight_C(C2_ID, C_ID, curr_c, cut_set); // cut_set est le cut-set de C2 
                SG.set_weight(C2_ID, C2_weight); // on calcule le poids de C2 
                best_dist.push_back(std::numeric_limits<int>::max()); // inf par défaut 
                pred_in_pcc.push_back({-1,-1}); // ajout d'un sommet defaut pour garder pred_in_pcc bien indéxé 

            }

            SG.add_arc_from_C1_to_C2(C_ID, C2_ID); // ajoute l'arc 
            
            int dist_from_C = best_dist[C_ID] + SG.weights[C2_ID]; 
            if(best_dist[C2_ID] > dist_from_C) { // voir si on améliore le pcc jusqu'à C2 en passant par C 
                
                best_dist[C2_ID] = dist_from_C; 
                pred_in_pcc[C2_ID] = {C_ID, curr_c}; // retenir d'où on vient  

            }

            // on fini la boucle sur curr_c 
            // -> on l'efface pour passer au prochain candidat 
            cut_set.erase(curr_c); 
        }

    }

}


std::vector<int> Master::rebuild_opt_order() const {

    std::vector<int> ordre_topo; 
    int curr_node = (int)pred_in_pcc.size() - 1; // on récup le dernier ens candidat ( {t} )

    while(curr_node != 0) {

        ordre_topo.push_back(pred_in_pcc[curr_node].second); 
        curr_node = pred_in_pcc[curr_node].first; 

    }

    std::reverse(ordre_topo.begin(), ordre_topo.end()); 
    return ordre_topo; 
}


bool Master::checker_DSC(const std::vector<int>& ordre_topo, int val_found) const {


    // commencer par vérifier que c'est bien un ordre topologique valide
    // ie : vérifier que pour toute paire (u,v) suivant l'ordre topo, 
    // (v,u) notin A 

    for(int i = 0; i < (int)ordre_topo.size()-1; ++i) {
        for(int j = i+1; j < (int)ordre_topo.size(); ++j) {

            int node1 = ordre_topo[i]; // 1er dans l'ordre
            int node2 = ordre_topo[j]; // 2nd

            for(int neigh : data.dag[node2])
                if(neigh == node1) {
                    std::cout << node1 << "est un successeur direct de "
                        << node2 << " dans le dag initial " << std::endl;
                    return false; 
                } 

        }
    }

    // calculer la valeur de l'ordre topologique et regarder qu'on obtient bien la même 

    int obj_val = 0; // pr stocker la valeur objective 

    for(int i = 0; i < (int)ordre_topo.size()-1; ++i) { // pour chaque ordre 
        int valeur_ordre_i = 0;
        for(int j = 0; j <= i; ++j) { // pr chq sommet dans la coupe
            
            bool go_next_in_coupe = false;
            int node_in_coupe = ordre_topo[j]; // stocke le noeud ds la coupe
            for(int k = i+1; k < (int)ordre_topo.size(); ++k) { // pr chq sommet hors coupe
                
                int node_hors_coupe = ordre_topo[k]; // noeud hors coupe
                for(int neigh : data.dag[node_in_coupe]) {
                    if(neigh == node_hors_coupe) {
                        ++valeur_ordre_i; // on a trouvé un succ hors coupe -> ++
                        go_next_in_coupe = true; 
                        break; // aller au prochain noeud in-coupe
                    }
                }
                if(go_next_in_coupe) break; 
            }
        }
        obj_val += valeur_ordre_i; 
    }

    if(obj_val != val_found) { // check la valeur obj 
        std::cout << "Erreur dans la valeur calculée : " << std::endl;
        std::cout << "Valeur trouvée : " << val_found << std::endl;
        std::cout << "Valeur correcte : " << obj_val << std::endl;
        return false; 
    } else {
        std::cout << "Valeur calculée OK !" << std::endl;
    }

    return true; 
}




