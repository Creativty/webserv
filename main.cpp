/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 15:45:19 by aindjare          #+#    #+#             */
/*   Updated: 2025/11/03 14:26:25 by xenobas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include <cstdlib>
#include <ctime>

i32	TEST_http(void) {
	std::srand((unsigned int)std::time(0));

	const string_view	string =
		"GET / HTTP/1.1\r\n" // 16 bytes
		"Content-Length: 0\r\n" // 19 bytes
		"\r\n"; // 2 bytes
		// 37 bytes
	HTTP_Request	req = HTTP_request_make();

	for (i32 cursor = 0; cursor < string.len; ) {
		i32		read_len = std::rand() % (string.len - cursor);
		if (read_len == 0) {
			read_len = 1;
		}
		byte*	read_bytes = cast(byte*)&string.text[cursor];

		// printf("%-4d bytes: \"%.*s\"\n", read_len, read_len, (char*)read_bytes);
		HTTP_request_read(req, read_bytes, read_len);
		cursor += read_len;
	}

	if (HTTP_request_is_error(req)) {
		fprintf(stderr, "An error has occurred during read.\n");
	} else if (!HTTP_request_is_closed(req)) {
		fprintf(stderr, "Request did not close even after all bytes were read.\n");
	}

	HTTP_request_delete(req);
	return (0);
}
i32	TEST_uri(void) {
	WEBSERV_URI	uri = WEBSERV_uri_decode("/Hello%20World/123?test&good&test=123&question=Are%20You%Bork%20Gay%3f#");
	string_view	str = WEBSERV_uri_encode(uri);
	printf("String: \"%.*s\"\n", str.len, str.text);
	str.free();
	WEBSERV_uri_delete(uri);
	return (0);
}

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


	TEST_http();
#if 0
	TEST_uri();
#endif

	TOML_delete(document);
	return (0);
}
