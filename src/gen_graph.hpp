#include <vector>
#include <random> 
#include <iostream> 
#include <cassert> 
#include <string> 
#include <fstream>
#include <queue>
#include <algorithm>

struct Dag 
{
    std::vector<std::vector<int>> dag; 
    int nb_nodes; // en comptant source & puit
    int nb_arcs;  // ici aussi 
    int degenerascy; 
    int HG_nb_edge; 

    std::vector<std::vector<bool>> TC; // transitive closure 
    std::vector<std::vector<int>> HG; // graphe de co-comparabilité 

    // constructeur 
    Dag(int size); 

    // calcule la transitive closure du dag 
    void compute_transitive_closure(); 

    // écrit le graphe dans un fichier .txt 
    void write_in_file() const; 

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
