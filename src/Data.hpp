#pragma once 

#include <iostream>
#include <vector>
#include <unordered_map> 
#include <unordered_set>
#include <string>
#include <random> 
#include <fstream> 
#include <algorithm> 
#include <queue>

using Dag = std::vector<std::vector<int>>;

/**
 * @brief structure qui représente un bit128 en deux parties de 64 bits
 * @param l les 64 bits de gauche du bit128
 * @param r les 64 bits de droite du bit128
 * @note elle est munie d'operator (==) & (^) pour pouvoir comparer des bit128
 *  et faire le XOR 
 */
struct keyHash {

    // l : partie gauche du nombre 
    // r : partie droite du nombre
    std::uint64_t l, r; 

    // définition de l'opérateur == (comparaison)
    bool operator==(const keyHash& other) const; 

    // définition de l'opérateur XOR 
    keyHash operator^(const keyHash& other) const; 

    // operateur d'affichage d'un keyHash
    std::ostream& operator<<(std::ostream& os) const; 

}; 


/**
 * @brief genere un keyhash aléatoire
 * @param moteur c'est le generateur aléatoire qu'on a créé avant
 *  et qui produit rapidement un entier encodé sur 64 bits
 * @return keyhash généré aléatoirement 
 */
keyHash randomGenerator(std::mt19937_64& moteur); 


struct Data {

    Dag dag; // dag initial
    Dag reverse_dag; // reverse_dag (on inverse le sens des arcs)

    int dag_size; 
    int nb_arcs; 
    
    std::vector<std::vector<bool>> TC; // transitive closure du dag initial
    std::vector<keyHash> node_to_hash; // à chaque noeud du dag, on associe un hash 

    Data(const std::string& file); // constructeur 

    void compute_transitive_closure(); // calcule la transitive closure (ON PEUT AMELIORER)
    void compute_node_to_hash(); // calcule les hash de chaque noeud du dag 
    
    void display_dag() const; // affichage dag et reverse dag 
    void display_reverse_dag() const; 

}; 









