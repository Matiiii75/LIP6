#include "common.hpp"


bool is_disjointV2(const std::vector<int>& succ_gamma, int t, 
    const std::vector<int>& C, const std::vector<std::vector<bool>>& TC) 
{ 
    for(int u : succ_gamma) { // pr chaque succ de gamma 
        if(u == t) continue; // si c'est le puit -> ignorer 
        for(int v : C) { // pr chaque candidat 
            if(TC[v][u]) { // si y'a un chemin de v à u 
                return true; // alors u pas dans S(C)
            }
        } 
    }

    return false; // si on arrive ici, tous les succ gamma sont dans S(C)
}

bool is_disjoint(const std::vector<int>& succ_gamma, int t, 
    const std::unordered_set<int> cut_set) 
{
    for(int u : succ_gamma) {
        if(u == t) continue; // puit -> ignorer 
        if(cut_set.count(u) == 0)
            return true; 
    }
    
    return false; 
}

bool is_includedV2(const std::vector<int>& v1, const std::vector<int>& v2, int t) {

    if(v1.size() > v2.size())
        return false; 

    int j = 0; 
    int v2_size = (int)v2.size(); 
    int v1_size = (int)v1.size(); 

    if(v1.back() == t) // on ignore le puit 
        --v1_size; 
    
    for(int i = 0; i < v1_size; ++i) { // pr chq index de v1

        while(j < v2_size && v2[j] < v1[i]) 
            ++j; // while j dépasse pas taille(v2) ET on est plus petit que v1 : incrémenter 

        if(j == v2_size || v1[i] != v2[j]) 
            return false; // si on a dépassé la taille de v2 OU que v1(i) dépasse v2(j)
        // -> conclusion : v1(i) n'existe pas dans v2
        
        ++j; // si le if d'avant s'est pas déclenché, on est dans le cas v1(i) = v2(j)
        // -> on continue 
            
    }

    return true; 
}


bool is_included(const std::vector<int>& v1, const std::unordered_set<int>& v2, int t) {

    int v1_size = (int)v1.size(); 

    if(v1_size > 0 && v1.back() == t) // on ignore le puit 
        --v1_size; 

    for(int i = 0; i < v1_size; ++i) { // pr chq index de v1
        if(v2.find(v1[i]) == v2.end()) return false; // si on le trouve pas dans v2  
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




