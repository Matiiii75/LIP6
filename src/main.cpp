#include "Master.hpp"
#include "Heuristics.hpp"

// int main(int argc, char* argv[]) {

//     std::string file = argv[1]; 
//     Data data(file); 
    
//     // la source c'est 0, le puit c'est le dernier sommet du dag (par défaut)
//     Master prog(data, 0, data.dag_size-1); 
//     prog.build_SG(); 
    
//     prog.SG.display_SG_detail(); 
//     prog.SG.display_weights(); 

//     std::vector<int> ordre_topo; 
//     ordre_topo = prog.rebuild_opt_order(); 

//     std::cout << "affichage ordre topo : " << std::endl;
//     for(int i : ordre_topo)
//         std::cout << i << " "; 
//     std::cout << std::endl;
    
//     int val_found = prog.best_dist.back(); 

//     std::cout << "De valeur : " << val_found << std::endl;

//     std::cout << "Entrée checker_DSC" << std::endl;
//     bool verif = prog.checker_DSC(ordre_topo, val_found); 
//     if(verif) std::cout << "checker OK !" << std::endl;

//     return 0; 
// }


int main(int argc, char* argv[]) {

    std::string file = argv[1]; 
    Data data(file); 

    data.display_dag(); 
    
    Heuristics test(data); 

    std::cout << "ordre topologique initial : " << std::endl;
    for(int i : test.ordre_to_node)
        std::cout << i << " "; 
    std::cout << std::endl;
    
    std::cout << "node_order_matrix : " << std::endl;
    for(auto& vec : test.node_order_matrix) {
        for(auto i : vec) {
            std::cout << i << " "; 
        }
        std::cout << std::endl;
    }

    test.SAA_optimize(100.0, 500); 

    std::cout << "ordre trouvé : " << std::endl;
    for(int i : test.ordre_to_node) {
        std::cout << i << " "; 
    }
    std::cout << std::endl;

    std::cout << "de valeur : " << test.obj_val << std::endl;

    std::cout << "valeur trouvée avec re-calcul : ";
    std::cout << test.compute_DSC_obj() << std::endl;

    Master m(data, 0, data.dag_size-1); 

    std::vector<int> enleve_fictifs; 
    for(int i : test.ordre_to_node)
        if(i != 0 && i != data.dag_size-1)  enleve_fictifs.push_back(i); 

    if(m.checker_DSC(enleve_fictifs, test.obj_val))
        std::cout << "checker OK" << std::endl;



    return 0; 
}


