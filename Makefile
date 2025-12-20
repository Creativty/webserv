# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: xenobas <rahimos.123@gmail.com>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/12/11 18:03:31 by xenobas           #+#    #+#              #
#    Updated: 2025/12/20 16:23:49 by aindjare         ###   ########.fr        #
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

File_2MiB.data:
	dd if=/dev/urandom of=File_2MiB.data bs=1M count=2 status=progress
File_8MiB.data:
	dd if=/dev/urandom of=File_8MiB.data bs=1M count=8 status=progress
File_16MiB.data:
	dd if=/dev/urandom of=File_16MiB.data bs=1M count=16 status=progress

data: File_2MiB.data File_8MiB.data File_16MiB.data

$(NAME): $(INCS) $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@	$(OBJS)

-include $(DEPS)

.PHONY: all clean fclean re
.SECONDARY: $(OBJS) $(DEPS)
