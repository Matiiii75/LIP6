#pragma once 

#include "Data.hpp" 
#include "gurobi_c++.h"


struct Gurobi_modeles {

    const Data& data; // données du probleme
    
    std::vector<int> opt_order; 
    int opt_val; 

    bool enable_lazy_cuts; 
    double time_limit; 

    Gurobi_modeles(
        const Data& _data, 
        bool _enable_lazy_cuts,
        double _time_limit
    ); 


    void modele_positions(); 


    void modele_positions_relatives(bool lazy_cuts_on); 

}; 


class My_callbacks : public GRBCallback {

    private: 

    int puit; 
    const std::vector<int>& all_nodes; 
    const std::vector<std::vector<GRBVar>>& z; 
    int nb_cuts_added; 
    
    public: 

    My_callbacks(int _puit, const std::vector<int>& _all_nodes, const std::vector<std::vector<GRBVar>>& _z); // constructeur 

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

