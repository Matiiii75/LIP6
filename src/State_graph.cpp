#include "State_graph.hpp"


State_graph::State_graph(const Data& _data, int s) : data(_data) // constructeur 
{

    // ajouter le premier sommet à SG, 
    // il s'agit de l'ensemble des successeurs de s 
    // (l.1 - algo 1)

    std::vector<int> first_cand; // ALLOUER K D'ESPACE MAX ?? 
    for(const int i : data.dag[s]) // calcul du premier sommet de SG 
        first_cand.push_back(i); 
    
    increase_sort_vector(first_cand); // tri ordre croissant 
    
    keyHash first_cand_hash; // calculer le hash de first_cand 
    first_cand_hash = compute_cand_hash(first_cand, data.node_to_hash); 
     
    // en appelant "add_cand_to_SG", on fait automatiquement les choses suivantes : 
    // - ajouter relation ID -> first_cand
    // - ajouter relation hash_first_cand -> ID 
    // - ajouter de la place pour stocker les succ de first_cand dans SG 
    // - ajouter de la place à l'index "ID" dans weights (par défaut -1)

    add_cand_to_SG(first_cand, first_cand_hash); 
    weights[0] = 0; // le poids de tt arc sortant vaut 0

}


int State_graph::is_cand_in_SG(const std::vector<int>& cand, const keyHash& cand_hash) const {

    auto it = hash_to_ID.find(cand_hash); // plus efficace de mémoriser l'emplacement par itérateur à l'avance 

    if(it == hash_to_ID.end()) { // si la clé n'existe pas 
        return -1; 
    }
    
    for(const int id : it->second) { // pr chq ID associé à cand_hash
        if(cand == ID_to_cands[id]) // si la clique associée à l'id courant == cand : on l'a trouvé -> renvoie vrai 
            return id; 
    }

    return -1; 
}


int State_graph::compute_weight_C(int C_ID, const std::vector<int>& cut_set) const {

    int K_ID = get_pred_C(C_ID); // on récupère le pred
    int K_weight = weights.at(K_ID); 
    // le ".at" lève une erreur si l'index K_ID n'existait pas (+ lent mais + safe)
    // on le garde pr la version expérimentale. 

    // on fait des alias sur les deux ensembles candidats 
    const std::vector<int>& K = ID_to_cands[K_ID]; 
    const std::vector<int>& C = ID_to_cands[C_ID]; 

    int C_weight = K_weight; // (l.3 - algo 2)
    int gamma = find_gamma(K, C); // récup gamma 

    for(int u : data.reverse_dag[gamma]) { // pr tout pred de gamma
        if(is_included(data.dag[u], cut_set))
            --C_weight; 
    }

    if(is_disjoint(data.reverse_dag[gamma], C, data.TC)) { // (l.8,9 - algo 2)
        ++C_weight; 
    }

    return C_weight; 
}


void State_graph::add_cand_to_SG(const std::vector<int>& cand, const keyHash& cand_hash) {

    ID_to_cands.push_back(cand); // ajoute la relation ID to cand 
    int cand_ID = (int)ID_to_cands.size() - 1; // on récupère son ID 
    hash_to_ID[cand_hash].push_back(cand_ID); // associer au hash son ID 
    SG.push_back({}); // initialiser un vecteur vide pour l'index "cand_ID"
    weights.push_back(-1); // on ajoute le poids -1 par défaut à l'ID 

}


void State_graph::add_arc_from_C1_to_C2(int cand1_ID, int cand2_ID) {
    SG[cand1_ID].push_back(cand2_ID); 
    preds_SG[cand2_ID] = cand1_ID; // sauvegarder le pred de cand2_ID 
}


int State_graph::get_pred_C(int C_ID) const {
    
    auto it = preds_SG.find(C_ID); // calcul itérateur pour pas le faire deux fois avec un count 
    if(it == preds_SG.end()) // si pas de pred enregistré 
        throw std::runtime_error("Erreur dans State_graph (get_pred_C) : pas de pred mémorisé"); 
    
    return it->second; 
}


std::vector<int> State_graph::get_cand(int ID) const {
    return ID_to_cands.at(ID); 
}


void State_graph::set_weight(int ID, int w) {

    if(ID >= (int)weights.size()) { // gestion erreur (cas improbable)
        throw std::out_of_range("State_graph : ID : " + std::to_string(ID) + 
        " n'existe pas en tant qu'index dans weights"); 
    }

    weights[ID] = w; 

}


void State_graph::display_SG() const {

    for(int i = 0; i < (int)SG.size(); ++i) {
        std::cout << i << " -> "; 
        for(int u : SG[i]) {
            std::cout << u << " "; 
        }
        std::cout << std::endl;
    }

}


void State_graph::display_SG_detail() const {

    std::cout << "--- AFFICHAGE GRAPHE D'ÉTATS ---" << std::endl;

    for(int i = 0; i < (int)SG.size(); ++i) {
        
        std::vector<int> cand = get_cand(i); // récup candidats ID = i
        
        std::cout << "{ "; // affichage candidats départ 
        for(int u : cand) 
            std::cout << u << " "; 
        std::cout << "} : "; 

        for(int u : SG[i]) { // pour tout ID voisin de i

            std::vector<int> cand2 = get_cand(u); // récup l'ensemble associé 

            std::cout << "{ "; // affichage candidats arrivée 
            for(int u : cand2) 
                std::cout << u << " "; 
            std::cout << "} ";

        }

        std::cout << std::endl;
    }
}


