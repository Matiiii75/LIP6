#include "gurobi_modeles.hpp"

// permet de lancer la résolution avec gurobi
// on doit donner l'instance, le choix de l'algo (position ou position relatives)
// ainsi que le choix d'activer les lazy-cuts pr position relative ou non
int main(int argc, char* argv[]) {

    if(argc != 4) 
        throw std::runtime_error("Expected 3 args in main"); 

    int algo_choice = atoi(argv[2]); // 0 : positions DSC | 1 : position relatives DSC | 2 : positions membership CW 
    int writing_results = atoi(argv[3]); // 0 : pas d'écritures dans fichier res | 1 : on écrit 

    std::string instance = argv[1]; 
    std::string file_name = getFileName(instance); 
    Data d(instance); 
    double time_limit = 600.00; 

    Gurobi_modeles Gm(d, time_limit); 

    if(algo_choice == 0) {
        Gm.modele_positions_DSC(); 
    }
    else if(algo_choice == 1) {
        bool relaxation_ON = false; 
        Gm.modele_positions_relatives_DSC(relaxation_ON); 
    }
    else if(algo_choice == 2) {
        Gm.modele_membership_positions_CW(); 
    }
    else if(algo_choice == 3) {
        Gm.modele_membership_DSC(); 
    }

    Gm.display_infos(); // affichage des informations 

    if(writing_results) {

        double val_opt = -1; 
        double gap = -1;        // ce sont des valeurs par défaut 
        double best_bound = -1; // dans le cas où on a pas trouvé de solution du tt 

        if(Gm.found_solution) { // si on a trouvé une solution 

            val_opt = Gm.obj_val; 
            gap = Gm.mip_gap; 
            best_bound = Gm.best_bound; 

        }

        write_gurobi_results(
            "results/results_gurobi.txt",
            file_name, 
            d.dag_size, 
            d.degenerascy, 
            algo_choice,
            val_opt, 
            Gm.solve_time, 
            gap, 
            best_bound, 
            Gm.found_solution
        ); 

    }

    return 0; 
}

