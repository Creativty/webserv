/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 17:00:44 by aindjare          #+#    #+#             */
/*   Updated: 2025/09/16 17:20:07 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "types.hpp"
#include "toml.hpp"
#include "terminal.hpp"

namespace file {
	bool	read(const char* in_path, std::string& out_string) {
		std::ifstream		in_stream(in_path);
		std::stringstream	out_stream;

		if (in_stream.fail())
			return (false);

		while (in_stream >> out_stream.rdbuf());
		out_string = out_stream.str();
		return (true);
	}
};

i32	main(i32 argc, type::cstring *argv) {
	if (argc != 2)
		return (1);

	std::string			config_string;
	if (!file::read(argv[1], config_string))
		return (2);

	toml::token_array	config_tokens;
	toml::value			config_scope(new toml::value_table);
	type::string		config_source = config_string.c_str();
	if (!toml::process(config_source, config_tokens, config_scope.data.table)) {
		std::cerr << TERMINAL_NOTICE_ERROR ": an error occurred while parsing " << config_source << std::endl;
		return (config_scope.free(), config_tokens.free(), 4);
	}

	std::cout << config_scope << std::endl;
	return (config_scope.free(), config_tokens.free(), 0);
}
