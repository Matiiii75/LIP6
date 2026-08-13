#pragma once 

#include "Data.hpp" 
#include "gurobi_c++.h"


struct Gurobi_modeles {

    const Data& data; // données du probleme
    
    std::vector<int> opt_order; 

    bool choix_user; // 0 = modele position | 1 modele positions relatives
    bool enable_lazy_cuts; 
    double time_limit; 

    // [ATTRIBUTS RELATIFS AUX RÉSULTATS DE L'AGORITHME]

    bool found_solution = false; // true si gurobi a trouvé au moins une solution  
    double solve_time; 
    double obj_val; 
    double best_bound; 
    double mip_gap; 

    // Constructeur : lancer la résolution avec le choix de l'utilisateur. 
    Gurobi_modeles(
        const Data& _data,
        bool _choix_user,  
        bool _enable_lazy_cuts,
        double _time_limit
    ); 

    // éxécute le modele basé sur les positions 
    void modele_positions(); 

    // éxécute le modele basé sur les positions relatives 
    void modele_positions_relatives(bool lazy_cuts_on, bool relaxation); 

    // affiche les résultats de l'algorithme 
    void display_infos() const;

}; 


class My_callbacks : public GRBCallback {

    private: 

    int puit; 
    const std::vector<std::vector<GRBVar>>& z; 
    int nb_cuts_added; 
    
    public: 

    My_callbacks(int _puit, const std::vector<std::vector<GRBVar>>& _z); // constructeur 

    protected: 

    struct violated_cut {
        int u; 
        int v;  // Pour retenir les contraintes violée (et donc les coupes à ajouter)
        int w; 
        violated_cut(int _u, int _v, int _w) : u(_u), v(_v), w(_w) {}
    }; 
    
    void handle_approx(double& X) const; 

    void callback() override; // l'override est là pour dire au compilateur "hé, il s'agit de la déclaration d'une fonction qui existe dans la classe mère GRBCallback"

    int get_nb_cuts_added() const; 

}; 

