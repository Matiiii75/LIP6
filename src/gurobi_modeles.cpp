#include "gurobi_modeles.hpp"


Gurobi_modeles::Gurobi_modeles(
    const Data& _data, 
    bool _enable_lazy_cuts, 
    double _time_limit
) : data(_data), enable_lazy_cuts(_enable_lazy_cuts), time_limit(_time_limit) {}


void Gurobi_modeles::modele_positions() {

    try {
        // GRBEnv et GRBModel sont déjà des objets RAII (l'API Gurobi gère
        // elle-même la libération des ressources sous-jacentes dans leur
        // destructeur) : pas besoin de new/delete, des objets locaux suffisent.
        GRBEnv env(true);
        env.set(GRB_IntParam_OutputFlag, 1);
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
        std::cout << "[ - - - Optimisation avec modèle basé sur les positions - - - ]" << std::endl << std::endl;
        model.optimize();

        int status = model.get(GRB_IntAttr_Status);

        if (status == GRB_OPTIMAL || status == GRB_TIME_LIMIT) {
            if (model.get(GRB_IntAttr_SolCount) > 0) {
                std::cout << "\n================ RESULTATS GUROBI ================\n";
                std::cout << "Statut : " << (status == GRB_OPTIMAL ? "OPTIMAL" : "TIME_LIMIT") << "\n";
                std::cout << "Cout minimal : " << model.get(GRB_DoubleAttr_ObjVal) << "\n";
                std::cout << "Best Bound : " << model.get(GRB_DoubleAttr_ObjBound) << "\n";
                std::cout << "Temps Gurobi : " << model.get(GRB_DoubleAttr_Runtime) << " s\n\n";

                std::cout << "Ordre des sommets trouve :\n";
                for (int i : all_pos) {
                    for (int j : all_nodes) {
                        if (x[i][j].get(GRB_DoubleAttr_X) > 0.5) {
                            std::cout << "Pos " << i << " : Sommet " << j << "\n";
                        }
                    }
                }
                std::cout << "==================================================\n";
            } else {
                std::cout << "[Gurobi] Temps limite atteint sans aucune solution trouvee.\n";
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


void Gurobi_modeles::modele_positions_relatives(bool lazy_cuts_on) {

    try {
        
        GRBEnv env(true);
        env.set(GRB_IntParam_OutputFlag, 1);
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

        // DÉFINITION VARIABLES 

        // z : zij = 1 si i est placé avant j 

        std::vector<std::vector<GRBVar>> z(nb_nodes, std::vector<GRBVar>(nb_nodes)); 

        for(int i = 0; i < nb_nodes; ++i) {
            for(int j = 0; j < nb_nodes; ++j) {
                std::string name = "z_" + std::to_string(i) + "_" + std::to_string(j); 
                z[i][j] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, name); 
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

                        model.addConstr(z[u][v] + z[v][w] + z[w][u] <= 2); 
                        model.addConstr(z[u][w] + z[w][v] + z[v][u] <= 2); 

                    }
                }
            }

        }

        // instanciation callbacks 

        My_callbacks Cb(puit, all_nodes, z); 

        if(lazy_cuts_on) {
            model.set(GRB_IntParam_LazyConstraints, 1); 
            model.setCallback(&Cb); 
        }

        // Résolution
        std::cout << "[ - - - Optimisation avec modèle basé sur les positions - - - ]" << std::endl << std::endl;
        model.optimize();

        int status = model.get(GRB_IntAttr_Status);

        if (status == GRB_OPTIMAL || status == GRB_TIME_LIMIT) {
            if (model.get(GRB_IntAttr_SolCount) > 0) {
                std::cout << "\n================ RESULTATS GUROBI ================\n";
                std::cout << "Statut : " << (status == GRB_OPTIMAL ? "OPTIMAL" : "TIME_LIMIT") << "\n";
                std::cout << "Cout minimal : " << model.get(GRB_DoubleAttr_ObjVal) << "\n";
                std::cout << "Best Bound : " << model.get(GRB_DoubleAttr_ObjBound) << "\n";
                std::cout << "Temps Gurobi : " << model.get(GRB_DoubleAttr_Runtime) << " s\n\n";

            } else {
                std::cout << "[Gurobi] Temps limite atteint sans aucune solution trouvee.\n";
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


My_callbacks::My_callbacks(int _puit, const std::vector<int>& _all_nodes, const std::vector<std::vector<GRBVar>>& _z) : 
    puit(_puit), all_nodes(_all_nodes), z(_z), nb_cuts_added(0) {}


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
                        handle_approx(val_z_uv); 
                        double val_z_vw = getSolution(z[v][w]); 
                        handle_approx(val_z_vw); 
                        double val_z_wu = getSolution(z[w][u]); 
                        handle_approx(val_z_wu); 

                        double val_z_uw = getSolution(z[u][w]); 
                        handle_approx(val_z_uw); 
                        double val_z_wv = getSolution(z[w][v]); 
                        handle_approx(val_z_wv); 
                        double val_z_vu = getSolution(z[v][u]); 
                        handle_approx(val_z_vu); 

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
