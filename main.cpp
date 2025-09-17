/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 17:00:44 by aindjare          #+#    #+#             */
/*   Updated: 2025/09/17 16:17:07 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <cerrno>
#include <cstring>

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

namespace webserv {
	enum config_kind {
		CONFIG_VARIANT_INVALID,

		CONFIG_VARIANT_CGI,
	};
	struct config_cgi {
		type::string						path;
		type::string						extension;
		type::string						interpreter;
	};
	struct config {
		u16									port;
		i64									max_num_client;
		type::dynamic_array<type::string>	hosts;

		config_kind							_kind;
		union {
			config_cgi*						cgi;
		}									_variant;
	};
};

bool	load_config_file(type::cstring cmdname, type::cstring confpath, type::dynamic_array<webserv::config>& configs) {
	std::string		confstr;
	if (!file::read(confpath, confstr)) {
		std::cerr << TERMINAL_COLOR_WHITE << cmdname << ": " TERMINAL_NOTICE_ERROR TERMINAL_STYLE_RESET ": Failed while reading file `" << confpath << "`" << std::endl;
		std::cerr << "\tReason: " << strerror(errno) << std::endl;
		return (false);
	}

	toml::token_array			tokens;
	type::string				source = confstr.c_str();
	toml::value					root(new toml::value_table);
	if (!toml::process(source, tokens, root.data.table))
		return (root.free(), tokens.free(), false);
	if (root.data.table->count == 0u) {
		std::cerr << TERMINAL_COLOR_WHITE << cmdname << ": " TERMINAL_NOTICE_ERROR TERMINAL_STYLE_RESET ": Disallowed empty config file `" << confpath << "`" << std::endl;
		return (root.free(), tokens.free(), false);
	}

	(void)configs; /* TODO(xenobas): parse configs */
	toml::value_table::iterator	root_iter(*root.data.table);
	bool						parse_ok = true;
	while (root_iter.next()) {
		toml::string&		key = root_iter.item->key;
		toml::value&		val = root_iter.item->value;
		if (key != "webserv") {
			std::cerr << TERMINAL_COLOR_WHITE << cmdname << ": " TERMINAL_NOTICE_ERROR TERMINAL_STYLE_RESET ": Disallowed key " << key << " usage in `" << confpath << "` global scope" << std::endl;
			std::cerr << "\tSuggestion: Only [[webserv]] entries are allowed at the global scope" << std::endl;
			parse_ok = false;
			continue ;
		}
		if (val.kind != toml::VALUE_ARRAY) {
			std::cerr << TERMINAL_COLOR_WHITE << cmdname << ": " TERMINAL_NOTICE_ERROR TERMINAL_STYLE_RESET ": Disallowed usage of `webserv` key in a `webserv = <value>` statement" << std::endl;
			std::cerr << "\tSuggestion: Only [[webserv]] entries are allowed at the global scope" << std::endl;
			parse_ok = false;
			continue ;
		}
		toml::value_array&	arr = *val.data.array;
		for (u64 i = 0ul; i < arr.len; ++i) {
			toml::value&	val = arr[i];
			if (val.kind != toml::VALUE_TABLE) {
				std::cerr << TERMINAL_COLOR_WHITE << cmdname << ": " TERMINAL_NOTICE_ERROR TERMINAL_STYLE_RESET ": Disallowed usage of `webserv` key in a `webserv = <value>` statement" << std::endl;
				std::cerr << "\tSuggestion: Only [[webserv]] entries are allowed at the global scope" << std::endl;
				parse_ok = false;
				break ;
			}
			if (val.data.table->count == 0u) {
				std::cerr << TERMINAL_COLOR_WHITE << cmdname << ": " TERMINAL_NOTICE_ERROR TERMINAL_STYLE_RESET ": Forbidden empty webserv entry" << std::endl;
				parse_ok = false;
				continue ;
			}
		}
		if (parse_ok == false) continue ;
	}
	return (root.free(), tokens.free(), parse_ok);
}

i32	main(i32 argc, type::cstring *argv) {
	if (argc != 2) {
		std::cerr << TERMINAL_COLOR_WHITE << argv[0] << ": " TERMINAL_NOTICE_ERROR TERMINAL_STYLE_RESET ": Incorrect number of arguments passed" << std::endl;
		std::cerr << "\tUsage: " << argv[0] << " <FILE>" << std::endl;
		return (1);
	}

	type::dynamic_array<webserv::config>	configs;
	if (!load_config_file(argv[0], argv[1], configs)) {
		std::cerr << TERMINAL_COLOR_WHITE << argv[0] << ": " TERMINAL_NOTICE_ERROR TERMINAL_STYLE_RESET ": Could not load the provided configuration file `" << argv[1] << "`" << std::endl;
		return (2);
	}

	return (0);
}
