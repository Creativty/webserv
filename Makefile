# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: aindjare <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/08/23 15:57:56 by aindjare          #+#    #+#              #
#    Updated: 2025/10/27 13:44:51 by aindjare         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		:=	webserv
SRCS		:=	$(wildcard *.cpp)
INCS		:=	$(wildcard *.hpp)
OBJS		:=	$(SRCS:.cpp=.o)
DEPS		:=	$(OBJS:.o=.d)

CXX			:=	c++
CXXFLAGS	:=	-Wall -Wextra -Werror -Wconversion -Wswitch-enum -std=c++98 -g -MMD -MP $$WEBSERV_DEFINES

all: $(NAME)

clean:
	$(RM) $(DEPS)
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

$(NAME): $(INCS) $(OBJS)
	$(CXX) -o $@	$(OBJS)

-include $(DEPS)

.PHONY: all clean fclean re
.SECONDARY: $(OBJS) $(DEPS)
