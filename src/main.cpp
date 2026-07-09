#include "Master.hpp"
#include "Heuristics.hpp"

void run_SAA(Data& data) {

    Heuristics h(data); // partie heuristique SAA 
    h.SAA_optimize(1000, 100); 
    std::cout << "Valeur trouvée -> " << h.obj_val << std::endl;

}

void run_param_comp_algo(Data& data) {

    // la source c'est 0, le puit c'est le dernier sommet du dag (par défaut)
    Master prog(data, 0, data.dag_size-1); 
    prog.build_SG(); 
    
    // prog.SG.display_SG_detail(); // à décommenter pour afficher le graphe d'états et ses données 
    // prog.SG.display_weights(); 

    std::vector<int> ordre_topo; 
    ordre_topo = prog.rebuild_opt_order(); 

    std::cout << "affichage ordre topo : " << std::endl; 
    for(int i : ordre_topo)
        std::cout << i << " "; 
    std::cout << std::endl;
    
    int val_found = prog.best_dist.back(); 

    std::cout << "De valeur : " << val_found << std::endl;

    std::cout << "Entrée checker_DSC" << std::endl;
    bool verif = prog.checker_DSC(ordre_topo, val_found); 
    if(verif) std::cout << "checker OK !" << std::endl;

    std::cout << "nombre de hash généré : " << prog.SG.hash_to_ID.size() << std::endl;
    std::cout << "nombre de candidats : " << prog.SG.ID_to_cands.size() << std::endl;

}

// arg 0 -> ./prog 
// arg 1 -> nom de l'instance 
// arg 2 -> 0 si on lance juste l'algo de complexité paramétrée, 
//          1 si on lance juste le SAA 
//          2 si on lance SAA + algo complexité paramétrée 
int main(int argc, char* argv[]) {

    if(argc != 3) 
        throw std::runtime_error("Nombre d'arguments fournis en arguments incorrect"); 

    int mode_execution = atoi(argv[2]); 

    std::string file = argv[1]; 
    Data data(file); 

    switch (mode_execution) 
    {
        case 0: // lancement algo complexité param 
            run_param_comp_algo(data); 
            break; 
        case 1: // lancement SAA seul 
            run_SAA(data);
            break;  
        case 2: // lancement SAA + algo complexité param
            run_SAA(data); 
            run_param_comp_algo(data); 
            break; 
        default: 
            throw std::runtime_error("main: pb dans le switch -> param choisis incorrect"); 
    }

    return 0; 
}


