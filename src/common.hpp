#pragma once 

#include "Data.hpp" 


/**
 * @brief fonction qui teste l'incusion de v1 dans v2 - {t} (car t compte pas dans l'objectif)
 * @param v1 le vecteur qui est inclus 
 * @param v2 le vecteur qui contient
 * @param t le noeud puit qu'on ne considère pas dans le calcul
 * @return true si inclus, false sinon
 * @warning les vecteurs doivent être triés dans l'ordre croissant 
 * @note on mettra t = -1 par défaut pour dire qu'on regarde juste une inclusion sans chercher à exclure le puit t 
 * @note c'est la version où v1 et v2 sont deux vecteurs triés
 */
bool is_includedV2(const std::vector<int>& v1, const std::vector<int>& v2, int t); 


/**
 * @brief fonction qui teste l'incusion de v1 dans v2 - {t} (car t compte pas dans l'objectif)
 * @param v1 le vecteur qui est inclus 
 * @param v2 unordered_set qui contient le cut set 
 * @param t le noeud puit qu'on ne considère pas dans le calcul
 * @return true si inclus, false sinon
 * @warning les vecteurs doivent être triés dans l'ordre croissant 
 * @note on mettra t = -1 par défaut pour dire qu'on regarde juste une inclusion sans chercher à exclure le puit t 
 * @note c'est la version où v1 est un vecteur et v2 est un unordered_set 
 */
bool is_included(const std::vector<int>& v1, const std::unordered_set<int>& v2, int t); 


/**
 * @brief détermine si il existe un successeur de gamma qui n'est pas dans le cut set
 * (l.8,9 - algo 2)
 * @param succ_gamma 
 * @param t le noeud puit du dag (à ne pas considérer dans les calculs justement)
 * @param C
 * @param TC
 * @return vrai si il existe un successeur, faux sinon 
 * @note fonction que j'utilisais initialement, mais j'ai décidé de la recoder pour utiliser cut_set en mode unordered_set
 */
bool is_disjointV2(const std::vector<int>& succ_gamma, int t, 
    const std::vector<int>& C, const std::vector<std::vector<bool>>& TC); 

/**
 * @brief détermine si il existe un successeur de gamma qui n'est pas dans le cut set
 * (l.8,9 - algo 2)
 * @param succ_gamma vecteur contenant les successeurs de gamma 
 * @param t le noeud puit du dag (à ne pas considérer dans les calculs justement)
 * @param cut_set le cut_set de C' (S(C')) dans l'algo (l.7)
 * @return vrai si il existe un successeur, faux sinon 
 * @note version où le cut_set est un unordered_set
 */
bool is_disjoint(const std::vector<int>& succ_gamma, int t, 
    const std::unordered_set<int> cut_set); 


/**
 * @brief trouve le gamma (l.4 - algo 2)
 * @param v1 candidat départ (K dans algo 2)
 * @param v2 candidat d'arrivée (C dans algo 2)
 * @return gamma ou -1 si pas trouvé
 * @warning return -1 devrait jamais arriver -> ajouter gestion erreur dans code appellant 
 */
int find_gamma(const std::vector<int>& v1, const std::vector<int>& v2); 


// simple tri croissant d'un vecteur (library algorithm)
void increase_sort_vector(std::vector<int>& vec); 


// fonction d'affichage de debug 
void debug(const std::string& s); 


// fonction d'affichage d'un vecteur 
void display_vec(const std::vector<int>& v); 


// fontion d'affihage d'une liste FIFO
void display_FIFO(std::queue<int> Q); 


// fonction qui tire un entier aléatoire dans [a,b]
int random_int(int a, int b, std::mt19937& gen); 


/**
 * @brief Ecrit les données obtenues par l'algo de complexité paramétrée
 * dans un fichier texte dans : "results/data_main.txt"
 * @param inst le nom de l'instance 
 * @param val_opt la valeur optimale
 * @param time le temps total nécéssité (calcul TC + main algo)
 * @param nb_nodes_SG le nombre de sommets dans le graphe d'états construit 
 */
void write_main_infos(const std::string& inst, int val_opt, double time, int nb_nodes_SG); 

