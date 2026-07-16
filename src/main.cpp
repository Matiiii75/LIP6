#include "Master.hpp"
#include "Heuristics.hpp"
#include "Timer.hpp"

// éxécute SAA sur une instance donnée 
// calcule et affiche la valeur obtenue 
// n'écrit rien dans un fichier 
void run_SAA(Data& data) {

    Heuristics h(data); // partie heuristique SAA 
    h.SAA_optimize(5000, 500); 
    std::cout << "Valeur trouvée -> " << h.obj_val << std::endl;

}

// éxécute l'algo de complexité paramétrée pour une instance donnée 
// affiche l'ordre topo optimal, la valeur associée, check la validité 
// et affiche le nombre de sommets de SGG généré, ainsi que le nombre de hash différents 
// n'écrit rien dans un fichier 
void run_param_comp_algo(Data& data) {

    // la source c'est 0, le puit c'est le dernier sommet du dag (par défaut)
    int source = 0; 
    int puit = data.dag_size - 1; 
    double time_limit = 1200.00; 
    Master prog(data, source, puit, time_limit);

    prog.build_SG(); // lancement de la construction de l'algorithme 

    if(prog.found_solution) {

        prog.extract_results(); 
    
        bool display_opt_order = true; 
        bool display_opt_val = true; 
        bool display_hash_infos = true; 

        prog.display_results( // affichage du résultat 
            display_opt_order,
            display_opt_val,
            display_hash_infos
        ); 

    } else {

        std::cout << "Arrêt algorithme : time_limit excedée" << std::endl;

    }

}


// arg 0 -> ./prog 
// arg 1 -> nom de l'instance 
// arg 2 -> 0 si on lance juste l'algo de complexité paramétrée, 
//          1 si on lance juste le SAA 
//          2 si on lance SAA + algo complexité paramétrée 
int main_bis(int argc, char* argv[]) {

    if(argc != 3) 
        throw std::runtime_error("Nombre d'arguments fournis en arguments incorrect"); 

    int mode_execution = atoi(argv[2]); 

    Timer tm; 
    tm.start_timer(); 

    std::string file = argv[1]; 
    Data data(file); 

    double data_import_time = tm.get_temps_passe(); // récupère le temps requis pour gérer les data 

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

    double temps_total = tm.get_temps_passe();
    double temps_generation_SG = temps_total - data_import_time; 
    std::cout << "temps total -> " << temps_total << std::endl;
    std::cout << "temps gestion des data -> " << data_import_time << std::endl;
    std::cout << "temps passé dans la génération du graphe d'états -> " << temps_generation_SG; 
    std::cout << std::endl;

    return 0; 
}

#include <filesystem> // pr gérer la recherche dans un dossier du pc (C++17)
#include <sstream>

// on lui donne un nom d'instance
// il extrait la degen du nom et la renvoie
int extrait_degen(const std::string& inst) {

    std::stringstream ss(inst); 
    std::string morceau; 

    std::getline(ss,morceau,'_'); // extrait la valeur de n
    std::getline(ss,morceau,'_'); // extrait 'n'
    std::getline(ss,morceau,'_'); // extrait la valeur de k
    
    int k = std::stoi(morceau); 
    
    return k; 
}

// trie les noms d'instances par ordre croissant de la degeneracy 
void trie_instances_degen_croissante(std::vector<std::string>& instances) {

    std::sort(instances.begin(), instances.end(),
        [](const std::string& a, const std::string& b)
    {
        return extrait_degen(a) < extrait_degen(b); 
    });

}


int main() {

    int degen_max = 50; 

    std::filesystem::path chemin = "../instances/k_variations/"; 
    std::vector<std::string> content_folder; 

    for(const auto& entree : std::filesystem::directory_iterator(chemin))  {
        if(entree.path().filename().string() == ".DS_Store") continue; 
        content_folder.push_back(entree.path().filename().string());
    }
         

    trie_instances_degen_croissante(content_folder); 

    // Execution de l'algorithme pour chaque instance 

    for(auto inst : content_folder) {

        if(extrait_degen(inst) > degen_max) continue; // ignore les instances de trop grande degen

        std::cout << inst << " -> "; 

        // débuter le minuteur 
        Timer tm; 
        tm.start_timer(); 

        // lire la data 
        std::string path_to_isnt = "../instances/k_variations/" + inst; 
        Data data(path_to_isnt); 

        // lancer l'execution pour l'instance 
        int source = 0; 
        int puit = data.dag_size - 1; 
        double time_limit = 1200.00; // time limit de 20 minutes 
        Master prog(data, source, puit, time_limit); 

        prog.build_SG(); 
        
        int val_opt = -1; 
        int nb_nodes_SG = -1;
        double total_time = tm.get_temps_passe(); 

        if(prog.found_solution) // si on a eu le time de trouver une solution 
        { 
            prog.extract_results(); // on extrait les résultats
            val_opt = prog.optimal_value; 
            nb_nodes_SG = prog.nb_candidats;  
        } 

        write_main_infos(inst, val_opt, total_time, nb_nodes_SG);
        
        std::cout << "val : " << val_opt << " | time : " << total_time << " | number nodes in SG : " << nb_nodes_SG << std::endl;

    }

}


// petit main pour gérer l'écriture des résultats de SAA dans un fichier texte 
int main2() {

    // commencer par stocker tous les fichiers dans le dossier d'instances
    std::filesystem::path chemin = "../inst/"; 
    std::vector<std::string> content_folder; 
   
    for(const auto& entree : std::filesystem::directory_iterator(chemin)) 
        content_folder.push_back(entree.path().filename().string()); 
    
    // puis, ouvrir fichier d'écriture 
    std::ofstream writing("../SAA_infs.txt"); 

    for(const std::string& inst : content_folder)
    {   
        if(inst == ".DS_Store") continue; 
        std::cout << "traitement de " << inst << std::endl; 
        std::string path = "../inst/" + inst; 
        Data data(path); // lire l'instance 
        Heuristics h(data); // lancer SAA 
        h.SAA_optimize(1000, 100); // avec temp = 1000 et palier = 100
        writing << inst << " " << h.obj_val << std::endl; // écrire la donnée 
    }

    writing.close(); 
    return 0; 
}

