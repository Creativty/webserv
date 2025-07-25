# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: aindjare <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/07/11 15:57:44 by aindjare          #+#    #+#              #
#    Updated: 2025/07/19 15:31:22 by aindjare         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		:=	webserv
SRCS		:=	$(wildcard *.cpp)
OBJS		:=	$(SRCS:.cpp=.o)
DEPS		:=	$(OBJS:.o=.d)

CXX			:=	c++
CXXFLAGS	:=	-Wall -Wextra -Werror -Wconversion -std=c++98 -g -MMD -MP

all: $(NAME)

clean:
	$(RM) $(OBJS) $(DEPS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

flags:
	@echo $(CXXFLAGS)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

-include $(DEPS)

.PHONY: all clean fclean re flags
.SECONDARY: $(OBJS) $(DEPS)
