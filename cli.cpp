/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cli.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 10:16:13 by aindjare          #+#    #+#             */
/*   Updated: 2025/10/27 14:01:05 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include <cstdarg>

string_view		CLI_exec_path = "<CLI_exec_path>";

void			CLI_debug_internal(const char* file, i32 line, const char *fmt, ...) {
#ifdef WEBSERV_DEBUG
	va_list	args;
	va_start(args, fmt);
	fprintf(stdout, "%s:%d: Debug: ", file, line);
	vfprintf(stdout, fmt, args);
	fprintf(stdout, "\n");
	va_end(args);
#else
	unused(fmt);
	unused(file);
	unused(line);
#endif
}

void			CLI_show_help(FILE* stream) {
	fprintf(stream, "%.*s: 42 Webserv implementation\n", CLI_exec_path.len, CLI_exec_path.text);
	fprintf(stream, "Usage:\n");
	fprintf(stream, "    %.*s <config_file>\n", CLI_exec_path.len, CLI_exec_path.text);
	fprintf(stream, "Parameters:\n");
	fprintf(stream, "    --help        Displays this help message\n");
	fprintf(stream, "    <config_toml> Input file in TOML syntax for describing server instances\n");
}
void			CLI_show_extra(const char* prefix, const char* fmt, ...) {
	va_list	args;
	va_start(args, fmt);
	fprintf(stderr, "    " TERMINAL_COLOR_WHITE "%s" TERMINAL_STYLE_RESET": ", prefix);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);
}

static void		CLI_show_error_file(const char* fmt, ...) {
	va_list	args;
	va_start(args, fmt);
	fprintf(stderr, "%.*s: " TERMINAL_COLOR_RED "File Error" TERMINAL_STYLE_RESET ": ", CLI_exec_path.len, CLI_exec_path.text);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);
}
void			CLI_show_error_file_ext(string_view file_path) {
	CLI_show_error_file("Input file name \"%.*s\" does not end in \".toml\"", file_path.len, file_path.text);
	// fprintf(stderr, "%.*s: CLI Error: Input file name \"%.*s\" must end in \".toml\"\n", CLI_exec_path.len, CLI_exec_path.text, file_path.len, file_path.text);
}
void			CLI_show_error_file_access(string_view file_path) {
	CLI_show_error_file("Could not access file at \"%.*s\"", file_path.len, file_path.text);
	CLI_show_extra("Description", "%m");
}
void			CLI_show_error_file_mode(string_view file_path, mode_t mode) {
	CLI_show_error_file("Input file path \"%.*s\" is not a regular file", file_path.len, file_path.text);
	if (S_ISDIR(mode))
		CLI_show_extra("Description", "\"%.*s\" is a directory", file_path.len, file_path.text);
	else if (S_ISLNK(mode))
		CLI_show_extra("Description", "\"%.*s\" is a symbolic link", file_path.len, file_path.text);
	else if (S_ISFIFO(mode))
		CLI_show_extra("Description", "\"%.*s\" is a pipe/FIFO special file", file_path.len, file_path.text);
	else if (S_ISSOCK(mode))
		CLI_show_extra("Description", "\"%.*s\" is a socket", file_path.len, file_path.text);
}

void			CLI_show_error_syntax(const Position pos, const char* fmt, ...) {
	va_list	args;
	va_start(args, fmt);
	fprintf(stderr, "%.*s:%d:%d: " TERMINAL_COLOR_RED "Syntax Error" TERMINAL_STYLE_RESET ": ", pos.file.len, pos.file.text, pos.col, pos.row);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);
}
void			CLI_show_error_config(const Position pos, const char* fmt, ...) {
	va_list	args;
	va_start(args, fmt);
	fprintf(stderr, "%.*s:%d:%d: " TERMINAL_COLOR_RED "Config Error" TERMINAL_STYLE_RESET ": ", pos.file.len, pos.file.text, pos.col, pos.row);
	vfprintf(stderr, fmt, args);
	va_end(args);
}

void			CLI_show_error_toml_line(const TOML_Document& document, const Position& pos) {
	string_view	source = document.lexer.source;

	i32			line_begin = pos.index;
	while (line_begin > 0) {
		if (source[line_begin - 1] == '\n')
			break ;
		line_begin--;
	}

	i32			line_end = pos.index;
	while (source.len > line_end + 1) {
		if (source[line_end + 1] == '\n')
			break ;
		line_end++;
	}

	i32			col_space = 5;

	string_view	line = source.slice(line_begin, line_end);
	fprintf(stderr, "%*d |    %.*s\n", col_space, pos.col, line.len, line.text);

	i32	cursor = line_begin - 4;
	fprintf(stderr, "%*s |", col_space, "");
	while (cursor < pos.index) {
		fprintf(stderr, " ");
		cursor++;
	}
	fprintf(stderr, "^\n");
}

void			CLI_show_error_toml_parse(const TOML_Document& document) {
	const dynamic_array<TOML_Error>&	errors = document.errors;
	for (i32 i = 0; i < errors.len; ++i) {
		const TOML_Error&	err = errors[i];
		switch (err.kind) {
		case TOML_ERROR_LOAD_BYTES: {
			string_view	file = document.file;
			CLI_show_error_file("Could not load bytes from \"%.*s\"", file.len, file.text);
		} break;
		case TOML_ERROR_TOKEN_UNTERMINATED: {
			CLI_show_error_syntax(err.pos, "Unterminated string");
			CLI_show_error_toml_line(document, err.pos);
		} break;
		case TOML_ERROR_INVALID:
		default: {
			CLI_show_error_syntax(err.pos, "Unknown error %d", err.kind);
		} break;
		}
	}
}
