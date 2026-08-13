#include "gurobi_modeles.hpp"

// permet de lancer la résolution avec gurobi
// on doit donner l'instance, le choix de l'algo (position ou position relatives)
// ainsi que le choix d'activer les lazy-cuts pr position relative ou non
int main(int argc, char* argv[]) {

    if(argc != 5) 
        throw std::runtime_error("Expected 4 args in main"); 

    int algo_choice = atoi(argv[2]); // 0 : positions | 1 : position relatives
    int lazy_cuts_on = atoi(argv[3]); // 0 : no lazy | 1 : avc lazy
    int writing_results = atoi(argv[4]); // 0 : pas d'écritures dans fichier res | 1 : on écrit 

    std::string instance = argv[1]; 
    std::string file_name = getFileName(instance); 
    Data d(instance); 

    Gurobi_modeles Gm(d, algo_choice, lazy_cuts_on, 600.0); 
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
            lazy_cuts_on,
            val_opt, 
            Gm.solve_time, 
            gap, 
            best_bound, 
            Gm.found_solution
        ); 

    }

    return 0; 
}

