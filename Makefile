# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: xenobas <rahimos.123@gmail.com>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/12/11 18:03:31 by xenobas           #+#    #+#              #
#    Updated: 2025/12/17 20:00:03 by aindjare         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		:=	webserv
SRCS		:=	$(wildcard src/*.cpp)
INCS		:=	$(wildcard src/*.hpp)
OBJS		:=	$(SRCS:.cpp=.o)
DEPS		:=	$(OBJS:.o=.d)

CXX			:=	c++
CXXFLAGS	:=	-std=c++98 -ggdb -MMD -MP \
				-DDEBUG \
				-Wall -Wextra -Werror -Wconversion -Wswitch-enum 

all: $(NAME)

clean:
	$(RM) $(DEPS)
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

san: CXXFLAGS += -fsanitize=address
san: fclean all

$(NAME): $(INCS) $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@	$(OBJS)

-include $(DEPS)

.PHONY: all clean fclean re
.SECONDARY: $(OBJS) $(DEPS)
