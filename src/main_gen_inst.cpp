#include "gen_graph.hpp"

void gen_single_inst(int n, float p, int ID) {
    
    Dag dag(n); 
    dag.gen_graph(p); 
    dag.compute_transitive_closure(); 
    dag.compute_HG(); 
    dag.compute_degenerascy(); 

    dag.write_in_file(ID); 

}   

void gen_graphs_for_stats() {

    std::ofstream csv("../data.csv"); // on va écrire les résultats obtenus dedans 
    std::vector<int> wanted_n = {100}; 

    for(int n : wanted_n) {
        for(float p = 0.05; p < 1.0; p += 0.05) {
            for(int test = 0; test < 10; ++test) {

                std::cout << "n : " << n << " / "; 
                std::cout << "p : " << p << " / "; 
                std::cout << "test : " << test << std::endl;

                Dag dag(n); 
                dag.gen_graph(p); 
                dag.compute_transitive_closure(); 
                dag.compute_HG(); 
                dag.compute_degenerascy(); 

                dag.write_in_file(test); // on lui donne l'ID de son test

                csv << n << " "; 
                csv << p << " "; 
                csv << dag.degenerascy << std::endl;

            }
        }
    }

    csv.close(); 

}


int main() {
    for(int i = 0; i < 10; ++ i) 
        gen_single_inst(500, 0.005, i); 
    return 0; 
}


int main2() {

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
    dag.write_in_file(0); 

    return 0; 
}
