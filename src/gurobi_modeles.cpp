#include "gurobi_modeles.hpp"


void Gurobi_modeles::modele_positions_DSC() {

    try {
        // GRBEnv et GRBModel sont déjà des objets RAII (l'API Gurobi gère
        // elle-même la libération des ressources sous-jacentes dans leur
        // destructeur) : pas besoin de new/delete, des objets locaux suffisent.
        GRBEnv env(true);
        env.set(GRB_IntParam_OutputFlag, 0); // 0 pr désactier les affichages dans le terminal
        env.set(GRB_IntParam_Threads, 1); // FORCER GUROBI A N'UTILISER QU'UN SEUL THREAD PAR INSTANCE
        env.start();

        GRBModel model(env);

        if (time_limit > 0.0) {
            model.set(GRB_DoubleParam_TimeLimit, time_limit);
        }

        int nb_nodes = data.dag_size;
        int source = 0;
        int puit = nb_nodes - 1;

        std::vector<int> all_pos;
        for (int i = 0; i < nb_nodes - 2; ++i) {
            all_pos.push_back(i);
        }

        std::vector<int> all_nodes;
        for (int i = 0; i < nb_nodes; ++i) {
            if (i != source && i != puit) {
                all_nodes.push_back(i);
            }
        }

        std::vector<std::pair<int, int>> all_arcs;
        for (int i = 0; i < nb_nodes; ++i) {
            for (int j : data.dag[i]) {
                if (i != source && j != puit) {
                    all_arcs.push_back({i, j});
                }
            }
        }

        int last_pos = all_pos.empty() ? -1 : all_pos.back();

        // Matrice de variables (indexée par all_pos et all_nodes)
        std::vector<std::vector<GRBVar>> x(nb_nodes, std::vector<GRBVar>(nb_nodes));
        std::vector<std::vector<GRBVar>> y(nb_nodes, std::vector<GRBVar>(nb_nodes));

        for (int i : all_pos) {
            for (int j : all_nodes) {
                std::string name_x = "x_" + std::to_string(i) + "_" + std::to_string(j);
                x[i][j] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, name_x);

                std::string name_y = "y_" + std::to_string(i) + "_" + std::to_string(j);
                y[i][j] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, name_y);
            }
        }

        // Objectif
        GRBLinExpr obj = 0;
        for (int i : all_pos) {
            for (int j : all_nodes) {
                obj += y[i][j];
            }
        }
        model.setObjective(obj, GRB_MINIMIZE);

        // Contrainte 1 : Toute position a un job unique
        for (int i : all_pos) {
            GRBLinExpr c1 = 0;
            for (int j : all_nodes) c1 += x[i][j];
            model.addConstr(c1 == 1, "pos_" + std::to_string(i));
        }

        // Contrainte 2 : Tout job a une position unique
        for (int j : all_nodes) {
            GRBLinExpr c2 = 0;
            for (int i : all_pos) c2 += x[i][j];
            model.addConstr(c2 == 1, "node_" + std::to_string(j));
        }

        // Contrainte 3 : Respect de l'ordre imposé par le DAG
        for (int i0 : all_pos) {
            for (auto& arc : all_arcs) {
                int j1 = arc.first;
                int j2 = arc.second;
                GRBLinExpr c3 = 0;
                for (int i = 0; i <= i0; ++i) {
                    c3 += (x[i][j1] - x[i][j2]);
                }
                model.addConstr(c3 >= 0);
            }
        }

        // Contrainte 4 : Définition des variables y
        for (int i0 : all_pos) {
            if (i0 == last_pos) continue;
            for (auto& arc : all_arcs) {
                int j1 = arc.first;
                int j2 = arc.second;
                GRBLinExpr rhs = 0;
                
                for (int i = 0; i <= i0; ++i) {
                    rhs += x[i][j1];
                }
                for (int i = i0 + 1; i <= last_pos; ++i) {
                    rhs += x[i][j2];
                }
                model.addConstr(y[i0][j1] + 1 >= rhs);
            }
        }

        // Résolution
        model.set(GRB_DoubleParam_MIPGap, 0.0); // OPTIMALITÉ STRICTE, on ne s'arrete pas tant que UB != LB
        model.optimize();

        int status = model.get(GRB_IntAttr_Status);

        if (status == GRB_OPTIMAL || status == GRB_TIME_LIMIT) {

            this->solve_time = model.get(GRB_DoubleAttr_Runtime); 

            if (model.get(GRB_IntAttr_SolCount) > 0) {

                this->found_solution = true; 
                this->obj_val = model.get(GRB_DoubleAttr_ObjVal);
                this->best_bound = model.get(GRB_DoubleAttr_ObjBound);
                this->mip_gap = model.get(GRB_DoubleAttr_MIPGap);

            } else {
                
                try {
                    this->best_bound = model.get(GRB_DoubleAttr_ObjBound); 
                } catch (const GRBException& e) {
                    std::cout << "best bound : non dispo, arret trop précoce"; 
                }

            }
        } else if (status == GRB_INFEASIBLE) {
            std::cout << "[Gurobi] ERREUR : Le modele est infaisable.\n";
        }

        // Pas de nettoyage manuel à faire : model puis env (ordre inverse de
        // construction) sont détruits automatiquement en sortie de portée,
        // que la fonction se termine normalement ou via une exception.

    } catch (const GRBException& e) {
        std::cerr << "\n[CRASH GUROBI] Code " << e.getErrorCode() << " : " << e.getMessage() << "\n";
    } catch (const std::exception& e) {
        std::cerr << "\n[CRASH C++] " << e.what() << "\n";
    }
}


void Gurobi_modeles::modele_positions_relatives_DSC(bool relaxation) {

    try {
        
        GRBEnv env(true); 
        env.set(GRB_IntParam_OutputFlag, 1);
        env.set(GRB_IntParam_Threads, 1); // FORCER GUROBI A N'UTILISER QU'UN SEUL THREAD PAR INSTANCE
        env.start();

        GRBModel model(env);

        if (time_limit > 0.0) {
            model.set(GRB_DoubleParam_TimeLimit, time_limit);
        }

        int nb_nodes = data.dag_size;
        int source = 0;
        int puit = nb_nodes - 1;

        char vtype = relaxation ? GRB_CONTINUOUS : GRB_BINARY; // si true -> relaxation sinon non 

        std::vector<int> all_pos;
        for (int i = 0; i < nb_nodes - 2; ++i) {
            all_pos.push_back(i);
        }

        std::vector<int> all_nodes;
        for (int i = 0; i < nb_nodes; ++i) {
            if (i != source && i != puit) {
                all_nodes.push_back(i);
            }
        }

        std::vector<std::pair<int, int>> all_arcs;
        for (int i = 0; i < nb_nodes; ++i) {
            for (int j : data.dag[i]) {
                if (i != source && j != puit) {
                    all_arcs.push_back({i, j});
                }
            }
        }

        // DÉFINITION VARIABLES 

        // z : zij = 1 si i est placé avant j 

        std::vector<std::vector<GRBVar>> z(nb_nodes, std::vector<GRBVar>(nb_nodes)); 

        for(int i = 0; i < nb_nodes; ++i) {
            for(int j = 0; j < nb_nodes; ++j) {
                std::string name = "z_" + std::to_string(i) + "_" + std::to_string(j); 
                z[i][j] = model.addVar(0.0, 1.0, 0.0, vtype, name); 
            }
        }

        // Phi 

        std::vector<GRBVar> Phi(nb_nodes); 

        for(int i : all_nodes) {
            std::string name = "Phi_" + std::to_string(i); 
            Phi[i] = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, name); 
        }

        // Mu 

        std::vector<GRBVar> Mu(nb_nodes); 

        for(int i : all_nodes) {
            std::string name = "Mu_" + std::to_string(i); 
            Mu[i] = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, name); 
        }

        // DÉFINITION OBJECTIF 

        GRBLinExpr obj = 0; 
        
        for(int u : all_nodes) {
            obj += Mu[u] - Phi[u]; 
        }
        model.setObjective(obj, GRB_MINIMIZE); 

        // DÉFINITION CONTRAINTES 

        // si (u,v) \in A -> zuv = 1

        for(auto& arc : all_arcs) {
            int u = arc.first; 
            int v = arc.second; 
            model.addConstr(z[u][v] == 1); 
        }

        // antisymétrie 

        for(int i = 0; i < (int)all_nodes.size(); ++i) {
            int u = all_nodes[i]; 
            for(int j = i+1; j < (int)all_nodes.size(); ++j) {
                int v = all_nodes[j]; 
                model.addConstr(z[u][v] + z[v][u] == 1); 
            }
        }

        // définitions de Phi

        for(int u : all_nodes) {
            GRBLinExpr cstr_def_Phi = 0; 
            for(int v : all_nodes) {
                if(v == u) continue; 
                cstr_def_Phi += z[v][u]; 
            }
            model.addConstr(Phi[u] == cstr_def_Phi); 
        }

        // DÉFINITION DE Mu 

        for(int u : all_nodes) {
            model.addConstr(Mu[u] >= Phi[u]); 
            for(int v : data.dag[u]) {
                if(v == puit) continue; 
                model.addConstr(Mu[u] >= Phi[v]); 
            }
        }

         

        for(int u = 1; u < puit-2; ++u) {
            for(int v = u+1; v < puit-1; ++v) {
                for(int w = v+1; w < puit; ++w) {

                    // model.addConstr(z[u][v] + z[v][w] + z[w][u] <= 2); 
                    // model.addConstr(z[u][w] + z[w][v] + z[v][u] <= 2); 

                    model.addConstr(z[u][v] + z[v][w] - 1 <= z[u][w]); 
                    model.addConstr(z[u][w] + z[w][v] - 1 <= z[u][v]); 

                }
            }
        }

        // Résolution
        model.optimize();

        int status = model.get(GRB_IntAttr_Status);

        if (status == GRB_OPTIMAL || status == GRB_TIME_LIMIT) {

            this->solve_time = model.get(GRB_DoubleAttr_Runtime); 

            if (model.get(GRB_IntAttr_SolCount) > 0) { // si on a trouvé au moins une solution 
                
                this->found_solution = true; 
                this->obj_val = model.get(GRB_DoubleAttr_ObjVal);
                this->best_bound = model.get(GRB_DoubleAttr_ObjBound);
                this->mip_gap = model.get(GRB_DoubleAttr_MIPGap);

            } else {
                
                try {
                    this->best_bound = model.get(GRB_DoubleAttr_ObjBound); 
                } catch (const GRBException& e) {
                    std::cout << "best bound : non dispo, arret trop précoce"; 
                }
            }
        } else if (status == GRB_INFEASIBLE) {
            std::cout << "[Gurobi] ERREUR : Le modele est infaisable.\n";
        }

        // Pas de nettoyage manuel à faire : model puis env (ordre inverse de
        // construction) sont détruits automatiquement en sortie de portée,
        // que la fonction se termine normalement ou via une exception.

    } catch (const GRBException& e) {
        std::cerr << "\n[CRASH GUROBI] Code " << e.getErrorCode() << " : " << e.getMessage() << "\n";
    } catch (const std::exception& e) {
        std::cerr << "\n[CRASH C++] " << e.what() << "\n";
    }

}


void Gurobi_modeles::modele_membership_DSC() {

    try {
        
        GRBEnv env(true);
        env.set(GRB_IntParam_OutputFlag, 1);
        env.set(GRB_IntParam_Threads, 1); // FORCER GUROBI A N'UTILISER QU'UN SEUL THREAD PAR INSTANCE
        env.start();

        GRBModel model(env);

        if (time_limit > 0.0) {
            model.set(GRB_DoubleParam_TimeLimit, time_limit);
        }

        int nb_nodes = data.dag_size;
        int source = 0; 
        int puit = nb_nodes-1; 
        int nb_pos = nb_nodes+1;
        
        std::vector<int> all_pos; // pr stocker toutes les positions [1,n]
        for(int i = 1; i < nb_pos; ++i) all_pos.push_back(i);  

        std::vector<std::pair<int, int>> all_arcs;
        for (int i = 0; i < nb_nodes; ++i) {
            for (int j : data.dag[i]) {
                if (i != source && j != puit) {
                    all_arcs.push_back({i, j});
                }
            }
        }

        // variables 

        // xui = 1 si phi(u) <= i
        std::vector<std::vector<GRBVar>> x(nb_nodes, std::vector<GRBVar>(nb_pos)); 

        for(int u = 0; u < nb_nodes; ++u) {
            for(int i : all_pos) {
                std::string name = "x_" + std::to_string(u) + "_" + std::to_string(i); 
                x[u][i] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, name); 
            }
        }

        // variable PHI

        std::vector<GRBVar> Phi(nb_nodes); 
        for(int u = 0; u < nb_nodes; ++u) 
            Phi[u] = model.addVar(0.0, GRB_INFINITY, 1.0, GRB_CONTINUOUS, "Phi_"+std::to_string(u)); 

        // CONTRAINTES 

        // ces deux contraintes définissent Phi(s) = 0 et Phi(t) = 0
        model.addConstr(Phi[source] == 0); 
        model.addConstr(Phi[puit] ==0); 

        // ces deux contraintes éliminent de la symétrie en fixant direct la position de source et puit 
        model.addConstr(x[source][1] == 1);               // phi(source) = 1
        model.addConstr(x[puit][nb_pos - 2] == 0);         // phi(puit) = nb_nodes (== n)

        // définition de Phi(u)

        for(int u = 1; u < nb_nodes-1; ++u) {
            for(int v : data.dag[u]) {
                if(v == puit) continue; 
                GRBLinExpr cstr_phi_u = 0; 
                for(int i : all_pos) 
                    cstr_phi_u += x[u][i] - x[v][i];
                model.addConstr(cstr_phi_u <= Phi[u]); 
            }
        }

        // si xui = 1, alors xu(i+1) = 1 
        
        for(int u = 0; u < nb_nodes; ++u) {
            for(int i = 1; i < nb_pos-1; ++i) {
                model.addConstr(x[u][i] <= x[u][i+1]); 
            }
        }

        // le nombre de sommets placés avant ou sur i vaut i

        for(int i = 1; i < nb_pos; ++i) {
            GRBLinExpr cstr_nb_sommets = 0; 
            for(int u = 0; u < nb_nodes; ++u)
                cstr_nb_sommets += x[u][i]; 
            model.addConstr(cstr_nb_sommets == i); 
        }

        // contraintes d'ordre topologique (u,v) \in A implique phi(u) < phi(v)

        for(auto& arc : all_arcs) {
            int u = arc.first; 
            int v = arc.second; 
            for(int i = 2; i < nb_pos; ++i) 
                model.addConstr(x[u][i-1] >= x[v][i]); 
        }

        // Résolution
        model.optimize();

        int status = model.get(GRB_IntAttr_Status);

        if (status == GRB_OPTIMAL || status == GRB_TIME_LIMIT) {

            this->solve_time = model.get(GRB_DoubleAttr_Runtime); 

            if (model.get(GRB_IntAttr_SolCount) > 0) { // si on a trouvé au moins une solution 
                
                this->found_solution = true; 
                this->obj_val = model.get(GRB_DoubleAttr_ObjVal);
                this->best_bound = model.get(GRB_DoubleAttr_ObjBound);
                this->mip_gap = model.get(GRB_DoubleAttr_MIPGap);

            } else {
                
                try {
                    this->best_bound = model.get(GRB_DoubleAttr_ObjBound); 
                } catch (const GRBException& e) {
                    std::cout << "best bound : non dispo, arret trop précoce"; 
                }
            }
        } else if (status == GRB_INFEASIBLE) {
            std::cout << "[Gurobi] ERREUR : Le modele est infaisable.\n";
        }

        // Pas de nettoyage manuel à faire : model puis env (ordre inverse de
        // construction) sont détruits automatiquement en sortie de portée,
        // que la fonction se termine normalement ou via une exception.

    } catch (const GRBException& e) {
        std::cerr << "\n[CRASH GUROBI] Code " << e.getErrorCode() << " : " << e.getMessage() << "\n";
    } catch (const std::exception& e) {
        std::cerr << "\n[CRASH C++] " << e.what() << "\n";
    }

}


void Gurobi_modeles::modele_membership_positions_CW() {

    try {
        
        GRBEnv env(true);
        env.set(GRB_IntParam_OutputFlag, 0);
        env.set(GRB_IntParam_Threads, 1); // FORCER GUROBI A N'UTILISER QU'UN SEUL THREAD PAR INSTANCE
        env.start();

        GRBModel model(env);

        if (time_limit > 0.0) {
            model.set(GRB_DoubleParam_TimeLimit, time_limit);
        }

        int nb_nodes = data.dag_size;
        int source = 0; 
        int puit = nb_nodes-1; 
        int nb_pos = nb_nodes+1;
        
        std::vector<int> all_pos; // pr stocker toutes les positions [1,n]
        for(int i = 1; i < nb_pos; ++i) all_pos.push_back(i);  

        std::vector<std::pair<int, int>> all_arcs;
        for (int i = 0; i < nb_nodes; ++i) {
            for (int j : data.dag[i]) {
                if (i != source && j != puit) {
                    all_arcs.push_back({i, j});
                }
            }
        }

        // variables 

        // xui = 1 si phi(u) <= i
        std::vector<std::vector<GRBVar>> x(nb_nodes, std::vector<GRBVar>(nb_pos)); 

        for(int u = 0; u < nb_nodes; ++u) {
            for(int i : all_pos) {
                std::string name = "x_" + std::to_string(u) + "_" + std::to_string(i); 
                x[u][i] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, name); 
            }
        }

        // zuvi = 1 si l'arc (u,v) traverse la position i 
        std::map<std::pair<int,int>, std::vector<GRBVar>> z; 
        for(const auto& arc : all_arcs) {
            int u = arc.first; 
            int v = arc.second; 
            z[{u,v}].resize(nb_pos); // allouer la taille juste pr cet arc
            for(int i : all_pos) {
                std::string name = "z_" + std::to_string(u) + "_" + std::to_string(v) + std::to_string(i); 
                // on peut déclarer z en CONTINUE 
                z[{u,v}][i] = model.addVar(0.0, 1.0 , 0.0, GRB_CONTINUOUS, name); 
            }
        }

        // variable C 
        GRBVar c = model.addVar(0.0, GRB_INFINITY, 1.0, GRB_CONTINUOUS, "c"); 

        // CONTRAINTES 

        // ces deux contraintes éliminent de la symétrie en fixant direct la position de source et puit 
        model.addConstr(x[source][1] == 1);               // phi(source) = 1
        model.addConstr(x[puit][nb_pos - 2] == 0);         // phi(puit) = nb_nodes (== n)

        // définition de c

        for(int i : all_pos) {
            GRBLinExpr cstr_def_c = 0; 
            for(auto& arc : all_arcs) 
                cstr_def_c += z[arc][i];
            model.addConstr(c >= cstr_def_c);  
        }

        // si xui = 1, alors xu(i+1) = 1 
        
        for(int u = 0; u < nb_nodes; ++u) {
            for(int i = 1; i < nb_pos-1; ++i) {
                model.addConstr(x[u][i] <= x[u][i+1]); 
            }
        }

        // le nombre de sommets placés avant ou sur i vaut i

        for(int i = 1; i < nb_pos; ++i) {
            GRBLinExpr cstr_nb_sommets = 0; 
            for(int u = 0; u < nb_nodes; ++u)
                cstr_nb_sommets += x[u][i]; 
            model.addConstr(cstr_nb_sommets == i); 
        }

        // définition de zuvi 
        for(auto& arc : all_arcs) {
            int u = arc.first; 
            int v = arc.second; 
            for(int i = 1; i < nb_pos; ++i) 
                model.addConstr(z[arc][i] >= x[u][i] - x[v][i]); 
        }

        // contraintes d'ordre topologique (u,v) \in A implique phi(u) < phi(v)

        for(auto& arc : all_arcs) {
            int u = arc.first; 
            int v = arc.second; 
            for(int i = 2; i < nb_pos; ++i) 
                model.addConstr(x[u][i-1] >= x[v][i]); 
        }

        // Résolution
        model.optimize();

        int status = model.get(GRB_IntAttr_Status);

        if (status == GRB_OPTIMAL || status == GRB_TIME_LIMIT) {

            this->solve_time = model.get(GRB_DoubleAttr_Runtime); 

            if (model.get(GRB_IntAttr_SolCount) > 0) { // si on a trouvé au moins une solution 
                
                this->found_solution = true; 
                this->obj_val = model.get(GRB_DoubleAttr_ObjVal);
                this->best_bound = model.get(GRB_DoubleAttr_ObjBound);
                this->mip_gap = model.get(GRB_DoubleAttr_MIPGap);

            } else {
                
                try {
                    this->best_bound = model.get(GRB_DoubleAttr_ObjBound); 
                } catch (const GRBException& e) {
                    std::cout << "best bound : non dispo, arret trop précoce"; 
                }
            }
        } else if (status == GRB_INFEASIBLE) {
            std::cout << "[Gurobi] ERREUR : Le modele est infaisable.\n";
        }

        // Pas de nettoyage manuel à faire : model puis env (ordre inverse de
        // construction) sont détruits automatiquement en sortie de portée,
        // que la fonction se termine normalement ou via une exception.

    } catch (const GRBException& e) {
        std::cerr << "\n[CRASH GUROBI] Code " << e.getErrorCode() << " : " << e.getMessage() << "\n";
    } catch (const std::exception& e) {
        std::cerr << "\n[CRASH C++] " << e.what() << "\n";
    }

}


void Gurobi_modeles::display_infos() const {

    std::cout << "----- [RESULTATS GUROBI] -----" << std::endl;    

    if(found_solution) { // si on a trouvé une solution
        std::cout << "--- [Solution trouvée] ---" << std::endl;
        std::cout << "[valeur] : " << this->obj_val << std::endl;
        std::cout << "[best bound] : " << this->best_bound << std::endl;
        std::cout << "[Gap] : " << this->mip_gap << std::endl;
        std::cout << "[Temps] : " << this->solve_time << std::endl;
    } else {
        std::cout << "--- [Aucune solution trouvée dans le temps imparti] ---" << std::endl;
        std::cout << "[Temps] : " << this->solve_time << std::endl;
    }

}

