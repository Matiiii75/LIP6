#include "Data.hpp"


/* DÉBUT PARTIE KEYHASH CANDIDATS */

bool keyHash::operator==(const keyHash& other) const {
    return l == other.l && r == other.r;
}

keyHash keyHash::operator^(const keyHash& other) const {
    keyHash nouveau;
    nouveau.l = l ^ other.l; 
    nouveau.r = r ^ other.r; 
    return nouveau;
}

std::ostream& keyHash::operator<<(std::ostream& os) const {
    os << l << " " << r; 
    return os; 
}


keyHash randomGenerator(std::mt19937_64& moteur) {

    // pr générer entre 0 et le max encodé sur 64 bits aléatoirement 
    std::uniform_int_distribution<std::uint64_t> dist(0, 
        std::numeric_limits<std::uint64_t>::max()); 

    keyHash nouveau; 
    nouveau.l = dist(moteur); 
    nouveau.r = dist(moteur); 

    return nouveau; 
}


keyHash compute_cand_hash(const std::vector<int>& cand, const std::vector<keyHash>& node_to_hash) {

    keyHash base; 
    base.l = 0; // on créer un hash vallant 0 au début
    base.r = 0; 

    for(const int c : cand) { // on xor tous les candidats entre eux 
        base = base ^ node_to_hash[c]; 
    }

    return base; // a la fin, base vaut le xor de tous les candidats 
}

/* FIN PARTIE KEYHASH CANDIDATS */

/* DÉBUT PARTIE DATA */

Data::Data(const std::string& file) {

    std::ifstream f(file); 
    
    if(!f.is_open()) 
        throw std::runtime_error("Couldn't open file " + file); 
    
    f >> dag_size >> nb_arcs >> degenerascy; 
    
    dag.resize(dag_size);  
    reverse_dag.resize(dag_size); 

    for(int i = 0; i < nb_arcs; ++i) {

        int from, to; 
        f >> from >> to; 
        dag[from].push_back(to);
        reverse_dag[to].push_back(from); 

    }

    compute_node_to_hash(); // calcul des hash 
    compute_transitive_closure(); 

}


void Data::compute_transitive_closure() { 

    std::vector<std::vector<bool>> trans_closure(dag_size, 
        std::vector<bool>(dag_size,0)); // init trans closure tt a 0
    
    // on copie les relations du graphe 
    for(int i = 0; i < dag_size; ++i) {
        for(int j : dag[i]) {
            trans_closure[i][j] = true; 
        }
    }
    
    for(int i = 0; i < dag_size; ++i) 
        trans_closure[i][i] = true; // la diagonale vaut 1

    
    for(int k = 0; k < dag_size; ++k) {
        for(int i = 0; i < dag_size; ++i) {
            for(int j = 0; j < dag_size; ++j) {

                // si i->k & k->j alors i->j
                if(trans_closure[i][k] && trans_closure[k][j])
                    trans_closure[i][j] = true; 

            }
        }
    }

    this->TC = trans_closure; 
}


void Data::compute_node_to_hash() {

    std::vector<keyHash> nodeToHash(dag_size); 

    std::random_device rd; // graine aléatoire
    std::mt19937_64 moteur(rd()); // generateur aléatoire : produit rapidement un nombre encodé sur 64 bits 

    for(int i = 0; i < dag_size; ++i) { // pr chq sommet 
        nodeToHash[i] = randomGenerator(moteur); // générer aléatoirement son hash 
    }

    node_to_hash = nodeToHash; // mémorise en attribut de classe 

}


void Data::display_dag() const {

    for(int i = 0; i < (int)dag.size(); ++i) {
        std::cout << i << " -> "; 
        for(int j = 0; j < (int)dag[i].size(); ++j) {
            std::cout << dag[i][j] << " "; 
        }
        std::cout << std::endl;
    }

}

void Data::display_reverse_dag() const {

    for(int i = 0; i < (int)reverse_dag.size(); ++i) {
        std::cout << i << " -> "; 
        for(int j = 0; j < (int)reverse_dag[i].size(); ++j) {
            std::cout << reverse_dag[i][j] << " "; 
        }
        std::cout << std::endl;
    }

}






