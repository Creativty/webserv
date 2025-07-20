/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sennakhl <sennakhl@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 16:03:49 by aindjare          #+#    #+#             */
/*   Updated: 2025/07/20 09:39:58 by sennakhl         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fstream>
#include <sstream>
#include <iostream>
#include "webserv.hpp"

std::string	read_entire_file(const std::string& path, bool *ok) {
	if (ok != nullptr)
		*ok = false;
	std::ifstream		file(path.c_str());
	if (file.fail())
		return ("");

	std::ostringstream	stream;
	file >> stream.rdbuf();
	if (file.fail() && !file.eof())
		return ("");
	if (ok != nullptr)
		*ok = true;
	return (stream.str());
}

int	main(void) {
	bool		source_ok;
	std::string	source = read_entire_file("config.toml", &source_ok);
	if (!source_ok) {
		std::cerr << "FATAL: could not read configuration off of config.toml" << std::endl;
		return (1);
	}

	toml::Tokens	tokens = toml::lex(source.c_str());
	// for (size_t i = 0; i < tokens.elems.size(); i++)
	// 	std::cout << tokens.elems[i] << std::endl;
	toml::Configs	configs = toml::parse(tokens);
	(void)configs;
	server();
	return (0);
}
