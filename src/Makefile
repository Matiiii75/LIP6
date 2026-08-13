# 1. Variables de configuration
CXX = g++
CXXFLAGS = -O3 -g -std=c++17 -Wall -Wextra

# --- DÉTECTION DU SYSTÈME ET DU MODULE GUROBI ---
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Darwin)
    # Configuration macOS (Apple Silicon / arm64)
    GUROBI_DIR = /Library/gurobi1302/macos_universal2
    GUROBI_INC = -I$(GUROBI_DIR)/include
    GUROBI_LIB = -L$(GUROBI_DIR)/lib -lgurobi_c++ -lgurobi130 -Wl,-rpath,$(GUROBI_DIR)/lib
else
    # Configuration Linux / Cluster LIP6
    GUROBI_DIR = $(GUROBI_HOME)
    GUROBI_INC = -I$(GUROBI_DIR)/include
    GUROBI_LIB = -L$(GUROBI_DIR)/lib -lgurobi_c++ -lgurobi130 -Wl,-rpath,$(GUROBI_DIR)/lib
endif

CXXFLAGS += $(GUROBI_INC)
# --- FIN CONFIGURATION GUROBI ---

# Noms des deux exécutables finaux
EXEC_CPP = prog
EXEC_GUROBI = prog_gurobi

# 2. Séparation stricte des fichiers source
# A) Fichiers partagés (SANS Gurobi et SANS main)
SRC_COMMON = common.cpp \
             Master.cpp \
             State_graph.cpp \
             Data.cpp \
             Heuristics.cpp \
             Timer.cpp \
             User_choices.cpp

# B) Fichiers spécifiques à Gurobi
SRC_GUROBI = gurobi_modeles.cpp \
             main_gurobi_modeles.cpp

# C) Point d'entrée pour le binaire purement C++ (FPT / SAA)
SRC_MAIN_CPP = main.cpp                  # Adapte le nom si ton main C++ s'appelle autrement !

# 3. Transformation des .cpp en .o
OBJ_COMMON   = $(SRC_COMMON:.cpp=.o)
OBJ_GUROBI   = $(SRC_GUROBI:.cpp=.o)
OBJ_MAIN_CPP = $(SRC_MAIN_CPP:.cpp=.o)

ALL_OBJ = $(OBJ_COMMON) $(OBJ_GUROBI) $(OBJ_MAIN_CPP)

# 4. Règles de construction
all: $(EXEC_CPP) $(EXEC_GUROBI)

# Exécutable 1 : Programme C++ classique (FPT / SAA) -> Aucune dépendance à la lib Gurobi
$(EXEC_CPP): $(OBJ_MAIN_CPP) $(OBJ_COMMON)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Exécutable 2 : Programme Gurobi -> Inclus gurobi_modeles.o ET lie la lib Gurobi
$(EXEC_GUROBI): $(OBJ_GUROBI) $(OBJ_COMMON)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(GUROBI_LIB)

# Règle générique de compilation des fichiers objets .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 5. Nettoyage
clean:
	rm -f $(ALL_OBJ) $(EXEC_CPP) $(EXEC_GUROBI) gurobi.log

fclean: clean
	rm -f $(EXEC_CPP) $(EXEC_GUROBI) gurobi.log

re: fclean all

.PHONY: all clean fclean re