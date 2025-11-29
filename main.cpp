/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 15:45:19 by aindjare          #+#    #+#             */
/*   Updated: 2025/11/29 11:00:25 by xenobas          ###   ########.fr       */
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

	#if 0
	{
		const WEBSERV_Instance&	instance = config.instances[0];
		const char				*paths[] = {
			"/",
			"/index.html",
			"/images/1337.jpeg",
			"/upload/Hyello.jpeg",
			"/assets/images/1337", /* Redirect */
			"/assets/images/1337.jpeg", /* Server */
		};

		for (i32 i = 0; i < cast(i32)count_of(paths); ++i) {
			const string_view	key = WEBSERV_http_route_pick(instance, paths[i]);
			std::cout << "path = " << string_view(paths[i]) << std::endl;
			std::cout << "\troute_key = " << key << std::endl;
			std::cout << std::endl;
		}
	}
	#endif

	WEBSERV_config_delete(config);
	return (0);
}
