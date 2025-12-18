/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 15:45:19 by aindjare          #+#    #+#             */
/*   Updated: 2025/12/18 13:22:53 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include <cstdlib>
#include <ctime>

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

	#if 0 /* TEST(xenobas): Route picker */
	{
		const char*				paths[] = {
			"/",
			"/index.html",
			"/images/1337.jpeg",
			"/images/1337.jpeg",
			"/upload",
			"/upload",
			"/assets/images/1337", /* Redirect */
			"/assets/images/1337.jpeg", /* Server */
		};
		WEBSERV_Method			methods[] = {
			WEBSERV_METHOD_GET,
			WEBSERV_METHOD_GET,
			WEBSERV_METHOD_GET,
			WEBSERV_METHOD_POST,
			WEBSERV_METHOD_GET,
			WEBSERV_METHOD_POST,
			WEBSERV_METHOD_GET,
			WEBSERV_METHOD_HEAD,
		};

		const WEBSERV_Instance&	instance = config.instances[0];
		for (i32 i = 0; i < cast(i32)count_of(paths); ++i) {
			const string_view	key = WEBSERV_http_route_pick(instance, paths[i]);
			printf("path \"%-24s\" ", paths[i]);
			if (!key) {
				printf("key not found\n");
			} else {
				printf("key \"%.*s\" was found ", key.len, key.text);

				const WEBSERV_Route&	route = instance.routes.get(key);
				b32						method_ok = WEBSERV_http_route_method_test(route, methods[i]);
				printf("method is %s\n", method_ok ? "allowed" : "forbidden");
			}
		}
	}
	#endif

	b32	ok_run = WEBSERV_context_run(config);
	CLI_debug("WEBSERV_context_run has terminated");

	WEBSERV_config_delete(config);
	return (ok_run ? 0 : 16);
}
