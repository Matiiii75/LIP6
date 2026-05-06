#pragma once 

#include "Data.hpp"
#include "State_graph.hpp"
#include "common.hpp"


struct Master {

    const Data& data; // ref constante vers data 

    State_graph SG; 

    std::queue<int> L; // liste FIFO pour stocker les ensembles candidats à traiter

    std::vector<int> best_dist; // pr stocker les pcc jusqu'à l'ID défini par l'index du vecteur

    // pr stocker les preds dans le pcc 
    // pred_in_pcc[i] = {pred(i),candidat ajouté}
    std::vector<std::pair<int,int>> pred_in_pcc; 

    /**
     * @brief constructeur de Master : établit une ref const vers data, 
     *  initialise SG et ajoute l'ID du premier ens. candidats dans la FIFO 
     * @param _data Les données du pb pré-calculées 
     * @param s le sommet de degré entrant 0 du dag initial
     */
    Master(const Data& _data, int _s, int _t);

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
     * @brief reconstruit l'ordre topologique optimal à partir de pred_in_pcc
     */
    std::vector<int> rebuild_opt_order() const; 


    /**
     * @brief vérifie la validité de l'ordre trouvé
     */
    bool checker_DSC(const std::vector<int>& ordre_topo, int val_found) const; 

}; 






