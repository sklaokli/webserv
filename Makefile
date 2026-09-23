# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: sklaokli <sklaokli@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/06/24 06:39:09 by sklaokli          #+#    #+#              #
#    Updated: 2026/09/16 00:55:42 by sklaokli         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		:=	webserv

SRC_DIR		:=	src
OBJ_DIR		:=	bin
INC_DIR		:=	include

SRC_MAIN	:=	main.cpp

SRC_CONFIG	:=	Config.cpp \
				ServerConfig.cpp \
				LocationConfig.cpp

SRC_PARSER	:=	Lexer.cpp \
				Parser.cpp

SRC_UTILS	:=	Logger.cpp \
				Utils.cpp

SRC			:=	$(addprefix $(SRC_DIR)/, $(SRC_MAIN)) \
				$(addprefix $(SRC_DIR)/config/, $(SRC_CONFIG)) \
				$(addprefix $(SRC_DIR)/parser/, $(SRC_PARSER)) \
				$(addprefix $(SRC_DIR)/utils/, $(SRC_UTILS))

SRC_HTTP := PathResolver.cpp HttpResponse.cpp MimeTypes.cpp AutoIndex.cpp Router.cpp
SRC += $(addprefix $(SRC_DIR)/http/, $(SRC_HTTP))

OBJ			:=	$(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
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

ROUTER_OBJ := $(filter-out $(OBJ_DIR)/main.o,$(OBJ))
ROUTER_DEMO := $(OBJ_DIR)/router_demo

$(ROUTER_DEMO): $(ROUTER_OBJ) tests/router_demo.cpp
	$(CXX) $(CXXFLAGS) tests/router_demo.cpp $(ROUTER_OBJ) -o $@

route-demo: $(ROUTER_DEMO)

test-router: $(ROUTER_DEMO)
	python3 tests/test_router.py $(ROUTER_DEMO)

.PHONY: route-demo test-router
