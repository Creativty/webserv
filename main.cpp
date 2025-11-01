/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 15:45:19 by aindjare          #+#    #+#             */
/*   Updated: 2025/11/01 15:29:50 by xenobas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

i32	main(i32 argc, cstring argv[]) {
	CLI_exec_path = argv[0];
	CLI_is_tty = isatty(STDOUT_FILENO);

	if (argc != 2 || string_view("--help") == argv[1]) {
		CLI_show_help(argc == 2 ? stderr : stdout);
		return (argc == 2 ? 0 : 1);
	}

	string_view	path_toml = string_view(argv[1]);
	if (!path_toml.has_suffix(".toml")) {
		CLI_show_error_file_ext(path_toml);
		return (2);
	}

	struct stat	stat_toml;
	if (!OS_stat_file(path_toml, &stat_toml) || !OS_access_file(path_toml, R_OK)) {
		CLI_show_error_file_access(path_toml);
		return (2);
	} else if (!(S_ISREG(stat_toml.st_mode))) {
		CLI_show_error_file_mode(path_toml, stat_toml.st_mode);
		return (2);
	}

	TOML_Document	document = TOML_parse_file(path_toml);
	if (!document.ok) {
		CLI_show_errors_toml_parse(document);

		TOML_delete(document);
		return (4);
	}

	WEBSERV_URI	uri = WEBSERV_uri_decode("/Hello%20World?test&good&test=123&question=Are%20You%Bork%20Gay%3f#");
	WEBSERV_uri_delete(uri);

	TOML_delete(document);
	return (0);
}
