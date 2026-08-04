#pragma once 

#include "Data.hpp" 


/**
 * @brief fonction qui teste l'inclusion de v1 dans v2 - {t} (car t compte pas dans l'objectif)
 * @param v1 le vecteur qui est inclus 
 * @param v2 vecteur de uint8_t où v2[u] = 1 si u /in cut-set, 0 sinon  
 * @param t le noeud puit qu'on ne considère pas dans le calcul
 * @return true si inclus, false sinon
 * @note on mettra t = -1 par défaut pour dire qu'on regarde juste une inclusion sans chercher à exclure le puit t 
 */
bool is_included(const std::vector<int>& v1, const std::vector<uint8_t>& v2, int t); 


/**
 * @brief détermine si il existe un successeur de gamma qui n'est pas dans le cut set
 * (l.8,9 - algo 2)
 * @param succ_gamma vecteur contenant les successeurs de gamma 
 * @param t le noeud puit du dag (à ne pas considérer dans les calculs justement)
 * @param cut_set vecteur de uint8_t où v2[u] = 1 si u /in cut-set, 0 sinon
 * @return vrai si il existe un successeur, faux sinon 
 * @note version où le cut_set est un unordered_set
 */
bool is_disjoint(const std::vector<int>& succ_gamma, int t, 
    const std::vector<uint8_t>& cut_set); 


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

// prend en entrée un chemin jusqu'à une instance et ressort juste le nom de l'instance 
std::string getFileName(const std::string& path); 

/**
 * @brief Ecrit les données obtenues par l'algo de complexité paramétrée
 * dans un fichier texte. 
 * @param path_to_write chemin vers le fichier où écrire 
 * @param inst le nom de l'instance 
 * @param dag_size taille du dag
 * @param degeneracy degeneracy du graphe de co-comparabilité du dag
 * @param val_opt la valeur optimale
 * @param time le temps total nécéssité (calcul TC + main algo)
 * @param nb_nodes_SG le nombre de sommets dans le graphe d'états construit 
 */
void write_main_infos(const std::string& path_to_write, const std::string& inst, int dag_size, int degeneracy, int val_opt, double time, int nb_nodes_SG); 

/**
 * @brief Ecrit les données obtenues par le recuit simulé 
 * dans un fichier texte. 
 * @param path_to_write chemin vers le fichier où écrire 
 * @param inst le nom de l'instance 
 * @param dag_size taille du dag
 * @param degeneracy degeneracy du graphe de co-comparabilité du dag
 * @param val_opt la valeur optimale
 * @param time le temps total nécéssité (calcul TC + main algo)
 * @param temperature la température choisie 
 * @param nb_iter_max nombre d'itérations max par pallier de température 
 */
void write_SAA_results(
    const std::string& path_to_write, 
    const std::string& inst, int dag_size, 
    int degeneracy, int val_opt, double time, 
    double temperature, int nb_iter_max
); 


