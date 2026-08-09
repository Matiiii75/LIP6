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
    double time_limit = 1200.0; 

    Master prog(data, source, puit, time_limit, uc); 
    prog.solve_DSC_with_pre_treatment(); 

    return 0; 
}


