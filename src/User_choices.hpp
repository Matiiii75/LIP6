#pragma once 

#include "Data.hpp"

struct User_choices {

    bool elaging_LB2_ON; 
    double elaging_LB2_percentage; 

    /**
     * @brief Constructeur. Il initialise les attributs sur des valeurs par défaut 
     */
    User_choices() : elaging_LB2_ON(false), elaging_LB2_percentage(-1.0) {}

    /**
     * @brief Set l'attribut 'elaging_LB2_ON' sur 'true'
     */
    void set_elaging_LB2_ON(); 

    /**
     * @brief Set l'attribut 'elaging_LB2_percentage'. 
     * @throw Erreur si _elaging_LB2_percentage n'est pas dans [0.0,1.0]
     */
    void set_elaging_LB2_percentage(double _elaging_LB2_percentage); 

}; 