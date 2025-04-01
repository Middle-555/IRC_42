# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: kpourcel <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/04/01 17:07:53 by kpourcel          #+#    #+#              #
#    Updated: 2025/04/01 17:10:39 by kpourcel         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Nom de l'exécutable
NAME = ircserv

# Compilateur et options
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g

# Sources et Objets
SRC =	src/main.cpp\
		src/Server.cpp\
		src/Client.cpp\
		src/CommandHandler.cpp\
		src/Channel.cpp

OBJ_DIR = obj
OBJ = $(SRC:%.cpp=$(OBJ_DIR)/%.o)

# Default rule
all: $(NAME)

# Linking
$(NAME): $(OBJ)
	@$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ)
	@echo "✅ Compilation terminée avec succès pour $(NAME)"

# Create obj dir once
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

# Compile object files
$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean objects
clean:
	@rm -rf $(OBJ_DIR)
	@echo "🧹 Fichiers objets supprimés"

# Full clean
fclean: clean
	@rm -f $(NAME)
	@echo "🧼 Nettoyage complet effectué"

# Rebuild everything
re: fclean all

.PHONY: all clean fclean re
