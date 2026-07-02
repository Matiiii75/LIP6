#include <vector>
#include <random> 
#include <iostream> 
#include <cassert> 
#include <string> 
#include <fstream>
#include <queue>
#include <algorithm>
#include <bitset>

struct Dag 
{
    std::vector<std::vector<int>> dag; 
    int nb_nodes; // en comptant source & puit
    int nb_arcs;  // ici aussi 
    int degenerascy; 
    int HG_nb_edge; 

    std::vector<std::bitset<5001>> TC; // transitive closure 
    
    // note pour le bitset<2001>. Plutôt que d'avoir une TC
    // sous forme de matrice binaire, on a un vectuer de bitsets
    // qui sont des tableaux de bits (ici 2001) bits au plus
    // qui sont contigus en mémoire (gain massif de temps)
    // la taille des bit sets doit malheuresement être choisie a l'avance. 
    // donc, comme je ne prévois pas de générer des instances de plus de 2000
    // sommets pr l'instant, j'ai choisis 2001 

    std::vector<std::vector<int>> HG; // graphe de co-comparabilité 

    // constructeur 
    Dag(int size); 

    // calcule la transitive closure du dag 
    void compute_transitive_closure(); 

    // écrit le graphe dans un fichier .txt 
    // ID permet de différencier des instances qui auraient les mm n & p
    void write_in_file(int ID) const; 

    // calcule le graphe de co-comparabilité du dag 
    void compute_HG(); 

    // calcule la degenerascy de HG 
    void compute_degenerascy(); 

    // génère aléatoirement un dag
    void gen_graph(double p); 

    // afficher les infos relatives au dag généré ainsi 
    void display_infos() const; 

    // afficher le dag 
    void display_dag() const; 

    // afficher graphe de co-comparabilité 
    void display_HG() const; 
}; 

// fonction qui tire aléatoirement un réel dans [a,b]
double random_double(double a, double b); 
