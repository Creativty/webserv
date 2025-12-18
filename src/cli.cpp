/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cli.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 10:16:13 by aindjare          #+#    #+#             */
/*   Updated: 2025/12/18 13:20:19 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

static const char*	webserv_config_error_kind_strings[] = {
#define CONFIG_ERROR_KIND(NAME, STRING, ...) STRING,
	CONFIG_ERROR_KINDS
#undef CONFIG_ERROR_KIND
};

static const char*	toml_value_kind_strings[] = {
#define TOML_VALUE_KIND(NAME, STRING, ...) STRING,
	TOML_VALUE_KINDS
#undef TOML_VALUE_KIND
};

b32				CLI_is_tty = 0;
string_view		CLI_exec_path = "<CLI_exec_path>";

void			CLI_debug_internal(const char* file, i32 line, const char *label, const char *fmt, ...) {
#ifdef DEBUG
	va_list	args;
	va_start(args, fmt);
	fprintf(stdout, "%s:%d: %s: ", file, line, label);
	vfprintf(stdout, fmt, args);
	fprintf(stdout, "\n");
	va_end(args);
#else
	unused(fmt);
	unused(label);
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
	if (CLI_is_tty) {
		fprintf(stderr, "    " TERMINAL_COLOR_WHITE "%s" TERMINAL_STYLE_RESET": ", prefix);
	} else {
		fprintf(stderr, "    %s: ", prefix);
	}
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);
}

static void		CLI_show_error_file(const char* fmt, ...) {
	va_list	args;
	va_start(args, fmt);
	if (CLI_is_tty) {
		fprintf(stderr, "%.*s: " TERMINAL_COLOR_RED "File Error" TERMINAL_STYLE_RESET ": ", CLI_exec_path.len, CLI_exec_path.text);
	} else {
		fprintf(stderr, "%.*s: File Error: ", CLI_exec_path.len, CLI_exec_path.text);
	}
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);
}
void			CLI_show_error_file_ext(string_view file_path) {
	CLI_show_error_file("Input file name \"%.*s\" does not end in \".toml\"", file_path.len, file_path.text);
}
void			CLI_show_error_file_access(string_view file_path) {
	CLI_show_error_file("Could not access file at \"%.*s\"", file_path.len, file_path.text);
	CLI_show_extra("Reason", "%m");
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
	if (CLI_is_tty) {
		fprintf(stderr, "%.*s:%d:%d: " TERMINAL_COLOR_RED "Syntax Error" TERMINAL_STYLE_RESET ": ", pos.file.len, pos.file.text, pos.col, pos.row);
	} else {
		fprintf(stderr, "%.*s:%d:%d: Syntax Error: ", pos.file.len, pos.file.text, pos.col, pos.row);
	}
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);
}

static void		CLI_show_error_toml_line(const TOML_Document& document, const Position& pos) {
	if (!CLI_is_tty)
		return ;

	string_view	source = document.lexer.source;

	i32			line_begin = pos.index;
	while (line_begin > 0) {
		if (source[line_begin - 1] == '\n')
			break ;
		line_begin--;
	}

	i32			line_end = pos.index;
	while (source.len > line_end) {
		if (source[line_end] == '\n')
			break ;
		line_end++;
	}

	i32			col_space = 5;

	string_view	line = source.slice(line_begin, line_end);
	fprintf(stderr, "%*d |    ", col_space, pos.col);
	for (i32 i = 0; i < line.len; ++i) {
		if (line[i] == '\t') {
			fprintf(stderr, TERMINAL_COLOR_BLACK "\\>  " TERMINAL_STYLE_RESET);
		} else {
			fprintf(stderr, "%c", line[i]);
		}
	}
	fprintf(stderr, "\n");

	i32	cursor = line_begin;
	fprintf(stderr, "%*s |    ", col_space, "");
	while (cursor < pos.index) {
		fprintf(stderr, " ");
		if (source[cursor] == '\t') {
			fprintf(stderr, "   ");
		}
		cursor++;
	}
	fprintf(stderr, "^\n");
}
void			CLI_show_errors_toml(const TOML_Document& document) {
	const dynamic_array<TOML_Error>&	errors = document.errors;
	for (i32 i = 0; i < errors.len; ++i) {
		const TOML_Error&	err = errors[i];
		switch (err.kind) {
		case TOML_ERROR_LOAD_STRING: {
			string_view	file = document.file;
			CLI_show_error_file("Could not load file \"%.*s\" contents", file.len, file.text);
			CLI_show_extra("Reason", "%m");
		} break;
		case TOML_ERROR_TOKEN_NUMBER_UNDERSCORE: {
			CLI_show_error_syntax(err.pos, "Invalid underscore usage in number");
			CLI_show_error_toml_line(document, err.pos);
		} break;
		case TOML_ERROR_TOKEN_NUMBER_DIGITS: {
			CLI_show_error_syntax(err.pos, "Missing number after sign");
			CLI_show_error_toml_line(document, err.pos);
		} break;
		case TOML_ERROR_TOKEN_INVALID: {
			char	b = document.lexer.source[err.pos.index];
			CLI_show_error_syntax(err.pos, "Invalid byte '%c': 0x%02X", b, b);
			CLI_show_error_toml_line(document, err.pos);
		} break;
		case TOML_ERROR_TOKEN_UNTERMINATED: {
			CLI_show_error_syntax(err.pos, "Unterminated string");
			CLI_show_error_toml_line(document, err.pos);
		} break;
		case TOML_ERROR_PARSER_EXPECT: {
			string_view	token = err.token.str;
			if (err.token.kind == TOML_TOKEN_EOL) {
				token = "\\n";
			} else if (err.token.kind == TOML_TOKEN_EOF) {
				token = "<EOF>";
			} else if (err.token.kind == TOML_TOKEN_INVALID) {
				token = "<INVALID>";
			}
			CLI_show_error_syntax(err.pos, "Expected \"%.*s\", got \"%.*s\"", err.str.len, err.str.text, token.len, token.text);
			CLI_show_error_toml_line(document, err.pos);
		} break;
		case TOML_ERROR_PARSER_SCOPE_KEY_DUP: {
			string_view	token = err.token.str;
			if (err.token.kind == TOML_TOKEN_EOL) {
				token = "\\n";
			} else if (err.token.kind == TOML_TOKEN_EOF) {
				token = "<EOF>";
			} else if (err.token.kind == TOML_TOKEN_INVALID) {
				token = "<INVALID>";
			}
			CLI_show_error_syntax(err.pos, "Duplicated entry \"%.*s\"", err.str.len, err.str.text);
			CLI_show_error_toml_line(document, err.pos);
		} break;
		case TOML_ERROR_PARSER_NUMBER_CHAR: {
			CLI_show_error_syntax(err.pos, "Number can only contain digits [0-9]");
			CLI_show_error_toml_line(document, err.pos);
		} break;
		case TOML_ERROR_PARSER_NUMBER_RANGE: {
			string_view	str = err.token.str;
			CLI_show_error_syntax(err.pos, "Cannot fit \"%.*s\" inside an i64", str.len, str.text);
			CLI_show_error_toml_line(document, err.pos);
		} break;
		case TOML_ERROR_PARSER_STRING_QUOTES: {
			string_view	str = err.token.str;
			CLI_show_error_syntax(err.pos, "Invalid string literal `%.*s'", str.len, str.text);
			CLI_show_error_toml_line(document, err.pos);
		} break;
		case TOML_ERROR_PARSER_UNSUPPORTED: {
			string_view	feature = err.str;
			string_view	token = err.token.str;
			CLI_show_error_syntax(err.pos, "Unsupported TOML feature `%.*s'", feature.len, feature.text);
			CLI_show_error_toml_line(document, err.pos);
			if (feature == "[table]") {
				CLI_show_extra("Suggestion", "Use an inline table `%.*s = { ... }`", token.len, token.text);
			}
		} break;
		case TOML_ERROR_INVALID:
		default: {
			CLI_show_error_syntax(err.pos, "Unknown error %d", err.kind);
		} break;
		}
	}
}

void			CLI_show_error_config(const char* fmt, ...) {
	va_list	args;
	va_start(args, fmt);
	if (CLI_is_tty) {
		fprintf(stderr, TERMINAL_COLOR_RED "Config Error" TERMINAL_STYLE_RESET ": ");
	} else {
		fprintf(stderr, "Config Error: ");
	}
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);
}
void			CLI_show_error_config(const Position pos, const char* fmt, ...) {
	va_list	args;
	va_start(args, fmt);
	if (CLI_is_tty) {
		fprintf(stderr, "%.*s:%d:%d: " TERMINAL_COLOR_RED "Config Error" TERMINAL_STYLE_RESET ": ", pos.file.len, pos.file.text, pos.col, pos.row);
	} else {
		fprintf(stderr, "%.*s:%d:%d: Config Error: ", pos.file.len, pos.file.text, pos.col, pos.row);
	}
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);
}
void			CLI_show_errors_config(const WEBSERV_Config& config) {
	const dynamic_array<WEBSERV_Config_Error>&	errors = config.errors;
	for (i32 i = 0; i < errors.len; ++i) {
		const WEBSERV_Config_Error& err = errors[i];
		if (err.kind == WEBSERV_CONFIG_ERROR_ROOT_TYPE) {
			CLI_show_error_config(err.pos, "Document is not a table");
		} else if (err.kind == WEBSERV_CONFIG_ERROR_ROOT_DATA) {
			CLI_show_error_config(err.pos, "Document is empty");
		} else if (err.kind == WEBSERV_CONFIG_ERROR_KEY_UNKNOWN) {
			CLI_show_error_config(err.pos, "Unrecognized key \"%.*s\"", err.str.len, err.str.text);
			CLI_show_error_toml_line(config.document, err.pos);
		} else if (err.kind == WEBSERV_CONFIG_ERROR_KEY_DISALLOWED) {
			CLI_show_error_config(err.pos, "Disallowed key, because %.*s", err.str.len, err.str.text);
			CLI_show_error_toml_line(config.document, err.pos);
		} else if (err.kind == WEBSERV_CONFIG_ERROR_KEY_REQUIRED) {
			CLI_show_error_config(err.pos, "Missing required key \"%.*s\"", err.str.len, err.str.text);
		} else if (err.kind == WEBSERV_CONFIG_ERROR_TYPE_MISMATCH) {
			const char*	type_str = toml_value_kind_strings[err.value->kind];
			CLI_show_error_config(err.pos, "Expected value of type \"%.*s\", Got \"%s\"", err.str.len, err.str.text, type_str);
			CLI_show_error_toml_line(config.document, err.pos);
		} else if (err.kind == WEBSERV_CONFIG_ERROR_INSTANCE_EMPTY) {
			CLI_show_error_config(err.pos, "Empty instance");
			CLI_show_error_toml_line(config.document, err.pos);
		} else if (err.kind == WEBSERV_CONFIG_ERROR_PORT_RANGE) {
			CLI_show_error_config(err.pos, "port value out of range, must be between 0 and 65535");
			CLI_show_error_toml_line(config.document, err.pos);
		} else if (err.kind == WEBSERV_CONFIG_ERROR_REQUEST_BODY_LIMIT_RANGE) {
			CLI_show_error_config(err.pos, "request_body_limit value is out of range, must be between 0 and 65535");
			CLI_show_error_toml_line(config.document, err.pos);
		} else if (err.kind == WEBSERV_CONFIG_ERROR_STRING_EMPTY) {
			CLI_show_error_config(err.pos, "Expected \"%.*s\", got an empty string value", err.str.len, err.str.text);
			CLI_show_error_toml_line(config.document, err.pos);
		} else if (err.kind == WEBSERV_CONFIG_ERROR_FILE_INVALID) {
			CLI_show_error_config(err.pos, "File \"%.*s\" is inaccessible or does not end in \".html\"", err.str.len, err.str.text);
			CLI_show_error_toml_line(config.document, err.pos);
		} else if (err.kind == WEBSERV_CONFIG_ERROR_VALUE_INVALID) {
			CLI_show_error_config(err.pos, "Invalid value is not a valid \"%.*s\"", err.str.len, err.str.text);
			CLI_show_error_toml_line(config.document, err.pos);
		} else if (err.kind == WEBSERV_CONFIG_ERROR_ROUTE_PATH_DUP) {
			CLI_show_error_config(err.pos, "Route path \"%.*s\" is a duplicate", err.str.len, err.str.text);
			CLI_show_error_toml_line(config.document, err.pos);

			string_view	str = err.str;
			str.free(); /* NOTE(xenobas): This was allocated specifically for error reporting */
		} else if (err.kind == WEBSERV_CONFIG_ERROR_INTERPRETER_INVALID) {
			const string_view&	path = err.str;
			const char*			desc = "does not exist";
			if (OS_access_file(path, F_OK)) {
				desc = "is not executable";
			}
			CLI_show_error_config(err.pos, "Interpreter located at \"%.*s\" %s", err.str.len, err.str.text, desc);
			CLI_show_error_toml_line(config.document, err.pos);
		} else if (err.kind == WEBSERV_CONFIG_ERROR_ROUTE_TYPE) {
			const TOML_Value*	value = err.value;
			b32					value_segfault = (value != 0 && value->String != 0);

			CLI_show_error_config(err.pos, "Unknown route type \"%.*s\"", value_segfault ? value->String->len : 6, value ? value->String->text : "(null)");
			CLI_show_error_toml_line(config.document, err.pos);

			char	buff[512] = { 0 };
			u64		index = 0;
			index += cast(u64)std::snprintf(&buff[index], 512ul - index, "\"%s\", ", "server"); // SERVER
			index += cast(u64)std::snprintf(&buff[index], 512ul - index, "\"%s\", ", "redirect"); // REDIRECT
			index += cast(u64)std::snprintf(&buff[index], 512ul - index, "\"%s\", ", "upload"); // UPLOAD
			index += cast(u64)std::snprintf(&buff[index], 512ul - index, "\"%s\"", "cgi"); // CGI

			CLI_show_extra("Hint", "Must be one of the following options { %s }", buff);
		} else if (err.kind == WEBSERV_CONFIG_ERROR_DIR_RO) {
			const string_view&	str = err.str;
			CLI_show_error_config(err.pos, "Directory \"%.*s\" either doesn't exist or isn't allowed to be read", str.len, str.text);
			CLI_show_error_toml_line(config.document, err.pos);
		} else if (err.kind == WEBSERV_CONFIG_ERROR_DIR_RW) {
			const string_view&	str = err.str;

			CLI_show_error_config(err.pos, "Directory \"%.*s\" either doesn't exist, isn't allowed to be read, or cannot be written", str.len, str.text);
			CLI_show_error_toml_line(config.document, err.pos);
		} else {
			const char*	error_str = webserv_config_error_kind_strings[err.kind];
			CLI_show_error_config(err.pos, "TODO: Error \"%s\" reporting is unimplemented", error_str);
		}
	}
}

void			CLI_show_error_runtime(const char* fmt, ...) {
	va_list	args;
	va_start(args, fmt);
	if (CLI_is_tty) {
		fprintf(stderr, TERMINAL_COLOR_RED "Runtime Error" TERMINAL_STYLE_RESET ": ");
	} else {
		fprintf(stderr, "Runtime Error: ");
	}
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);
}

void			CLI_flush(void) {
	::fflush(stdout);
	::fflush(stderr);
}
