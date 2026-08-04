#include "common.hpp"

bool is_disjoint(const std::vector<int>& succ_gamma, int t, 
    const std::vector<uint8_t>& cut_set) 
{
    for(int u : succ_gamma) {
        if(u == t) continue; // puit -> ignorer 
        if(cut_set[u] == 0) // si il est pas dans le cut_set
            return true; 
    }
    
    return false; 
}

bool is_included(const std::vector<int>& v1, const std::vector<uint8_t>& v2, int t) {

    int v1_size = (int)v1.size(); 

    if(v1_size > 0 && v1.back() == t) // on ignore le puit 
        --v1_size; 

    for(int i = 0; i < v1_size; ++i) { // pr chq index de v1
        int v_i = v1[i]; 
        if(v2[v_i] == 0) return false; // si il est pas dans v2
    }

    return true; // si on arrive là c'est que tout élément de v1 est dans v2
}


int find_gamma(const std::vector<int>& v1, const std::vector<int>& v2) {

    int v2_size = (int)v2.size(); 
    int j = 0; 

    for(int i = 0; i < (int)v1.size(); ++i) {

        while(j < v2_size && v2[j] < v1[i]) 
            ++j; // on continue jusqu'a depasser v2SIZE ou bien atteindre v1[i] dans v2 

        if(j < v2_size && v2[j] > v1[i]) 
            return v1[i]; // on a pas trouvé v1[i] dans v2 -> le return 

        else if (j < v2_size && v2[j] == v1[i]) 
            ++j; // on a atteint v1[i] -> ++j et next itération 

        else break; // 

    }

    if(v2_size < (int)v1.size()) // cas v2 > v1 
        return v1[v2_size]; 

    return -1; // on a pas trouvé de gamma (devrait pas arriver !!) 
}


void increase_sort_vector(std::vector<int>& vec) {
    sort(vec.begin(), vec.end()); 
}


void debug(const std::string& s) 
{
    std::cout << s << std::endl;
}


void display_vec(const std::vector<int>& v) 
{
    for(const int i : v) 
        std::cout << i << " "; 
    std::cout << std::endl;
}


void display_FIFO(std::queue<int> Q) 
{   
    std::cout << "affichage fifo : " << std::endl;
    while(!Q.empty()) {
        std::cout << Q.front(); 
        Q.pop(); 
    }
    std::cout << std::endl;
}


int random_int(int a, int b, std::mt19937& gen) 
{
    std::uniform_int_distribution<> distrib(a,b); 
    return distrib(gen); 
}


std::string getFileName(const std::string& path) {

    size_t lastSlash = path.find_last_of("/\\"); // compatible mac/linux
    return (lastSlash == std::string::npos) ? path : path.substr(lastSlash + 1);

}


void write_main_infos(
    const std::string& path_to_write, 
    const std::string& inst, int dag_size, 
    int degeneracy, int val_opt, double time, 
    int nb_nodes_SG) 
{
    std::ofstream writing(path_to_write, std::ios::app); 
    writing << inst << " " << dag_size << " " << degeneracy << " "; 
    writing << val_opt << " " << nb_nodes_SG << " " << time << std::endl;
    writing.close(); 
}




