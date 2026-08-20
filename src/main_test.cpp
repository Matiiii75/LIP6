#include "Master.hpp"
#include "Heuristics.hpp"
#include "Timer.hpp"

int main(int argc, char* argv[]) {

    if(argc != 2)
        std::cerr << "erreur -> nb main::args" << std::endl;
    
    std::string inst = argv[1]; 
    
    Data data(inst); 

    User_choices uc;
    uc.set_composantes_pre_treatment_ON(); // activé pré traitement 
    
    int source = 0; 
    int puit = data.dag_size-1; 
    double time_limit = 600.0; 

    Master prog(data, source, puit, time_limit, uc); 
    prog.solve_DSC_with_pre_treatment(); 

    // extraction des résultats obtenus avc pre traitement (A MODIFIER PLUS TARD)
    
    std::string path_to_write = "../results/results_pre_treatment.txt"; 
    std::string inst_name = getFileName(inst); 
    int val_opt = prog.optimal_value; 
    double time = prog.total_time; 
    int nb_composantes = prog.pre_treatment->nb_composantes; 
    int nb_pb_solved = prog.pre_treatment->nb_sub_problems_solved; 
    int nb_total_cands = prog.pre_treatment->total_cand_generated; 

    write_pre_treatment_results // écriture dans fichier résultat 
    (
        path_to_write, 
        inst_name, 
        data.dag_size,
        data.degenerascy,
        val_opt,
        time,
        nb_composantes,
        nb_pb_solved,
        nb_total_cands
    );

    return 0; 
}


