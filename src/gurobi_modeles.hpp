#pragma once 

#include "Data.hpp" 
#include "gurobi_c++.h"


struct Gurobi_modeles {

    const Data& data; // données du probleme
    
    std::vector<int> opt_order; 
    double time_limit; 

    // [ATTRIBUTS RELATIFS AUX RÉSULTATS DE L'AGORITHME]

    bool found_solution = false; // true si gurobi a trouvé au moins une solution  
    double solve_time; 
    double obj_val; 
    double best_bound; 
    double mip_gap; 

    // Constructeur : lancer la résolution avec le choix de l'utilisateur. 
    Gurobi_modeles(const Data& _data, double _time_limit): data(_data), time_limit(_time_limit) {}

    /* DAG SUM CUT */

    // éxécute le modele basé sur les positions 
    void modele_positions_DSC(); 

    // éxécute le modele basé sur les positions relatives 
    void modele_positions_relatives_DSC(bool relaxation); 

    // éxécute le modèle basé sur les position membership
    void modele_membership_DSC(); 

    /* DAG CUTWIDTH */

    // éxécute le modèle basé sur les positions membership 
    // (variable xui = 1 si phi(u) <= i)
    void modele_membership_positions_CW(); 

    // affiche les résultats de l'algorithme 
    void display_infos() const;

}; 

