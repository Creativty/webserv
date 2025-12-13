# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: xenobas <rahimos.123@gmail.com>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/12/11 18:03:31 by xenobas           #+#    #+#              #
#    Updated: 2025/12/11 18:03:32 by xenobas          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		:=	webserv
SRCS		:=	$(wildcard *.cpp)
INCS		:=	$(wildcard *.hpp)
OBJS		:=	$(SRCS:.cpp=.o)
DEPS		:=	$(OBJS:.o=.d)

CXX			:=	c++
<<<<<<< HEAD
CXXFLAGS	:=	-Wall -Wextra -Werror -Wconversion -Wswitch-enum -std=c++98 -ggdb -MMD -MP $$WEBSERV_DEFINES -DWEBSERV_DEBUG -D_GLIBCXX_USE_CXX11_ABI=0
=======
CXXFLAGS	:=	-std=c++98 -ggdb -MMD -MP \
				-DDEBUG \
				-Wall -Wextra -Werror -Wconversion -Wswitch-enum 
>>>>>>> 4784aa75fdb43a8fb04e245ec4198174a17a4c9e

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
