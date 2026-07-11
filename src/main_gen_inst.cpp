#include "gen_graph.hpp"

// permet de générer une instance avec un n,p et ID donnés
void gen_single_inst(int n, float p, int ID) {
    
    Dag dag(n); 
    dag.gen_graph(p); 
    dag.compute_transitive_closure(); 
    dag.compute_HG(); 
    dag.compute_degenerascy(); 

    std::cout << "generating -> " << n << "_n_"; 
    std::cout << dag.degenerascy << "_k_"; 
    std::cout << ID << "_ID.txt" << std::endl;

    dag.write_in_file(ID); 

}   

// permet de générer un ensemble d'instances de n,p & ID différents 
// j'utilisais cette fonction afin de générer une multitude de graphes et 
// d'identifier quels paramètres donnaient quelle degeneracy 
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

