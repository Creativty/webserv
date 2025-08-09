#!/usr/bin/env bash
set -xe

c++ -Wall -Wextra -Werror -Wconversion -Wswitch-enum -std=c++98 -g string_view.cpp toml.cpp main.cpp -o bin
./bin config.toml
