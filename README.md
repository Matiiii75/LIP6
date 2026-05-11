4 fichiers : 

Data.cpp : 

    implémentations de la structure "KeyHash" qui contient 
    -> r & l parties droite et gauche (chacun 64 bits) pour former un entiers 128 bits
    -> redéfinition de quelques operators pr le KeyHash (xor, ==)

    implémentations de la structure "Data" qui contient 
    
    -> dag initial G
    -> reverse_dag pour savoir en O(1) les prédécesseurs directs de tout sommet
    -> la fermeture transitive du dag 
    -> node_to_hash qui à chaque sommet de G associe son hash 
    -> readfile pour lire G depuis un .txt (doit avoir un unique sommet s (resp. t) de degré entrant (resp. sortant) = 0 )

    
common.cpp : 

    Quelques fonction communes à plusieurs de mes algorithmes. Ne pas se fier au nom donné aux fonction qui sont tout de même très spécifiques 


State_graph.cpp : 

    Attributs : 
    - s & t : source et puit de G
    - data : référence constante sur les données du dag
    - ID_to_cands : A chaque ID (index du vecteur), associe l'ensemble candidat (ou Bordure externe selon comment on l'appele)
    - hash_to_ID : à chaque Keyhash, associe l'ensemble de ID qui lui sont associés (deux ID peuvent être associés à un même hash avec un très faible proba comme on a vu)
    - SG : encodage du graphe d'états par liste d'adjacence
    - weights : pour mon problème, certaines fonctions objectifs permettent de pondérer seulement les sommets de SG c'est ce que je fais ici
    
    Méthodes : 

    - Constructeur : récupère les données data, construit le premier sommet de SG et détermine son hash
    - des fonctions qui vérifies l'existence d'un candidat dans SG, qui calculent le 1er candidat, qui calculent le poids qui ajoute un arc (C1,C2)


Master.cpp

    C'est la classe qui gère le programme. On l'appele depuis le main en lui donnant les données, elle appelle State_graph, stocke le pcc en même temps qu'elle construit le SG, et implémente la logique principale de l'algo de complexité paramétrée que j'étudie. 
    -> La fonction "compute_cut_set" devrait t'être utile, c'est celle ci qui permet de retrouver les éléments "déjà choisis pour l'ordre topologique (sans retenir l'ordre exact)", comme encodé dans le noeud courant (ensemble candidat) du graphe d'état. 


Heuristics.cpp : 

    Inutile pour toi, permet seulement de résoudre rapidement mon probleme et de trouver une bonne borne primale sur la valeur de l'objectif, afin d'ensuite élaguer des branches du graphe d'états à leur création


Compiler le projet : 

créer un dossier "instances" à la racine du projet, compiler avec le makefile (cmd "make") à la racine, 
puis éxécuter "./prog ../instances/nom_instance"

