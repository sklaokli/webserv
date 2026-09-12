# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/06/24 06:39:09 by sklaokli          #+#    #+#              #
#    Updated: 2026/09/13 03:02:21 by sklaokli         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		:=	webserv

SRC_DIR		:=	src
OBJ_DIR		:=	bin
INC_DIR		:=	include

FILES		:=	main.cpp \
				Logger.cpp \
				Parser.cpp \
				Utils.cpp


SRC			:=	$(addprefix $(SRC_DIR)/, $(FILES))
OBJ			:=	$(addprefix $(OBJ_DIR)/, $(FILES:%.cpp=%.o))
DEP			:=	$(OBJ:%.o=%.d)

CXX			:=	c++

# CXXFLAGS	+=	-Wall -Wextra -Werror
CXXFLAGS	+=	-std=c++98 -pedantic
CXXFLAGS	+=	-MMD -MP
CXXFLAGS	+=	-I$(INC_DIR)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

-include $(DEP)

.PHONY: all clean fclean re
