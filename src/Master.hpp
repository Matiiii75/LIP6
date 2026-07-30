#pragma once 

#include "Data.hpp"
#include "State_graph.hpp"
#include "Heuristics.hpp"
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
    
    int SAA_value; 

    std::unordered_map<int,int> lower_bounds_2; // dico qui associe a chaque C_ID sa LB2 si elle existe 

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
     * @brief calcule et set la borne LB2 du sommet initial du graphe d'états. 
     * La valeur de la borne inférieure sur ce sommet est tout simplement la somme
     * des |Phi(u)| pour tout u \in V-{s,t}. 
     * @note par défaut dans data, taille_blocages_hors_cut[s] ou [t] = 0
     */
    void set_first_cand_LB2(); 


    /**
     * @brief calcule cut_set, hors_cut_set et la taille de cut_set (tous passés par référence)
     * @param cand candidat pour lequel on recherche le cut set 
     * @param cut_set_size passé par référence, pour recupérer la taille du cut_set
     * @param cut_set vecteur de 0/1 passé en référence
     * @param hors_cut_set vecteur passé en référence qui contiendra les éléments d'hors-cut-set
     */
    void compute_cut_set(const std::vector<int>& cand, int& cut_set_size, 
        std::vector<uint8_t>& cut_set, std::vector<int>& hors_cut_set) const; 


    /**
     * @brief Calcule la différence de LB2 entre deux sommets adjacents dans SG. 
     * Le sommet de départ est définit par cut_set - {gamma} et le sommet d'arrivée par
     * cut_set. 
     * @param gamma c'est le candidat qu'on vient de faire rentrer dans cut_set
     * @param cut_set c'est le cut_set associé au sommet d'arrivée 
     * @param hors_cut_set vecteur contenant les éléments de V-S pr parcours par index + rapide
     */
    int compute_delta_LB2(int gamma, const std::vector<uint8_t>& cut_set, 
        const std::vector<int>& hors_cut_set) const; 
    

    /**
     * @brief Calcule la borne LB2 du sommet C_ID de SG. Il s'agit d'une version incrémentale qui se sert de la 
     * borne LB2 calculé pour C_pred, le prédécesseur de C_ID dans SG (le sommet C_pred est celui d'où on vient
     * dans SG pour avoir la meilleur valeur en C_ID). 
     * @param C_ID id du sommet de SG pour lequel on calcule la borne
     * @param cut_set le cut_set associé à l'ensemble candidat C_ID
     * @param hors_cut_set un vecteur qui recence les éléments hors du cut set pour un parcours optimisé de ceux-ci
     * @warning la borne inférieure LB2 de C_pred doit avoir été calculée
     * @throw erreur si le predecesseur de C_ID n'as pas de borne LB2 enregistrée 
     */
    int compute_C_LB2(int C_ID, const std::vector<uint8_t>& cut_set, const std::vector<int>& hors_cut_set) const; 


    /**
     * @brief Lance le calcul de la borne LB2 et vérifie si un élagage est possible depuis le noeud C_ID. 
     * @param C_ID noeud depuis lequel on élague
     * @param cut_set cut-set associé à l'ensemble candidat C_ID
     * @param hors_cut_set vecteur stockant les éléments hors-cut-set (parcours simplifié)
     * @return true si on élague, false sinon 
     */
    bool try_elaging_LB2(int C_ID, const std::vector<uint8_t>& cut_set, const std::vector<int>& hors_cut_set); 


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






