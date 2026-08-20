#!/bin/bash

# ==============================================================================
# UTILISATION DU SCRIPT
# ==============================================================================
# Ce script permet de lancer les benchmarks en parallèle sur un dossier d'instances.
# Il peut être exécuté localement ou soumis sur un noeud de calcul via OAR.
#
# SYNTAXE :
#   ./nom_du_script.sh [MODE]
#
# ARGUMENT (Optionnel) :
#   Si aucun argument n'est fourni, le mode par défaut (fpt_elag) sera utilisé.
#
# MODES DISPONIBLES :
#   fpt               : Algorithme FPT exact en C++ sans élagage
#   fpt_elag          : Algorithme FPT exact en C++ avec élagage (DÉFAUT)
#   fpt_pre_treatment : Algorithme FPT exact en C++ avec pré-traitement 
#   saa               : Métaheuristique (Recuit simulé)
#   gurobi_pos        : Solveur Gurobi - Modèle PLNE sur les positions absolues
#   gurobi_rel        : Solveur Gurobi - Modèle PLNE sur les positions relatives (statique)
#   gurobi_rel_lazy   : Solveur Gurobi - Modèle PLNE sur les positions relatives avec Lazy Constraint
#
# EXEMPLES D'EXÉCUTION :
#   1. Lancement local avec le mode par défaut (fpt_elag) :
#      ./nom_du_script.sh
#
#   2. Lancement local avec le modèle Gurobi et Lazy Constraints :
#      ./nom_du_script.sh gurobi_rel_lazy
#
#   3. Soumission sur le cluster (OAR) en demandant 8 coeurs :
#      oarsub -l /nodes=1/core=8 -S "./nom_du_script.sh gurobi_rel_lazy"
# ==============================================================================

# Mode par défaut si aucun argument n'est passé à oarsub
MODE_CIBLE="${1:-fpt_elag}"

# Chemins et ressources
INST_DIR="/home/periat/LIP6/instances"
RES_DIR="/home/periat/LIP6/results"
EXEC_CPP="/home/periat/LIP6/src/prog"
EXEC_GUROBI="/home/periat/LIP6/src/prog_gurobi"

NB_CORES=8
WRITING_RESULTS=1
ELAG_PERCENT_VALUES=(0.3 0.5 0.7 0.8)

# ==============================================================================
# 2. DÉDUCTION AUTOMATIQUE DES PARAMÈTRES
# ==============================================================================
TYPE_EXEC=""
MODE=0
ELAGING=0
GUROBI_MODELE=0
LAZY=0

case "$MODE_CIBLE" in
    fpt)
        TYPE_EXEC="prog"
        MODE=0
        ELAGING=0
        ;;
    fpt_elag)
        TYPE_EXEC="prog"
        MODE=0
        ELAGING=1
        ;;
    fpt_pre_treatment)
        TYPE_EXEC="prog"
        MODE=2
        ELAGING=0
        ;;
    saa)
        TYPE_EXEC="prog"
        MODE=1
        ELAGING=0
        ;;
    gurobi_pos)
        TYPE_EXEC="gurobi"
        GUROBI_MODELE=0
        LAZY=0
        ;;
    gurobi_rel)
        TYPE_EXEC="gurobi"
        GUROBI_MODELE=1
        LAZY=0
        ;;
    gurobi_rel_lazy)
        TYPE_EXEC="gurobi"
        GUROBI_MODELE=1
        LAZY=1
        ;;
    *)
        echo "Erreur : Mode '$MODE_CIBLE' inconnu."
        echo "Modes valides : fpt | fpt_elag | fpt_pre_treatment | saa | gurobi_pos | gurobi_rel | gurobi_rel_lazy"
        exit 1
        ;;
esac

# ==============================================================================
# 3. RÉSUMÉ DANS LES LOGS OAR (Écrit dans le fichier .stdout)
# ==============================================================================
echo "========================================"
echo " JOB OAR - RECAPITULATIF DE CONFIGURATION"
echo "========================================"
echo "- Date de lancement  : $(date)"
echo "- Node / Host        : $(hostname)"
echo "- Mode selectionne   : $MODE_CIBLE"
echo "- Executable utilise : $TYPE_EXEC"
if [[ "$TYPE_EXEC" == "prog" ]]; then
    echo "- Mode (0=CP, 1=SAA, 2=FPT+Pre-trait) : $MODE"
    echo "- Elagage                           : $ELAGING"
else
    echo "- Modele PL          : $GUROBI_MODELE"
    echo "- Lazy cuts          : $LAZY"
fi
echo "- Coeurs alloues     : $NB_CORES"
echo "========================================"

# ==============================================================================
# 4. ENVIRONNEMENT MODULES & DOSSIERS
# ==============================================================================
if [ -f /etc/profile.d/modules.sh ]; then
    source /etc/profile.d/modules.sh
elif [ -f /usr/share/modules/init/bash ]; then
    source /usr/share/modules/init/bash
fi

module load gurobi/13.0.2
mkdir -p "$RES_DIR"

# ==============================================================================
# 5. FONCTION DE TRAITEMENT D'UNE INSTANCE
# ==============================================================================
run_one_instance() {
    
    local file="$1"
    local filename=$(basename "$file") # on extrait le nom du fichier du chemin 
    local n=$(echo "$filename" | cut -d'_' -f1) # on coupe le texte au premier '_' pr extraitre la taille n 

    if [[ "$TYPE_EXEC" == "gurobi" ]]; then
        
        if ((n>500)); then # si n > 500
            return 0 # ça quitte la fonction proprement (comme un continue finalement), l'instance est ignorée 
        fi

        echo "start $filename (gurobi)" # print qui informe qu'on démarre l'instance en question
        "$EXEC_GUROBI" "$file" "$GUROBI_MODELE" "$LAZY" "$WRITING_RESULTS" # éxécution de la commande 
        echo "done $filename (gurobi)" # print qui informe qu'on a terminé de traiter l'instance 

    else
        if (( ELAGING == 1 && MODE == 0 )); then 
            for ELAG_CHOICE in "${ELAG_PERCENT_VALUES[@]}"; do
                "$EXEC_CPP" "$file" "$MODE" "$ELAGING" "$WRITING_RESULTS" "$ELAG_CHOICE"
            done    
        else # EXECUTION SAA OU BIEN ALGO FPT SANS ELAGAGE 
            "$EXEC_CPP" "$file" "$MODE" "$ELAGING" "$WRITING_RESULTS"
        fi
    fi
}

export -f run_one_instance
export TYPE_EXEC EXEC_CPP EXEC_GUROBI MODE ELAGING WRITING_RESULTS ELAG_PERCENT_VALUES 
export GUROBI_MODELE LAZY

# ==============================================================================
# 6. EXECUTION PARALLELE
# ==============================================================================
echo "Demarrage des calculs..."
ls "$INST_DIR"/*.txt | sort -V -t'k' -k2 | parallel -j "$NB_CORES" run_one_instance {}
echo "Fin des calculs : $(date)"