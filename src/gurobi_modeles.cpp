#include "gurobi_modeles.hpp"


Gurobi_modeles::Gurobi_modeles(
    const Data& _data, 
    bool _choix_user,
    bool _enable_lazy_cuts, 
    double _time_limit
) : data(_data), choix_user(_choix_user), enable_lazy_cuts(_enable_lazy_cuts), time_limit(_time_limit) {

    if(choix_user) { // lancer position simple
        this->modele_positions_relatives(_enable_lazy_cuts, false); // relaxation = false; 
    } else { // lancer position relative 
        this->modele_positions(); 
    }

}


void Gurobi_modeles::modele_positions() {

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


void Gurobi_modeles::modele_positions_relatives(bool lazy_cuts_on, bool relaxation) {

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

        if(lazy_cuts_on == false) { // contraintes actives seulement si lazy cuts off 

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

        }

        // instanciation callbacks 

        My_callbacks Cb(puit, z); 

        if(lazy_cuts_on) {
            model.set(GRB_IntParam_LazyConstraints, 1); 
            model.setCallback(&Cb); 
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


My_callbacks::My_callbacks(int _puit, const std::vector<std::vector<GRBVar>>& _z) : 
    puit(_puit), z(_z), nb_cuts_added(0) {}


void My_callbacks::handle_approx(double& X) const 
{
    if(X >= 0.5) X = 1.0; 
    else X = 0.0; 
}


void My_callbacks::callback() {

    try 
    {
        if(where == GRB_CB_MIPSOL) { // si on a une solution ENTIÈRE 

            std::vector<violated_cut> all_violations;; 

            for(int u = 1; u < puit-2; ++u) {
                for(int v = u+1; v < puit-1; ++v) {
                    for(int w = v+1; w < puit; ++w) {

                        double val_z_uv = getSolution(z[u][v]);
                        double val_z_vw = getSolution(z[v][w]); 
                        double val_z_wu = getSolution(z[w][u]); 

                        double val_z_uw = getSolution(z[u][w]); 
                        double val_z_wv = getSolution(z[w][v]);
                        double val_z_vu = getSolution(z[v][u]); 

                        // check si ça viole 
                        if(val_z_uv + val_z_vw + val_z_wu > 2.0) {
                            violated_cut VC1(u,v,w); 
                            all_violations.push_back(VC1); 
                        } 

                        if(val_z_uw + val_z_wv + val_z_vu > 2.0) {
                            violated_cut VC2(u,w,v); 
                            all_violations.push_back(VC2); 
                        }

                    }
                }
            }

            if((int)all_violations.size() > 0) // detecté des coupes qui violent 
            {
                for(const violated_cut& vc : all_violations)
                {
                    addLazy(z[vc.u][vc.v] + z[vc.v][vc.w] + z[vc.w][vc.u] <= 2); 
                    nb_cuts_added++; 
                }
            }

        }
    } catch (const GRBException& e) {
        std::cerr << "Erreur dans le callback : " << e.getMessage() << std::endl;
    }

}


int My_callbacks::get_nb_cuts_added() const {
    return nb_cuts_added; 
}


void Gurobi_modeles::display_infos() const {

    std::cout << "----- [RESULTATS GUROBI] -----" << std::endl;    

    if(choix_user) {
        std::cout << "--- [MODELE POSITIONS RELATIVES] ---" << std::endl;
        std::cout << "--- [LAZY ACTIVÉES ?] : " << enable_lazy_cuts << std::endl;
    } else {
        std::cout << "--- [MODELE POSITIONS] ---" << std::endl;
    }

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

