#!/bin/bash

# ==============================================================================
# CONFIGURATION ET ENVIRONNEMENT
# ==============================================================================

if [ -f /etc/profile.d/modules.sh ]; then
    source /etc/profile.d/modules.sh
elif [ -f /usr/share/modules/init/bash ]; then
    source /usr/share/modules/init/bash
fi

# Charger le module Gurobi requis pour le binaire
module load gurobi/13.0.2

# Chemins d'accès
EXEC="/home/periat/LIP6/src/prog"
INST_DIR="/home/periat/LIP6/instances"
RES_DIR="/home/periat/LIP6/results"

# Paramètres passés à ton 'main' C++
# Format: ./prog <instance> <mode_execution> <elaging_LB2_choice> <writing_results>
MODE=0
ELAGING=1
WRITING_RESULTS=1

# Nombre de cœurs à utiliser en parallèle
NB_CORES=64

mkdir -p "$RES_DIR"

# ==============================================================================
# FONCTION DE TRAITEMENT D'UNE INSTANCE
# ==============================================================================
run_one_instance() {
    local file="$1"

    # Lancement direct et pur du binaire C++
    # Aucune redirection de fichier ni timeout côté Bash
    "$EXEC" "$file" "$MODE" "$ELAGING" "$WRITING_RESULTS"
}

export -f run_one_instance
export EXEC MODE ELAGING WRITING_RESULTS

# ==============================================================================
# EXECUTION PARALLELE
# ==============================================================================

# Distribution des instances triées par k croissant sur les cœurs
ls "$INST_DIR"/*.txt | sort -V -t'k' -k2 | parallel -j "$NB_CORES" run_one_instance
