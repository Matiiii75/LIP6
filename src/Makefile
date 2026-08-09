# 1. Variables de configuration
CXX = g++
CXXFLAGS = -O3 -g -std=c++17 -Wall -Wextra

# --- DÉTECTION DU SYSTÈME ET DU MODULE GUROBI ---
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Darwin)
    # Configuration macOS
    GUROBI_DIR = /Library/gurobi1302/macos_universal2
    GUROBI_INC = -I$(GUROBI_DIR)/include
    GUROBI_LIB = -L$(GUROBI_DIR)/lib -lgurobi_c++ -lgurobi130 -Wl,-rpath,$(GUROBI_DIR)/lib
else
    # Configuration Linux / Cluster LIP6
    # GUROBI_HOME est automatiquement défini par 'module load gurobi/13.0.2'
    GUROBI_DIR = $(GUROBI_HOME)
    GUROBI_INC = -I$(GUROBI_DIR)/include
    GUROBI_LIB = -L$(GUROBI_DIR)/lib -lgurobi_c++ -lgurobi130 -Wl,-rpath,$(GUROBI_DIR)/lib
endif

CXXFLAGS += $(GUROBI_INC)
# --- FIN CONFIGURATION GUROBI ---

NAME = prog

# 2. Liste des fichiers source
SRC = common.cpp \
      Master.cpp \
      State_graph.cpp \
      Data.cpp \
      Heuristics.cpp \
      Timer.cpp \
      gurobi_modeles.cpp \
      User_choices.cpp \
      main.cpp

# 3. Transformation des .cpp en .o
OBJ = $(SRC:.cpp=.o)

# 4. Règle principale
all: $(NAME)

# Lien final
$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME) $(GUROBI_LIB)

# Règle de compilation des fichiers objets .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 5. Nettoyage
clean:
	rm -f $(OBJ) prog gurobi.log

fclean: clean
	rm -f $(OBJ) $(NAME) gurobi.log

re: fclean all

.PHONY: all clean fclean re