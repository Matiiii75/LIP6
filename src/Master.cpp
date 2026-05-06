#include "Master.hpp"


Master::Master(const Data& _data, int s) : data(_data), SG(_data, s)
{
    L.push(0); // ajouter l'ID du premier candidat  
}


std::vector<int> Master::compute_cut_set(const std::vector<int>& cand) const {

    std::vector<int> cut_set; // comme on connait k, on peut allouer a l'avance !!
    
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
            cut_set.push_back(i); // si c'est faux, alors il est hors successeur, donc dans le cut set 
    }

    return cut_set; 
}

// algo 1 
void Master::build_SG() {

    while(!L.empty()) 
    {  
        
        int C_ID = L.front(); // on récupère l'ID de l'ens. cand. devans la FIFO
        L.pop(); // on l'efface 

        std::vector<int> C = SG.get_cand(C_ID); 
        std::vector<int> cut_set = compute_cut_set(C); // on calcule le cut_set associé

        if(C_ID != 0) { // si l'ID est 0 -> alors c'est s et on a déjà set le poids a 0
            int C_weight = SG.compute_weight_C(C_ID, cut_set); // on récup le poids de C
            SG.set_weight(C_ID, C_weight); // ajout du poids 
        } 

        std::vector<int> C2; 
        C2.reserve(C.size()-1); // on réserve l'espace pour copier le C 
    
        for(int i = 0; i < (int)C.size(); ++i) { // pr chq candidat de C

            C2 = C; // copier C, mettre le candidat en derniere pos, et le suppr 
            C2[i] = C2.back(); 
            C2.pop_back(); 

            int curr_c = C[i]; // copie c 

            std::vector<int> extended_cut_set = cut_set; // il faut étendre le cut set au noeud candidat 
            auto it = std::upper_bound(extended_cut_set.begin(), extended_cut_set.end(), curr_c); 
            extended_cut_set.insert(it, curr_c); 

            for(const int u : data.dag[curr_c]) { // (l.8)

                if(is_included(data.reverse_dag[u], extended_cut_set))
                    C2.push_back(u); 

            }

            increase_sort_vector(C2); // tri croissant 
            
            if(C2.size() == 0) continue; // si C2 est vide, inutile de continuer 

            keyHash C2_hash = compute_cand_hash(C2, data.node_to_hash); 
            int C2_ID = SG.is_cand_in_SG(C2, C2_hash); 

            if(C2_ID == -1) { // (l.11)
                
                SG.add_cand_to_SG(C2, C2_hash); 
                C2_ID = (int)SG.ID_to_cands.size()-1; // on vient de l'ajouter, son ID est le dernier index 
                L.push(C2_ID); // ajout à FIFO

            }

            SG.add_arc_from_C1_to_C2(C_ID, C2_ID); // ajoute l'arc 

        }

    }

}






