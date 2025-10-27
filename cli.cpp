/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cli.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 10:16:13 by aindjare          #+#    #+#             */
/*   Updated: 2025/10/27 10:43:45 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

string_view	CLI_exec_path = "<CLI_exec_path>";

void		CLI_show_error_file_ext(string_view file_path) {
	fprintf(stderr, "%.*s: CLI Error: Input file name \"%.*s\" must end in \".toml\"\n", CLI_exec_path.len, CLI_exec_path.text, file_path.len, file_path.text);
}
void		CLI_show_error_file_stat(string_view file_path) {
	fprintf(stderr, "%.*s: CLI Error: Could not stat/access file at \"%.*s\"\n", CLI_exec_path.len, CLI_exec_path.text, file_path.len, file_path.text);
	fprintf(stderr, "    Description: %m\n");
}
void		CLI_show_error_file_mode(string_view file_path, mode_t mode) {
	fprintf(stderr, "%.*s: CLI Error: Input file path \"%.*s\" is not a regular file\n", CLI_exec_path.len, CLI_exec_path.text, file_path.len, file_path.text);
	if (S_ISDIR(mode))
		fprintf(stderr, "    Description: \"%.*s\" is a directory\n", file_path.len, file_path.text);
	else if (S_ISLNK(mode))
		fprintf(stderr, "    Description: \"%.*s\" is a symbolic link\n", file_path.len, file_path.text);
	else if (S_ISFIFO(mode))
		fprintf(stderr, "    Description: \"%.*s\" is a pipe/FIFO special file\n", file_path.len, file_path.text);
	else if (S_ISSOCK(mode))
		fprintf(stderr, "    Description: \"%.*s\" is a socket\n", file_path.len, file_path.text);
}
void		CLI_show_help(FILE* stream) {
	fprintf(stream, "%.*s: 42 Webserv implementation\n", CLI_exec_path.len, CLI_exec_path.text);
	fprintf(stream, "Usage:\n");
	fprintf(stream, "    %.*s <config_file>\n", CLI_exec_path.len, CLI_exec_path.text);
	fprintf(stream, "Parameters:\n");
	fprintf(stream, "    --help        Displays this help message\n");
	fprintf(stream, "    <config.toml> Input file using TOML syntax for describing server instances\n");
}
