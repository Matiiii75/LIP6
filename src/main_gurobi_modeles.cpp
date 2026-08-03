#include "gurobi_modeles.hpp"

int main(int argc, char* argv[]) {

    if(argc != 4) 
        throw std::runtime_error("Expected 4 args in main"); 

    int algo_choice = atoi(argv[2]); // 0 : positions | 1 : position relatives
    int lazy_cuts_on = atoi(argv[3]); // 0 : no lazy | 1 : avc lazy

    std::string instance = argv[1]; 
    Data d(instance); 

    Gurobi_modeles Gm(d, 0, 600.0); 

    if(algo_choice == 0)
        Gm.modele_positions(); 
    else 
        Gm.modele_positions_relatives(lazy_cuts_on, false); 

    return 0; 
}

