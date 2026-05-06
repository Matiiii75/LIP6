#pragma once 

#include "Data.hpp"
#include "State_graph.hpp"
#include "common.hpp"


struct Master {

    const Data& data; // ref constante vers data 

    State_graph SG; 

    std::queue<int> L; // liste FIFO pour stocker les ensembles candidats à traiter


    /**
     * @brief constructeur de Master : établit une ref const vers data, 
     *  initialise SG et ajoute l'ID du premier ens. candidats dans la FIFO 
     * @param _data Les données du pb pré-calculées 
     * @param s le sommet de degré entrant 0 du dag initial
     */
    Master(const Data& _data, int s);

    /**
     * @brief calcul le cut set associé à un ensemble candidat
     * @param cand candidat pour lequel on recherche le cut set 
     * @return vecteur contenant le cut set 
     */
    std::vector<int> compute_cut_set(const std::vector<int>& cand) const; 


    /**
     * @brief construit intégralement le graphe d'états SG 
     */
    void build_SG();
    
    
    /**
     * @brief cherche un ordre topologique sur les sommets de SG en vue 
     *  de résoudre un probleme de pcc (algo de bellman
     */
    std::vector<int> find_topological_order() const; 


    std::vector<int> solve(int& value) const; 

}; 






