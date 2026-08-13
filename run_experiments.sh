#!/bin/bash

# ==============================================================================
# 1. CONFIGURATION EN DUR (Modifiable directement si pas d'argument)
# ==============================================================================
# Modes disponibles : 
#   - "fpt"             : CP sans élagage
#   - "fpt_elag"        : CP avec élagage
#   - "saa"             : Recuit simulé
#   - "gurobi_pos"      : Modèle Gurobi Positions simples
#   - "gurobi_rel"      : Modèle Gurobi Positions relatives sans lazy cuts
#   - "gurobi_rel_lazy" : Modèle Gurobi Positions relatives AVEC lazy cuts

# Mode par défaut si aucun argument n'est passé à oarsub
MODE_CIBLE="${1:-fpt_elag}"

# Chemins et ressources
INST_DIR="/home/periat/LIP6/instances"
RES_DIR="/home/periat/LIP6/results"
EXEC_CPP="/home/periat/LIP6/src/prog"
EXEC_GUROBI="/home/periat/LIP6/src/prog_gurobi"

NB_CORES=64
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
        echo "Modes valides : fpt | fpt_elag | saa | gurobi_pos | gurobi_rel | gurobi_rel_lazy"
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
    echo "- Mode (0=CP, 1=SAA) : $MODE"
    echo "- Elagage            : $ELAGING"
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
    
    if [[ "$TYPE_EXEC" == "gurobi" ]]; then
        "$EXEC_GUROBI" "$file" "$GUROBI_MODELE" "$LAZY" "$WRITING_RESULTS"
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