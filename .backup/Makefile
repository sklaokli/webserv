# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/06/24 06:39:09 by sklaokli          #+#    #+#              #
#    Updated: 2026/09/04 21:56:28 by sklaokli         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		:=	webserv

SRC_DIR		:=	src
OBJ_DIR		:=	bin
INC_DIR		:=	include

FILES		:=	main.cpp \
				utils/Utils.cpp \
				utils/Logger.cpp \
				config/Config.cpp \
				config/ConfigParser.cpp \
				network/Poller.cpp \
				network/Client.cpp \
				network/Server.cpp \
				http/HttpRequest.cpp \
				http/HttpResponse.cpp \
				http/AutoIndex.cpp \
				http/Router.cpp \
				http/SessionManager.cpp \
				cgi/CgiHandler.cpp

SRC			:=	$(addprefix $(SRC_DIR)/, $(FILES))
OBJ			:=	$(addprefix $(OBJ_DIR)/, $(FILES:%.cpp=%.o))
DEP			:=	$(OBJ:%.o=%.d)

CXX			:=	c++

CXXFLAGS	+=	-Wall -Wextra -Werror
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
