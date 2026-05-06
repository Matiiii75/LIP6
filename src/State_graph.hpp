#pragma once 

#include "Data.hpp"
#include "common.hpp"


// keyHasher custom pour l'unordered_map de la struct "state_graph"
// lui permet de hasher les clés
struct KeyHasher { 

    std::size_t operator()(const keyHash& k) const {
        return static_cast<std::size_t>(k.l); 
    }

}; 


// operateur de comparaison custom entre deux clé pour l'um de struct "state_graph" 
struct KeyEqual {

    bool operator()(const keyHash& k1, const keyHash& k2) const {
        return k1.l == k2.l && k1.r == k2.r; 
    }

}; 


struct State_graph {

    int s, t; // pour mémoriser les sommets source et puit qu'on a ajouté 

    const Data& data; // reférence constante vers un objet data passé au constructeur (évite copie)

    std::vector<std::vector<int>> ID_to_cands; // associe à chaque ID son ensemble candidat 
    
    // associe à chaque keyHash un ID 
    // on donne deux choses supplémentaires à l'unordered map pour qu'elle sache quoi faire : 
    // KeyHasher : lui permet de Hasher nos keyHash (qui est une structure perso)
    // KeyEqual : lui permet de comparer deux keyHash 

    std::unordered_map<keyHash, std::vector<int>, KeyHasher, KeyEqual> hash_to_ID; 

    std::vector<std::vector<int>> SG; // liste d'adjacence du state_graph

    std::vector<int> weights; // les poids de chaque ID de candidats (par index)


    State_graph(const Data& _data, int _s, int _t); 

    /**
     * @brief vérifie si un ensemble candidat est déja dans SG 
     * @param cand ensemble candidat à vérifier 
     * @param cand_hash le hash de l'ensemble candidat (pré-calculé)
     * @return l'ID si cand est déjà dans SG, -1 sinon 
     * @warning cand doit être trié par ordre croissant
     */
    int is_cand_in_SG(const std::vector<int>& cand, const keyHash& cand_hash) const ; 


    /**
     * @brief calcule le poids des arcs sortant de l'ensemble candidats C 
     *  par récurrence avec le poids d'un prédécesseur K 
     * @param C_ID ID de l'ensemble candidat dont on calcule le poids des arcs sortant
     * @param K_ID un prédécesseur de C_ID
     * @param c le candidat de K qu'on a ajouté à S(C)
     * @param cut_set c'est le cut set associé à C (noté S(C) dans mon rapport)
     * @return le poids 
     */
    int compute_weight_C(int C_ID, int K_ID, int c, const std::vector<int>& cut_set) const ; 


    /**
     * @brief ajoute un ensemble candidat au graphe d'états 
     * @param cand l'ensemble candidat à ajouter 
     * @param cand_hash le hash pré-calculé de cand 
     * @warning on doit rentrer dans cette fonction QUE SI cand n'est pas déjà dans SG 
     * @note la fonction 
     */
    void add_cand_to_SG(const std::vector<int>& cand, const keyHash& cand_hash); 


    /**
     * @brief ajoute l'arc (C1,C2) d'un certain poids au graphe d'états 
     * @param cand1_ID le sommet départ 
     * @param cand2_ID le sommet arrivée 
     * @param weight le poids de l'arc 
     * @warning On doit avoir au préalable vérifié que cand1_ID et cand2_ID sont bien dans SG et ont bien le bon ID associé 
     */
    void add_arc_from_C1_to_C2(int cand1_ID, int cand2_ID); 


    /**
     * @brief trouve un pred de C dans SG
     * @param C_ID ID de C, l'ensemble candidat dont on cherche un predecesseur dans SG
     * @return ID du predecesseur de C_ID dans SG
     */
    int get_pred_C(int C_ID) const; 


    /**
     * @brief getter qui renvoie l'ensemble candidat associé à un ID 
     * @param ID l'ID voulu
     * @warning devrait pas arriver mais si l'index ID n'existait pas, lèvera une erreur 
     * @note ref const sur le type de renvoie pour éviter copie puisqu'on va juste le parcourir 
     */
    std::vector<int> get_cand(int ID) const; 


    /**
     * @brief set le poids d'un ID 
     * @param ID 
     * @param w le poids de ID 
     * @note lève une erreur si ID >= taille(weights)
     */
    void set_weight(int ID, int w); 


    // méthode d'affichage de SG dans le terminal 
    void display_SG() const; 


    // méthode d'affichage de SG avec les relations entre ensembles 
    void display_SG_detail() const; 


    // afficher les poids de chaque ensemble candidat
    void display_weights() const; 

}; 




