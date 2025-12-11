# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: sennakhl <sennakhl@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/08/23 15:57:56 by aindjare          #+#    #+#              #
#    Updated: 2025/11/20 09:50:38 by sennakhl         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		:=	webserv
SRCS		:=	$(wildcard *.cpp)
INCS		:=	$(wildcard *.hpp)
OBJS		:=	$(SRCS:.cpp=.o)
DEPS		:=	$(OBJS:.o=.d)

CXX			:=	c++
CXXFLAGS	:=	-Wall -Wextra -Werror -Wconversion -Wswitch-enum -std=c++98 -ggdb -MMD -MP $$WEBSERV_DEFINES -DWEBSERV_DEBUG -D_GLIBCXX_USE_CXX11_ABI=0

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
