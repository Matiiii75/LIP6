#include "gen_graph.hpp"

int main() {

    Dag dag(2000); 

    std::cout << "Génération du dag ... " << std::flush;
    dag.gen_graph(0.95); 
    std::cout << "OK" << std::endl;

    std::cout << "Calcul de la fermeture transitive ... " << std::flush;
    dag.compute_transitive_closure(); 
    std::cout << "OK" << std::endl;

    std::cout << "Calcul du graphe de co-comparabilité ... " << std::flush; 
    dag.compute_HG(); 
    std::cout << "OK" << std::endl;

    std::cout << "Calcul de la degenerascy ... " << std::flush; 
    dag.compute_degenerascy();
    std::cout << "OK" << std::endl;
    
    // dag.display_dag();
    dag.display_HG();  
    dag.display_infos(); 
    dag.write_in_file(); 

    return 0; 
}