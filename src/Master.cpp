#include "Master.hpp"


Pre_treatment::Pre_treatment(const Data& _data, int _s, int _t) :
    data(_data), nb_composantes(0), nb_sub_problems_solved(0), total_cand_generated(0)
{
    this->compute_composantes(_s, _t); // lance le calcule des composantes 
}


void Pre_treatment::compute_composantes(int s, int t) {

    std::vector<int> to_visit; 
    for(int u = 1; u < t; ++u) // remplir 
        to_visit.push_back(u); 

    std::vector<uint8_t> visited(data.dag_size, 0); // pour mémoriser ceux qu'on a visité 

    for(int u : to_visit) { 
        if(visited[u]) continue; 
        
        std::vector<int> composante; 
        visited[u] = 1; // retenir qu'on l'a visité 
        std::queue<int> fifo_list; 
        fifo_list.push(u); 

        while(!fifo_list.empty()) { 

            int curr = fifo_list.front(); // récup le premier sommet
            fifo_list.pop(); // le retirer 
            composante.push_back(curr); // l'ajouter a la composante 

            for(int curr_succ : data.dag[curr]) { // pr tt succ de curr
                if(curr_succ == t || visited[curr_succ]) continue; // ignorer le puit et les sommets déjà visités 
                visited[curr_succ] = 1; // on l'a visité
                fifo_list.push(curr_succ); // ajt a la fifo pr traiter ses voisis (qui seront dans la composante aussi du coup)
            }

            for(int curr_preds : data.reverse_dag[curr]) {
                if(curr_preds == s || visited[curr_preds]) continue; // ignorer source et sommets déjà visités 
                visited[curr_preds] = 1; 
                fifo_list.push(curr_preds); 
            }

        }
        composantes.push_back(composante); // ajt a l'ensemble des composantes 
    }

    this->nb_composantes = (int)composantes.size();

}


void Pre_treatment::add_composante_size(int size) {
    composantes_sizes.push_back(size); 
}


Dag Pre_treatment::create_dag_from_composante(const std::vector<int>& composante) const {

    int new_dag_size = (int)composante.size()+2; // car on ajt s & t 
    int initial_puit = (int)this->data.dag_size-1; // puis dans le dag initial
    int source = 0; 
    int puit = new_dag_size-1; // puit du nv dag 

    std::vector<int> new_to_old(new_dag_size); // new_to_old[i] = j -> j devient i 
    std::unordered_map<int,int> old_to_new; // otn[i] = j -> i est devenu j 

    new_to_old[source] = source; 
    old_to_new[source] = source;    // les nv s & t sont associés à eux-même
    new_to_old[puit] = puit;        
    old_to_new[puit] = puit; 

    int new_next_dispo = 1; // valeur du prochain nv noeud dispo
    for(int u : composante) {
        new_to_old[new_next_dispo] = u; 
        old_to_new[u] = new_next_dispo; 
        new_next_dispo++; // on passe au prochain 
    }

    // création du sous graphe 

    Dag new_dag(new_dag_size);
    std::vector<bool> has_0_pred(new_dag_size, true); // true -> degré entrant nul
    
    for(int u : composante) { // pr chq sommet de la composante 
        int new_u = old_to_new.at(u); // on récup sa nv valeur
        for(int u_neigh : this->data.dag[u]) { 
            if(u_neigh == initial_puit) continue; 
            int new_neigh_u = old_to_new.at(u_neigh); // nv valeur du voisin
            has_0_pred[new_neigh_u] = false; 
            new_dag[new_u].push_back(new_neigh_u); // ajt ds nv dag 
        }
    }

    // connecter source et puit dans le nv dag 

    for(int u = 1; u < puit; ++u) { 
        // source 
        if(has_0_pred[u]) // si il a 0 pred -> connecter a la source 
            new_dag[source].push_back(u); 
        // puit 
        if((int)new_dag[u].size() == 0) { // si ps de succ 
            new_dag[u].push_back(puit);   // connecter puit
        }
    }

    return new_dag; 

}


Master::Master(
    const Data& _data, 
    int _s, int _t, 
    double _time_limit, 
    const User_choices& _user_choices) : 
    data(_data), 
    SG(_data, _s, _t), 
    time_limit(_time_limit), 
    user_choices(_user_choices)
{
    master_time_data.start_timer(); // début du timer de Master

    if(_user_choices.elaging_LB2_ON) // lancer le calcul de SAA puisqu'on va utiliser l'élagage 
    {
        double initial_temp = -1; // on demande au constructeur de heuristic d'appele init_temp()
        Heuristics h(_data, initial_temp, 100000); 
        h.SAA_optimize(); // on résoud avec recuit simulé
        SAA_value = h.obj_val; // on récup la valeur calculée 
        std::cout << "SAA value : " << SAA_value << std::endl;
        set_first_cand_LB2_DSC(); // on calcule LB2 POUR S = {}
        nb_elaged_branch_by_LB2_DSC = 0; 
        this->size_begin_elag = (int)(_data.dag_size * _user_choices.elaging_LB2_percentage); 
    }

    if(_user_choices.composantes_pre_treatment_ON) { // si le pré-traitement par composantes connexes est actif 
        // emplace() passe les arguments directement au constructeur de Pre_treatment 
        pre_treatment.emplace(_data, _s, _t);  
    }
}


void Master::compute_cut_set(const std::vector<int>& cand, int& cut_set_size,
    std::vector<uint8_t>& cut_set, std::vector<int>& hors_cut_set) const {

    // On se rappelle que pour calculer le cut-set, il faut déterminer les éléments 
    // qui sont pas dans N_G^+(cand). On met tous les éléments à 1. On parcours N_G^+(cand)
    // grâce à la transitive closure et on met 0 pr chq sommet qu'on rencontre. 

    for(const int c : cand) { // pr chq candidat 
        for(int i = 0; i < data.dag_size; ++i) { // pr chq noeud du graphe
            if(data.TC[c][i] && cut_set[i]) { // si c->i && on a pas encore vu i 
                cut_set[i] = 0; // si c -> i, i n'est pas dans le cut-set 
                hors_cut_set.push_back(i); 
            }
        }
    }

    cut_set_size = data.dag_size - (int)hors_cut_set.size(); 
}


std::vector<int> Master::rebuild_opt_order() const {

    std::vector<int> ordre_topo; 
    int curr_node = (int)pred_in_pcc.size() - 1; // on récup le dernier ens candidat ( {t} )

    while(curr_node != 0) {

        ordre_topo.push_back(pred_in_pcc[curr_node].second); 
        curr_node = pred_in_pcc[curr_node].first; 

    }

    std::reverse(ordre_topo.begin(), ordre_topo.end()); 
    return ordre_topo; 
}


void Master::extract_results() {

    this->optimal_order = rebuild_opt_order(); 
    this->optimal_value = best_dist_DSC.back(); 

    if(this->checker_DSC(this->optimal_order, this->optimal_value) == false) {
        throw std::runtime_error("Master::checker_DSC -> solution non valide"); 
    }

    this->nb_hash_generated = SG.hash_to_ID.size(); 
    this->nb_candidats = SG.ID_to_cands.size(); 

}


int Master::get_nb_cands_generated() const {
    return SG.ID_to_cands.size(); 
}


void Master::display_results(
    bool display_inst_name,
    bool display_n_and_k,  
    bool display_opt_order, 
    bool display_time, 
    bool display_opt_val, 
    bool hash_infos,
    bool display_LB2_elaging_infos
) const {

    std::cout << "-------------------------------------------------------------------------"; 
    std::cout << std::endl;

    std::cout << "               [----- AFFICHAGES RÉSULTATS -----]" << std::endl;
    std::cout << std::endl;

    if(display_inst_name) {
        std::cout << "[Instance]              : " << this->data.instance_name << std::endl;
    }

    if(display_n_and_k) {
        std::cout << "[Dag size & Degeneracy] : "; 
        std::cout << this->data.dag_size << " / "; 
        std::cout << this->data.degenerascy << std::endl;
    }

    if(display_opt_order) {
        std::cout << "[Ordre topologique optimal] : " << std::endl;
        for(int i : this->optimal_order) 
            std::cout << i << ", "; 
        std::cout << std::endl;
    }

    if(display_time) {
        std::cout << "[Temps total]           : "; 
        std::cout << this->total_time << " sec" << std::endl;
    }

    if(display_opt_val) 
        std::cout << "[valeur optimale]       : " << this->optimal_value << std::endl;


    if(hash_infos) {
        std::cout << "[nombre de hash généré] : " << this->nb_hash_generated << std::endl;
        std::cout << "[nombre de candidats]   : " << this->nb_candidats << std::endl;
    }

    if(display_LB2_elaging_infos && user_choices.elaging_LB2_ON) {
        std::cout << "[elag_from_size]        : " << this->size_begin_elag << std::endl;
        std::cout << "[nombre de sommets duquel démarre un élagage] : ";
        std::cout << this->nb_elaged_branch_by_LB2_DSC << std::endl;
    }

    std::cout << std::endl;
    std::cout << "-------------------------------------------------------------------------"; 

}


void Master::display_results_pre_treatment() const {

    std::cout << "-------------------------------------------------------------------------"; 
    std::cout << std::endl;

    std::cout << "               [----- AFFICHAGES RÉSULTATS -----]" << std::endl;
    std::cout << std::endl;

    std::cout << "[Instance]              : " << this->data.instance_name << std::endl;
    
    std::cout << "[Dag size & Degeneracy] : "; 
    std::cout << this->data.dag_size << " / "; 
    std::cout << this->data.degenerascy << std::endl;

    std::cout << "[Temps total]           : "; 
    std::cout << this->total_time << " sec" << std::endl;

    std::cout << "[valeur optimale]       : " << this->optimal_value << std::endl;

    std::cout << "[nombre de candidats]   : " << this->nb_candidats << std::endl;

    std::cout << "[nombre de composantes] : " << this->pre_treatment->nb_composantes; 
    std::cout << std::endl;

    std::cout << "[graphes états générés] : " << this->pre_treatment->nb_sub_problems_solved; 
    std::cout << std::endl; 

    std::cout << std::endl;
    std::cout << "-------------------------------------------------------------------------";

}


int Master::get_DSC_optimal_value() const {
    return this->best_dist_DSC.back(); 
}


void Master::set_first_cand_LB2_DSC() {

    int first_cand_LB2 = 0; 
    for(int u = 0; u < data.dag_size; ++u) 
        first_cand_LB2 += SG.taille_blocages_hors_cut[u]; 
    
    this->lower_bounds_2_DSC[0] = first_cand_LB2; 
}


int Master::compute_delta_LB2_DSC(int gamma, const std::vector<uint8_t>& cut_set, 
    const std::vector<int>& hors_cut_set) const 
{
    // PARTIE IMPACT SUR ICS

    // mémorisons les sommets de cut_set pr lesquels on a déja eliminé gamma de leur blocage 
    std::vector<uint8_t> seen(data.dag_size, 0);
    int ics_delta = 0; 

    for(int v : hors_cut_set) { // on va chercher les u dans S qui perdent une participatio à leur ensemble de blocage lambda(u) par mouvement du gamma
        if(v == SG.t) continue; // ignorer le puit 
        if(data.TC[gamma][v] == 0) continue; // si gamma -/-> v : ignorer 
        for(int u : data.reverse_dag[v]) { // pr tt pred direct de v 
            if(u == SG.s || u == gamma || cut_set[u] == 0 || seen[u]) continue; // ignorer source, u \in V-S ou u déjà vus 
            ics_delta--; // si on arrive là -> alors on a trouvé un u dont le lambda(u) perd un élément (gamma)
            seen[u] = 1; // u a été vux
        }
    }

    // on a pas traité les u dans S qui sont pred direct de gamma (puisque v ne peut pas = gamma car gamma \in cut_set)
    // deux cas peuvent se présenter : 
    //  (1) -> ils ont d'autre succ direct hors-cut-set et gamma n'est pas sur un chemin vers eux : 
    //        Alors, faire entrer gamma change la taille de lambda(u) (-1)
    //  (2) -> ils ont que gamma comme succ hors-cut-set : 
    //        Alors, faire entre gamma change la taille de lambda(u) de 1 à 0 et c'est déjà compté dans delta_pdscv

    for(int u : data.reverse_dag[gamma]) {
        if(u == SG.s || cut_set[u] == 0 || seen[u]) continue; 

        // ici, on sait que gamma bloquait u, et qu'il ne le bloque plus. 
        // dans le cas où gamma n'est pas le seul bloquant de u, alors u a au moins un autre succ direct 
        // dans hors-cut-set. Et donc, ics_value--
        // Mais, si gamma est le seul bloquant de u, alors la taille de lambda(u) passe de 1 à 0, 
        // mais on ne doit pas décrément ics_value car c'est compté dans delta_pdscv

        bool only_gamma_is_blocking = true; 
        
        for(int neigh_u : data.dag[u]) { // pr tt succ direct de u 
            if(neigh_u == gamma) continue; 
            if(cut_set[neigh_u] == 0) { // on a trouvé un autre bloquant de u hors cut set 
                only_gamma_is_blocking = false; 
                break; 
            } 
        }

        if(only_gamma_is_blocking == false) { // si un autre sommet que gamma bloquait 
            ics_delta--; // on a trouvé un autre bloquant de u 
        } 

        seen[u] = 1; // on a vu u 
    }

    // PARTIE IMPACT SUR GAMMA (et donc HCS)

    int hcs_delta = 0; 
    hcs_delta -= SG.taille_blocages_hors_cut[gamma]; // hcs perd la participation de Phi(gamma)

    // on calcule |lambda(gamma)| : 

    int lambda_gamma = 0; 
    for(int v : hors_cut_set) {
        if(v == SG.t) continue; 
        for(int w : data.dag[gamma]) {
            if(w == SG.t || data.TC[v][w] == 0) continue; 
            lambda_gamma++; 
            break; // prochain v 
        }
    }
    
    if(lambda_gamma > 0) // si gamma admet un ensemble de blocage de taille > 0 
        ics_delta += lambda_gamma - 1; // lambda gamma participe a la borne
    // on retire 1 car si gamma a au moins un successeur hors cut_set, 
    // alors il est compté dans SG.weight[C_ID] (delta_pdscv) 
    // où C_ID est l'ensemble candidat du cut-set où l'on vient d'ajouter gamma 

    return ics_delta + hcs_delta;
} 


int Master::compute_C_LB2_DSC(int C_ID, const std::vector<uint8_t>& cut_set, const std::vector<int>& hors_cut_set) const {

    int C_pred = pred_in_pcc[C_ID].first; // récup l'ID du prédécesseur optimal de C
    int gamma = pred_in_pcc[C_ID].second; // récup le candidat ajouté entre C_pred et C 
    
    int C_pred_LB; 
    auto it = lower_bounds_2_DSC.find(C_pred); // sécurité d'existence ici 
    if(it != lower_bounds_2_DSC.end()) C_pred_LB = it->second; // si on a trouvé une borne pour C_pred 
    else return SG.compute_LB2_from_C_DSC(cut_set, hors_cut_set, best_dist_DSC[C_ID]); 

    int delta_LB2 = compute_delta_LB2_DSC(gamma, cut_set, hors_cut_set);
    int delta_pdscv = SG.weights[C_ID]; // le poids de C_ID, c'est exactement ce qu'on rajoute a pdscv lors de l'ajout de gamma 

    return C_pred_LB + delta_pdscv + delta_LB2; // si delta_LB2 < 0, alors faire rentrer gamma en cut_set fait diminuer la borne
}


bool Master::try_elaging_LB2_DSC(int C_ID, const std::vector<uint8_t>& cut_set, const std::vector<int>& hors_cut_set) {

    if(C_ID == 0) return false; // C_ID = 0 -> il s'agit du first cand = {}

    int C_LB2 = compute_C_LB2_DSC(C_ID, cut_set, hors_cut_set); 
    lower_bounds_2_DSC[C_ID] = C_LB2; // on mémorise la borne de C_ID
    if(C_LB2 > SAA_value) 
        return true; // on peut élaguer 

    return false; 
}


void Master::build_SG_DSC() {   

    L.push(0); // ajouter l'ID du premier candidat  
    best_dist_DSC.push_back(0); // le coût pour aller au premier candidat est nul 
    pred_in_pcc.push_back({-1,-1}); 

    int iteration_count = 0; // compte les iter pr savoir quand vérifier le temps 
    bool stoped_prema = false; // permet de savoir si on a stoppé l'algo prématurémment 

    while(!L.empty()) 
    {  

        if(iteration_count % 10000 == 0) { // VERIFICATION < TIME_LIMIT 
            double temps_courant = master_time_data.get_temps_passe(); 
            if(temps_courant >= time_limit) {
                stoped_prema = true; 
                break; 
            }
        }

        int C_ID = L.front(); // on récupère l'ID de l'ens. cand. devans la FIFO
        L.pop(); // on l'efface 

        std::vector<int> C = SG.get_cand(C_ID); 
        int cut_set_size = 0; // 0 par défaut, on va l'incrémenter comme il faut dans compute_cut_set() 
        std::vector<uint8_t> cut_set(data.dag_size, 1); 
        std::vector<int> hors_cut_set; 
        compute_cut_set(C, cut_set_size, cut_set, hors_cut_set); // on calcule le cut_set associé
        
        // vérifier la borne LB2 
        if(user_choices.elaging_LB2_ON && (cut_set_size >= size_begin_elag) && try_elaging_LB2_DSC(C_ID, cut_set, hors_cut_set)) // true -> élagage 
        { 
            iteration_count++; 
            if(iteration_count % 25000 == 0)   
                std::cout << "elagage d'un noeud taille " << cut_set_size << std::endl;
            nb_elaged_branch_by_LB2_DSC++; 
            continue; 
        }
        
        std::vector<int> C2; 
        C2.reserve(C.size()-1); // on réserve l'espace pour copier le C 
    
        for(int i = 0; i < (int)C.size(); ++i) { // pr chq candidat de C

            C2 = C; // copier C, mettre le candidat en derniere pos, et le suppr 
            C2[i] = C2.back(); 
            C2.pop_back(); 

            int curr_c = C[i]; // copie c 

            cut_set[curr_c] = 1; // le candidat rentre dans le cut-set 

            for(const int u : data.dag[curr_c]) { // (l.7) : pr chq succ u du candidat

                if(is_included(data.reverse_dag[u], cut_set, -1)) // si tt les pred de u sont dans cut-set
                    C2.push_back(u); // -1 signifie qu'on regarde la simple inclusion de r-dag dans e-c-s 

            }

            increase_sort_vector(C2); // tri croissant 
            
            if(C2.size() == 0) continue; // si C2 est vide, inutile de continuer 

            keyHash C2_hash = compute_cand_hash(C2, data.node_to_hash); 
            int C2_ID = SG.is_cand_in_SG(C2, C2_hash); 

            if(C2_ID == -1) { // (l.10) (si il vaut -1 c'est qu'on a pas trouvé d'ID pr ce hash)
                
                SG.add_cand_to_SG(C2, C2_hash); 
                C2_ID = (int)SG.ID_to_cands.size()-1; // on vient de l'ajouter, son ID est le dernier index 
                L.push(C2_ID); // ajout à FIFO
                int C2_weight = SG.compute_weight_C_DSC(C2_ID, C_ID, curr_c, cut_set); // cut_set est le cut-set de C2 
                SG.set_weight(C2_ID, C2_weight); // on calcule le poids de C2 
                best_dist_DSC.push_back(std::numeric_limits<int>::max()); // inf par défaut 
                pred_in_pcc.push_back({-1,-1}); // ajout d'un sommet defaut pour garder pred_in_pcc bien indéxé 

            }

            SG.add_arc_from_C1_to_C2(C_ID, C2_ID); // ajoute l'arc 
            
            int dist_from_C = best_dist_DSC[C_ID] + SG.weights[C_ID]; 
            if(best_dist_DSC[C2_ID] > dist_from_C) { // voir si on améliore le pcc jusqu'à C2 en passant par C 
                
                best_dist_DSC[C2_ID] = dist_from_C; 
                pred_in_pcc[C2_ID] = {C_ID, curr_c}; // retenir d'où on vient  

            }

            // on fini la boucle sur curr_c 
            // -> on l'enlève du cut-set et passe au prochain candidat 
            cut_set[curr_c] = 0; 
        }

    }

    if(stoped_prema == false) // si pas d'arret prématuré -> on a trouvé une solution 
        found_solution = true; 

    this->total_time = master_time_data.get_temps_passe(); // on récupère le temps total de l'algorithme. 
}


void Master::solve_DSC_with_pre_treatment() {

    if(!user_choices.composantes_pre_treatment_ON) { // si on entre sans avoir activé le pré-traitement
        throw std::runtime_error 
        (
            "Master::solve_DSC_with_pre_treatment -> Erreur, l'user n'a pas activé le pré-traitement"
        );
    }

    int DSC_value = 0; // le total des sous probleme est la valeur pour le probleme principal

    for(auto& composante : pre_treatment->composantes) // pr chq composante 
    {      
        int taille_composante = (int)composante.size(); 

        if(taille_composante == 1) {
            pre_treatment->add_composante_size(1); 
            continue; // un sommet seul ne participe pas à DSC
        }
        else if(taille_composante == 2) {
            pre_treatment->add_composante_size(2);
            DSC_value++; // deux sommets ds un mm comp connexe, ne peuvent participer que de 1 
            continue; 
        }
        else if(taille_composante > 2) {

            pre_treatment->add_composante_size(taille_composante); 
            
            std::vector<std::vector<int>> sub_graph_induced; 
            // on récupère le sous graphe induit par les sommets de la composante
            sub_graph_induced = pre_treatment->create_dag_from_composante(composante);  
            Data data_sub_graph_induced(sub_graph_induced); // on créer l'objet data avec 
            
            int sub_graph_source = 0; 
            int sub_graph_puit = (int)sub_graph_induced.size()-1; 

            User_choices sub_pb_user_choices; // on laisse les valeurs par défaut 

            // sub_pb_user_choices.set_elaging_LB2_ON(); 
            // sub_pb_user_choices.set_elaging_LB2_percentage(0.1); 
            // donc pas de pré traitement (logique car ça marcherait pas)
            // pas d'élagage 

            double curr_time = master_time_data.get_temps_passe(); // on récup le temps actuel
            double temps_restant = time_limit - curr_time; 

            if(temps_restant < 0.001) // si on a atteint le temps max 
                break; // on sort de l'algo
            
            Master prog_sub_graph_induced // construction du sous probleme 
            (
                data_sub_graph_induced, 
                sub_graph_source, 
                sub_graph_puit,
                temps_restant, // appel avec le temps restant  
                sub_pb_user_choices
            ); 

            prog_sub_graph_induced.build_SG_DSC(); // lancement de l'algorithme pour le sous probleme

            if(prog_sub_graph_induced.found_solution) { // si on a pas arreté à cause du temps
                DSC_value += prog_sub_graph_induced.get_DSC_optimal_value(); 
                pre_treatment->total_cand_generated += prog_sub_graph_induced.get_nb_cands_generated(); 
            } else { // si on a arreté un sous-problème à cause du temps 
                this->found_solution = false; // alors on mémorise qu'on a pas trouvé de solution
                break; // on sort de la boucle sur les composantes 
            }

            pre_treatment->nb_sub_problems_solved++; 

        } else {
            throw std::runtime_error("Master::solve_DSC_with_pre_treatment -> composante size anormale"); 
        }
    }
    
    // on récupère certaines valeurs 
    this->optimal_value = DSC_value; // au pire, si on a résolu aucun pb, elle vaudra 0
    this->nb_candidats = pre_treatment->total_cand_generated; 
    this->total_time = master_time_data.get_temps_passe(); 
}


bool Master::checker_DSC(const std::vector<int>& ordre_topo, int val_found) const {


    // commencer par vérifier que c'est bien un ordre topologique valide
    // ie : vérifier que pour toute paire (u,v) suivant l'ordre topo, 
    // (v,u) notin A 

    for(int i = 0; i < (int)ordre_topo.size()-1; ++i) {
        for(int j = i+1; j < (int)ordre_topo.size(); ++j) {

            int node1 = ordre_topo[i]; // 1er dans l'ordre
            int node2 = ordre_topo[j]; // 2nd

            for(int neigh : data.dag[node2])
                if(neigh == node1) {
                    std::cout << node1 << "est un successeur direct de "
                        << node2 << " dans le dag initial " << std::endl;
                    return false; 
                } 

        }
    }

    // calculer la valeur de l'ordre topologique et regarder qu'on obtient bien la même 

    int obj_val = 0; // pr stocker la valeur objective 

    for(int i = 0; i < (int)ordre_topo.size()-1; ++i) { // pour chaque ordre 
        int valeur_ordre_i = 0;
        for(int j = 0; j <= i; ++j) { // pr chq sommet dans la coupe
            
            bool go_next_in_coupe = false;
            int node_in_coupe = ordre_topo[j]; // stocke le noeud ds la coupe
            for(int k = i+1; k < (int)ordre_topo.size(); ++k) { // pr chq sommet hors coupe
                
                int node_hors_coupe = ordre_topo[k]; // noeud hors coupe
                for(int neigh : data.dag[node_in_coupe]) {
                    if(neigh == node_hors_coupe) {
                        ++valeur_ordre_i; // on a trouvé un succ hors coupe -> ++
                        go_next_in_coupe = true; 
                        break; // aller au prochain noeud in-coupe
                    }
                }
                if(go_next_in_coupe) break; 
            }
        }
        obj_val += valeur_ordre_i; 
    }

    if(obj_val != val_found) { // check la valeur obj 
        std::cout << "Erreur dans la valeur calculée : " << std::endl;
        std::cout << "Valeur trouvée : " << val_found << std::endl;
        std::cout << "Valeur correcte : " << obj_val << std::endl;
        return false; 
    } 
    
    return true; 
}

