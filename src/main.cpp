#include "Master.hpp"
#include "Heuristics.hpp"

int main(int argc, char* argv[]) {

    std::string file = argv[1]; 
    Data data(file); 
    
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

    return 0; 
}


