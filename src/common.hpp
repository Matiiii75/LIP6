#pragma once 

#include "Data.hpp" 


/**
 * @brief fonction qui teste l'incusion d'un vecteur dans un autre
 * @param v1 le vecteur qui est inclus 
 * @param v2 le vecteur qui contient
 * @return true si inclus, false sinon
 * @warning les vecteurs doivent être triés dans l'ordre croissant 
 */
bool is_included(const std::vector<int>& v1, const std::vector<int>& v2); 


/**
 * @brief détermine si il existe un successeur de gamma qui n'est pas dans le cut set
 * (l.8,9 - algo 2)
 * @param succ_gamma 
 * @param C
 * @param TC
 * @return vrai si il existe un successeur, faux sinon 
 */
bool is_disjoint(const std::vector<int>& succ_gamma, 
    const std::vector<int>& C, const std::vector<std::vector<bool>>& TC); 


/**
 * @brief trouve le gamma (l.4 - algo 2)
 * @param v1 candidat départ (K dans algo 2)
 * @param v2 candidat d'arrivée (C dans algo 2)
 * @return gamma ou -1 si pas trouvé
 * @warning return -1 devrait jamais arriver -> ajouter gestion erreur dans code appellant 
 */
int find_gamma(const std::vector<int>& v1, const std::vector<int>& v2); 


/**
 * @brief calcule le hash d'un ensemble de candidats (noeud du graphe d'états)
 * @param cand l'ensemble candidat 
 * @param node_to_hash structure qui associé à chaque noeud du dag un hash 
 * @return hash de l'ensemble candidats
 */
keyHash compute_cand_hash(const std::vector<int>& cand, const std::vector<keyHash>& node_to_hash); 

// simple tri croissant d'un vecteur (library algorithm)
void increase_sort_vector(std::vector<int>& vec); 


// fonction d'affichage de debug 
void debug(const std::string& s); 


// fonction d'affichage d'un vecteur 
void display_vec(const std::vector<int>& v); 


// fontion d'affihage d'une liste FIFO
void display_FIFO(std::queue<int> Q); 



