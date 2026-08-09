#include "Master.hpp"
#include "Heuristics.hpp"
#include "Timer.hpp"

int main(int argc, char* argv[]) {

    if(argc != 2) 
        std::cout << "nb args incorrect" << std::endl;

    std::string inst = argv[1]; 
    Data data(inst); // import des data 

    Elaging_user_choice euc; 
    euc.set_params(0, 0.0); // choix par défaut 

    int source = 0; 
    int puit = data.dag_size - 1; 
    Master prog(data, source, puit, 600.0, euc); 

    prog.compute_composantes(); 
    std::cout << "AFFICHAGE COMPOSANTES : " << std::endl;
    for(int i = 0; i < (int)prog.composantes.size(); ++i) {
        std::cout << "{"; 
        for(int u : prog.composantes[i]) {
            std::cout << u << " "; 
        }
        std::cout << "}" << std::endl;
    }
    std::cout << "AFFICHAGE COMPOSANTES TERMINÉ" << std::endl;

    for(const auto& composante : prog.composantes) {
        std::vector<std::vector<int>> new_dag; 
        new_dag = create_dag_from_composante(data.dag, composante); 
        std::cout << "----- AFFICHAGE DAG -----" << std::endl;
        for(int i = 0; i < (int)new_dag.size(); ++i) {
            std::cout << i << " -> {"; 
            for(int u : new_dag[i]) {
                std::cout << u << " ";
            }
            std::cout << "}" << std::endl;
        }
        std::cout << "----- AFFICHAGE DAG TERMINÉ -----" << std::endl;
        std::cout << std::endl;
    }   

    std::vector<int> composante1 = prog.composantes[0]; 
    std::vector<std::vector<int>> new_dag; 
    new_dag = create_dag_from_composante(data.dag, composante1); 

    Data new_data(new_dag); 
    new_data.display_dag(); 

    Master new_prog(new_data, 0, new_data.dag_size-1, 600.0, euc); 
    new_prog.build_SG_DSC(); 
    
    if(new_prog.found_solution) {
        new_prog.extract_results(); 

        bool display_inst_name = false; 
        bool display_n_and_k = true; 
        bool display_opt_order = false; 
        bool display_time = true; 
        bool display_opt_val = true; 
        bool display_hash_infos = true; 
        bool display_LB2_elaging_infos = true;

        new_prog.display_results( // affichage du résultat 
            display_inst_name,
            display_n_and_k, 
            display_opt_order,
            display_time, 
            display_opt_val,
            display_hash_infos,
            display_LB2_elaging_infos
        ); 
    } else {
        std::cout << "AUCUN SOLUTION TROUVÉE HUMMMM" << std::endl;
    }

    return 0; 
}