# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: aindjare <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/08/23 15:57:56 by aindjare          #+#    #+#              #
#    Updated: 2025/09/08 15:48:04 by aindjare         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		:=	webserv
SRCS		:=	$(wildcard *.cpp)
OBJS		:=	$(SRCS:.cpp=.o)
DEPS		:=	$(OBJS:.o=.d)

CXX			:=	c++
CXXFLAGS	:=	-Wall -Wextra -Werror -Wconversion -Wswitch-enum -std=c++98 -g -MMD -MP

all: $(NAME)

clean:
	$(RM) $(OBJS) $(DEPS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@	$^

-include $(DEPS)

.PHONY: all clean fclean re
.SECONDARY: $(OBJS) $(DEPS)
