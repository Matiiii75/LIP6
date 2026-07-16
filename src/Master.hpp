#pragma once 

#include "Data.hpp"
#include "State_graph.hpp"
#include "common.hpp"
#include "Timer.hpp"


struct Master {

    const Data& data; // ref constante vers data 

    State_graph SG; 

    std::queue<int> L; // liste FIFO pour stocker les ensembles candidats à traiter

    std::vector<int> best_dist; // pr stocker les pcc jusqu'à l'ID défini par l'index du vecteur

    // pr stocker les preds dans le pcc 
    // pred_in_pcc[i] = {pred(i),candidat ajouté}
    std::vector<std::pair<int,int>> pred_in_pcc; 

    /* ATTRIBUTS RELATIFS AUX RÉSULTATS DE L'ALGORITHME */

    std::vector<int> optimal_order; // contiendra l'ordre topologique optimal
    int optimal_value; // contiendra la valeur optimale associée à l'ordre optimal calculé
    int nb_hash_generated; 
    int nb_candidats; 

    double time_limit; // limite choisie par l'utilisateur 
    Timer master_time_data; // permettra de vérifier que l'algorithme ne mets pas trop de temps 
    bool found_solution = false; // indique si on a trouvé une solution (faux par défaut)

    /**
     * @brief constructeur de Master : établit une ref const vers data, 
     *  initialise SG et ajoute l'ID du premier ens. candidats dans la FIFO 
     * @param _data Les données du pb pré-calculées 
     * @param _s le sommet de degré entrant 0 du dag initial
     * @param _t le sommet de degré sortant 0 du dag initial
     * @param _time_limit la limite de temps imposée par l'utilisateur 
     */
    Master(const Data& _data, int _s, int _t, double _time_limit);

    /**
     * @brief calcul le cut set associé à un ensemble candidat
     * @param cand candidat pour lequel on recherche le cut set 
     * @return unordered_set contenant le cut set 
     */
    std::unordered_set<int> compute_cut_set(const std::vector<int>& cand) const; 


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


    /**
     * @brief une fois l'algorithme terminé, extrait les résultats tels que : 
     * - l'ordre topologique optimal
     * - la valeur optimale associée 
     * - le nombre de hash générés 
     * - le nombre de candidats 
     */
    void extract_results();  


    /**
     * @brief Affiche les résultats calculés, tels que : 
     * - l'ordre topologique optimal
     * - la valeur optimale associée 
     * - le nombre de hash générés 
     * - le nombre de candidats 
     */
    void display_results(bool display_opt_order, bool display_opt_val, bool hash_infos) const; 

}; 






