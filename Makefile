# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: aindjare <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/08/23 15:57:56 by aindjare          #+#    #+#              #
#    Updated: 2025/09/16 14:58:35 by aindjare         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		:=	webserv
SRCS		:=	main.cpp  # $(wildcard *.cpp)
OBJS		:=	$(SRCS:.cpp=.o)
DEPS		:=	$(OBJS:.o=.d)
INCS		:=	$(wildcard *.hpp)

CXX			:=	c++
CXXFLAGS	:=	-Wall -Wextra -Werror -Wconversion -Wswitch-enum -std=c++98 -g -MMD -MP

all: $(NAME)

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(DEPS)
	$(RM) $(NAME)

re: fclean all

$(NAME): $(INCS) $(SRCS)
	$(CXX) -o $@	$(SRCS)

-include $(DEPS)

.PHONY: all clean fclean re
.SECONDARY: $(OBJS) $(DEPS)
