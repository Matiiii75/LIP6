# 1. Variables de configuration
CXX = clang++
CXXFLAGS = # -fsanitize=address -g -Wall -Wextra -std=c++17
NAME = prog

# 2. Liste des fichiers (À METTRE À JOUR QUAND TU AJOUTES UN FICHIER)
SRC = main.cpp \
      common.cpp \
	  Master.cpp \
	  State_graph.cpp \
	  Data.cpp \
	  Heuristics.cpp

# 3. Transformation des .cpp en .o (fichiers objets)
OBJ = $(SRC:.cpp=.o)

# 4. Règle principale
all: $(NAME)

# Lien final (crée l'exécutable)
$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

# Règle pour compiler chaque fichier .cpp en .o
# Le % est un joker qui correspond au nom du fichier
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 5. Nettoyage
clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re