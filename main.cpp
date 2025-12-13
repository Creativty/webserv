/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sennakhl <sennakhl@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 15:45:19 by aindjare          #+#    #+#             */
/*   Updated: 2025/12/02 09:43:14 by sennakhl         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include <cstdlib>
#include <ctime>

#include <iostream>



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
		CLI_show_errors_toml(document);

		TOML_delete(document);
		return (4);
	}

	WEBSERV_Config	config = WEBSERV_config_parse(document);
	if (!config.ok) {
		CLI_show_errors_config(config);

		WEBSERV_config_delete(config);
		return (8);
	}

	if (false)
	{
		const WEBSERV_Instance&	instance = config.instances[0];
		const char				*paths[] = {
			// "/",
			// "/index.html",
			// "/images/1337.jpeg",
			"/upload/Hyello.jpeg",
			// "/assets/images/1337", /* Redirect */
			// "/assets/images/1337.jpeg", /* Server */
			// "/cgi/script.py",
		};

		for (i32 i = 0; i < cast(i32)count_of(paths); ++i) {
			const string_view	key = WEBSERV_http_route_pick(instance, paths[i]);
			WEBSERV_Route route = instance.routes.get(key);

			// string_view path = string_view(paths[i]);

			std::cout << "path = " << string_view(paths[i]) << std::endl;
			std::cout << "\troute_key = " << key << std::endl;
			std::cout << "\troute_path = " << route.path << std::endl;
			std::cout << "\troute_upload = " << route.Upload.directory << std::endl;

			// const std::string p = paths[i];

			// std::string typ = " " + getContentType(p);
			// std::cout << typ << std::endl;
			// std::string ext = getFileExtension(typ);
			// std::cout << ext << std::endl;

			// ext = ext.substr(1);

			// std::cout << route.CGI.interpreters.get(string_view(ext.c_str())) << std::endl;

			// std::cout << std::endl;
		}
	}

	server(config);
	

	WEBSERV_config_delete(config);
	return (0);
}
