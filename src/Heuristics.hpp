#pragma once 

#include "Data.hpp"
#include "common.hpp"

/**
 * @brief structure qui mémorise les deux indexs à swap 
 */
struct BestSwap 
{
    int indx_l; 
    int indx_r; 
    int delta; 
    bool found_move; 

    BestSwap() : indx_l(-1), indx_r(-1), delta(-1), found_move(false) {} // constructeur par defaut
}; 


struct Heuristics {

    int obj_val; 

    const Data& data; // ref vers les datas

    std::vector<int> ordre_to_node; // vecteur qui associe à chq position son noeud

    std::vector<int> node_to_ordre; // vecteur qui associe à chq noeud sa position 

    std::vector<std::vector<int>> node_order_matrix; // matrice tq m[u][i] = nb de succ directs de u STRICTEMENT apres l'ordre i 

    /**
     * @brief constructeur : trouve un ordre topologique de départ, compute l'order matrix (MP dans le pseudo-code)
     * @param _data : ce sont les données du problème stockées dans la classe Data
     */
    Heuristics(const Data& _data); 

    /**
     * @brief trouve un ordre topologique valide initial 
     * @return un vecteur où vec[i] = sommet en i-eme position dans l'ordre 
     * @warning le dag donné doit avoir un unique sommet de degré entrant 0 (s)
     */
    void init_ordre_topo(); // fonction qui trouve un ordre topologique dans le dag 


    /**
     * @brief calcule une matrice M telle que M[u][i] = nombre de succ directs de u strictement après l'ordre i
     */
    void compute_node_order_matrix(); 


    /**
     * @brief calcule l'objectif DSC grâce à la matrice node_order_matrix 
     */
    int compute_DSC_obj() const; 

    
    void display_node_order_matrix() const; 
    
    
    /* FONCTIONS POUR RECUIT SIMULÉ DSC */

    /**
     * @brief vérifie la validité d'un mouvement
     * @param indx l'indice i de l'échange ui <-> ui+1
     * @return true si échange ok, false sinon
     * @note renvoie false si indx == 0 ou == dag.size-1
     * Utilise node_order_matrix pour identifier si ils sont voisins 
     */
    bool is_move_valid(int indx) const; 


    /**
     * @brief calcule la diff d'objectif lié au swap ui/ui+1 
     * @param indx l'index "i" du ui/ui+1 
     * @return delta 
     * @note delta < 0 -> mouvement méliorant, > 0 -> empirant 
     * @warning lève une erreur si indx == 0
     */
    int compute_delta_DSC(int indx) const; 

    
    /**
     * @brief appeler si on applique le mouvement ui/ui+1
     * @param indx c'est l'indice i 
     * @param delta le delta calculé pour maj obj_val
     * @return modifie node_order_matrix, ordre_to_node, node_to_ordre & obj_val
     */
    void apply_move(const BestSwap& bs); 


    /**
     * @brief critère de métropolis pour le recuit simulé 
     * @param temp température courante 
     * @param delta différence entre la valeur d'objectif courante et la valeur après mouvement 
     * @param gen moteur d'aléatoire créé dans le recuit simulé 
     * @return true si on accepte le mouvement, false si on le rejette 
     */
    bool metropolis(double temp, int delta, std::mt19937& gen) const; 


    /**
     * @brief lance l'optimisation du problème par le recuit simulé
     * @param temp la température initiale
     * @param iter_max nombre d'itérations pour chaque pallier de température 
     * @return un ordre topologique stocké en attribut de Heuristics:: et sa valeur objectif associée 
     */
    void SAA_optimize(double temp, int iter_max); 
    
}; 