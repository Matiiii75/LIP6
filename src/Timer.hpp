#pragma once 

#include <iostream>
#include <chrono> 

// ce fichier permet la gestion du temps comme : 
// - démarrer un minuteur
// - récupérer le temps courant 

struct Timer {

    std::chrono::steady_clock::time_point start; 

    // démarrer un minuteur 
    void start_timer(); 

    // récuperer le temps passé depuis le début 
    double get_temps_passe(); 

};  



