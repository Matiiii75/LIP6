#include "Master.hpp"
#include "Heuristics.hpp"

// éxécute SAA sur une instance donnée 
// calcule et affiche la valeur obtenue 
// n'écrit rien dans un fichier 
void run_SAA(Data& data) {

    Heuristics h(data); // partie heuristique SAA 
    h.SAA_optimize(1000, 100); 
    std::cout << "Valeur trouvée -> " << h.obj_val << std::endl;

}

// éxécute l'algo de complexité paramétrée pour une instance donnée 
// affiche l'ordre topo optimal, la valeur associée, check la validité 
// et affiche le nombre de sommets de SGG généré, ainsi que le nombre de hash différents 
// n'écrit rien dans un fichier 
void run_param_comp_algo(Data& data) {

    // la source c'est 0, le puit c'est le dernier sommet du dag (par défaut)
    Master prog(data, 0, data.dag_size-1); 
    prog.build_SG(); 

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

#include <filesystem> // pr gérer la recherche dans un dossier du pc (C++17)

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


