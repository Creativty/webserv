/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 15:46:57 by aindjare          #+#    #+#             */
/*   Updated: 2025/10/27 10:43:18 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP
#include "base.hpp"
#include "string_view.hpp"
#include "dynamic_array.hpp"
#include "hash_table.hpp"

#include <cstdio>
#include <cstring>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

struct Position {
	i32			index;
	i32			row;
	i32			col;
	string_view	file;
};

enum TOML_Token_Kind {
	TOML_TOKEN_INVALID,

	TOML_TOKEN_EQUALS, // =
	TOML_TOKEN_COMMA, // ,

	TOML_TOKEN_OSQUARE, // [
	TOML_TOKEN_CSQUARE, // ]

	TOML_TOKEN_OSQUARE_2X, // [[
	TOML_TOKEN_CSQUARE_2X, // ]]

	TOML_TOKEN_OCURLY, // {
	TOML_TOKEN_CCURLY, // }

	TOML_TOKEN_IDENT, // [a-zA-Z_]+[a-zA-Z0-9_]*

	TOML_TOKEN_TRUE, // true
	TOML_TOKEN_FALSE, // false

	TOML_TOKEN_STRING, // ".*"
	TOML_TOKEN_NUMBER, // [0-9]+
};

struct TOML_Token {
	TOML_Token_Kind	kind;
	string_view		str;
	Position		pos;
};

struct TOML_Document {
	dynamic_array<TOML_Token>	tokens;
	// dynamic_array<TOML_Error>	error;
};

enum WEBSERV_Method {
	WEBSERV_METHOD_INVALID,

	WEBSERV_METHOD_GET,
	WEBSERV_METHOD_POST,
	WEBSERV_METHOD_PUT,

	WEBSERV_METHOD_COUNT,
};

enum WEBSERV_Route_Kind {
	WEBSERV_ROUTE_INVALID,

	WEBSERV_ROUTE_BASIC,
	WEBSERV_ROUTE_REDIRECT,
	WEBSERV_ROUTE_UPLOAD,
	WEBSERV_ROUTE_CGI,
};

struct WEBSERV_Interface {
	u16	port;
	u32	address;
};

struct WEBSERV_Route_Basic {
	b32	directory_list;
};

struct WEBSERV_Route_Redirect {
	string_view	location;
};

struct WEBSERV_Route_Upload {
};

struct WEBSERV_Route_CGI {
	hash_table<string_view>		env;
};

struct WEBSERV_Route {
	string_view			pattern;
	WEBSERV_Method		methods_whitelist;

	WEBSERV_Route_Kind	kind;

	/* NOTE(xenobas):
	 * C++ doesn't allow non trivial constructor in union types
	 * So we do this abomination instead
	 */
	WEBSERV_Route_Basic		basic;
	WEBSERV_Route_Redirect	redirect;
	WEBSERV_Route_Upload	upload;
	WEBSERV_Route_CGI		cgi;
};

struct WEBSERV_Config {
	dynamic_array<WEBSERV_Interface>	interfaces;

	u32									request_body_max;

	string_view							error_4xx;
	string_view							error_5xx;

	hash_table<WEBSERV_Route>			routes;
};

typedef dynamic_array<WEBSERV_Config>	WEBSERV_Config_Array;

extern string_view		CLI_exec_path;

b32						OS_read_file(const string_view path, dynamic_array<byte>& out_data);
b32						OS_stat_file(const string_view& path, struct stat* buf = 0);
b32						OS_access_file(const string_view& _path, i32 flags = F_OK);

void					CLI_show_error_file_ext(string_view file_path);
void					CLI_show_error_file_stat(string_view file_path);
void					CLI_show_error_file_mode(string_view file_path, mode_t mode);
void					CLI_show_help(FILE* stream);

TOML_Document			TOML_parse_file(const string_view path);
WEBSERV_Config_Array	WEBSERV_parse_document(const TOML_Document document);
#endif
