#include "Master.hpp"
#include "Heuristics.hpp"
#include "Timer.hpp"

// éxécute SAA sur une instance donnée 
// calcule et affiche la valeur obtenue 
// n'écrit rien dans un fichier 
void run_SAA(Data& data, bool write_results) {

    double initial_temp = -1; // -1 -> appeler méthode init_temp()
    Heuristics h(data, initial_temp, 100000); // partie heuristique SAA
    h.SAA_optimize(); // lancement de l'algo 

    bool display_inst_name = true; 
    bool display_n_and_k = true; 
    bool display_order_found = false; 
    bool display_best_val = true; 
    bool display_time = true; 
    bool display_temperature = true;  
    bool display_nb_iter_max = true; 

    h.display_results( // affichage des données voulues 
        display_inst_name, 
        display_n_and_k,   
        display_order_found, 
        display_best_val,
        display_time,  
        display_temperature, 
        display_nb_iter_max
    ); 

    if(write_results) { // écriture dans un fichier si voulu

        write_SAA_results(
            "results/results_SAA.txt",
            h.data.instance_name, 
            h.data.dag_size, 
            h.data.degenerascy,
            h.obj_val,
            h.total_time,
            h.initial_temp,
            h.nb_iter_max
        );

    }

}

// éxécute l'algo de complexité paramétrée pour une instance donnée 
// affiche l'ordre topo optimal, la valeur associée, check la validité 
// et affiche le nombre de sommets de SGG généré, ainsi que le nombre de hash différents 
// n'écrit rien dans un fichier 
void run_param_comp_algo(Data& data, bool write_results, bool enable_LB2_elaging) {

    // la source c'est 0, le puit c'est le dernier sommet du dag (par défaut)
    int source = 0; 
    int puit = data.dag_size - 1; 
    double time_limit = 600.00; 

    Master prog(data, source, puit, time_limit, enable_LB2_elaging);
    prog.build_SG_DSC(); // lancement de la construction de l'algorithme 

    int nb_elaged_nodes = -1; // par défaut, on considère l'élagage désactivé 
    std::string path_to_write = "results/results_algo.txt"; // par défaut (fichier pour élagage OFF)
    if(enable_LB2_elaging) {
        path_to_write = "results/results_algo_LB2_ON.txt"; 
        nb_elaged_nodes = prog.nb_elaged_branch_by_LB2_DSC; // si on l'a activé -> on récup
    }   
    if(prog.found_solution) {

        prog.extract_results();
        
        bool display_inst_name = true; 
        bool display_n_and_k = true; 
        bool display_opt_order = false; 
        bool display_time = true; 
        bool display_opt_val = true; 
        bool display_hash_infos = true; 
        bool display_LB2_elaging_infos = true; 

        prog.display_results( // affichage du résultat 
            display_inst_name,
            display_n_and_k, 
            display_opt_order,
            display_time, 
            display_opt_val,
            display_hash_infos,
            display_LB2_elaging_infos
        ); 

        if(write_results) { // écriture dans un fichier 

            write_main_infos(
                path_to_write,
                prog.data.instance_name,
                prog.data.dag_size, 
                prog.data.degenerascy, 
                prog.optimal_value, 
                prog.total_time,
                prog.nb_candidats, 
                nb_elaged_nodes
            );
        }

    } else {

        std::cout << "Arrêt algorithme : time_limit excedée" << std::endl;

        if(write_results) { // si on souhaite écrire dans un fichier 

            int optimal_value = -1; 
            int nb_candidats = prog.SG.ID_to_cands.size(); // on récup la taille de SG lors de l'arrêt

            write_main_infos(
                path_to_write,
                prog.data.instance_name, 
                prog.data.dag_size, 
                prog.data.degenerascy, 
                optimal_value, 
                prog.total_time, 
                nb_candidats,
                nb_elaged_nodes
            );

        }

    }

}

// arg 0 -> ./prog 
// arg 1 -> nom de l'instance 
// arg 2 -> 0 si on lance juste l'algo de complexité paramétrée, 
//          1 si on lance juste le SAA 
//          2 si on lance SAA + algo complexité paramétrée 
// arg 3 -> 0 si on active l'élagage avec la borne LB2 
//          1 sinon
// arg 4 -> 0 si on désactive l'écriture des résultats dans dossier results 
//          1 si on active 
int main(int argc, char* argv[]) {

    if(argc != 5) 
        throw std::runtime_error("Nombre d'arguments fournis en arguments incorrect"); 

    std::string file = argv[1]; 
    int mode_execution = atoi(argv[2]); // 0 -> algo de complexité param | 1 -> SAA | 2 -> algo et SAA
    int elaging_LB2_choice = atoi(argv[3]); // 0 -> sans élagage | 1 -> avec élagage 
    int writing_results = atoi(argv[4]); // 0 -> pas d'écritures dans le fichier results | 1 -> écritures activées 

    if(mode_execution != 0 && mode_execution != 1 && mode_execution != 2) // gestions erreurs arguments 
        throw std::runtime_error("mode_execution (main argument) doit être 0,1 ou 2"); 
    if(elaging_LB2_choice != 0 && elaging_LB2_choice != 1)
        throw std::runtime_error("elaging_LB2_choice (main argument) doit être 0 ou 1"); 
    if(writing_results != 0 && writing_results != 1)
        throw std::runtime_error("Writing results (main argument) doit être 0 ou 1"); 

    Data data(file); 

    switch (mode_execution) 
    {
        case 0: // lancement algo complexité param  
            run_param_comp_algo(data, writing_results, elaging_LB2_choice); 
            break; 
        case 1: // lancement SAA seul 
            run_SAA(data, writing_results);
            break;  
        case 2: // lancement SAA + algo complexité param
            run_SAA(data, writing_results); 
            run_param_comp_algo(data, writing_results, elaging_LB2_choice); 
            break; 
        default: 
            throw std::runtime_error("main: pb dans le switch -> param choisis incorrect"); 
    }

    return 0; 
}


#include <filesystem>

namespace fs = std::filesystem; 

std::vector<std::string> list_text_files(const std::string& folder_path) {
    std::vector<std::string> text_files;

    if (!fs::exists(folder_path) || !fs::is_directory(folder_path)) {
        return text_files;
    }

    for (const auto& entry : fs::directory_iterator(folder_path)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            text_files.push_back(entry.path().string());
        }
    }

    return text_files;
}

// un simple main pour lancer le calcul de LB2 sur toutes les instances pour un cut set vide a chaque fois
// int main()
// {

//     std::vector<std::string> all_inst = list_text_files("../instances/"); 

//     for(std::string& inst : all_inst) {

//         Data data(inst); 
//         State_graph SG(data, 0, data.dag_size-1); 
        
//         int LB2 = 0; 
//         for(int i = 0; i < (int)SG.taille_blocages_hors_cut.size(); ++i) {
//             LB2 += SG.taille_blocages_hors_cut[i]; 
//         }

//         std::cout << "valeur de la borne : " << LB2 << std::endl;

//         // écriture dans un fichiers text dans results/

//         std::string path_to_write = "../results/results_LB2_empty_set.txt"; 
//         std::ofstream writing(path_to_write, std::ios::app); 
//         writing << data.instance_name << " ";
//         writing << data.dag_size << " "; 
//         writing << data.degenerascy << " "; 
//         writing << LB2 << std::endl;
//         writing.close(); 

//     }

//     return 0; 
// }
