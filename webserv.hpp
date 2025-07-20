/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sennakhl <sennakhl@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 16:01:16 by aindjare          #+#    #+#             */
/*   Updated: 2025/07/20 10:55:18 by sennakhl         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef   webserv_HPP
#define   webserv_HPP
#include  <vector>
// #include  <string>
// #include  <ostream>
#include  <cstdlib>
#include <map>
#include <string>
#include <ostream>
#include <iostream>
#define   cast
#define   nullptr NULL

enum HTTP_Method {
	HTTP_METHOD_INVALID,
	HTTP_METHOD_GET,
	HTTP_METHOD_POST,
	HTTP_METHOD_HEAD,
	HTTP_METHOD_DELETE,
};

struct Request {
	HTTP_Method							method;
	std::string							uri;
	std::string							version;

	std::map<std::string, std::string>	headers;
	std::string							body;
};

struct Token {
	int			line, column;
	std::string	text;
};

enum Parse_Error {
	PARSE_ERROR_NONE,
	PARSE_ERROR_NOT_DELIMITED,
	PARSE_ERROR_SUCCESSIVE_SPACE,
	PARSE_ERROR_NOT_ENOUGH_FIELDS,
	PARSE_ERROR_HEADER_MISSING_KEY,
	PARSE_ERROR_UNSUPPORTED_METHOD,
	PARSE_ERROR_UNSUPPORTED_VERSION,
	PARSE_ERROR_HEADER_ENTRY_INVALID_VALUE,
	PARSE_ERROR_HEADER_ENTRY_DELIMITER,
	PARSE_ERROR_HEADER_ENTRY_INCOMPLETE,
	PARSE_ERROR_HEADER_SECTION_DELIMITER,
};

namespace toml {
	enum Token_Kind {
		TOKEN_INVALID,
		TOKEN_COMMENT,
		TOKEN_STRING,
		TOKEN_NUMBER,
		TOKEN_IDENTIFIER,
		TOKEN_EOL,
		TOKEN_SEPARATOR,
		TOKEN_ASSIGN,
		TOKEN_DICT_OPEN,
		TOKEN_DICT_CLOSE,
		TOKEN_TABLE_OPEN,
		TOKEN_TABLE_CLOSE,
		TOKEN_EOF,
	};
	struct Token {
		std::string	text;
		Token_Kind	kind;
		int			line, column;

		Token();
		Token(const char *text, int len, Token_Kind kind, int line, int column);
	};

	enum Token_Error_Kind {
		UNTERMINATED,
	};
	struct Token_Error {
		Token				token;
		Token_Error_Kind	kind;
	};

	struct Tokens {
		std::vector<Token>			elems;
		std::vector<Token_Error>	errors;
	};

	struct Config {
	};
	typedef std::vector<Config>	Configs;

	Tokens	lex(const char *source);
	Configs	parse(Tokens& tokens);
};

std::ostream&	operator<<(std::ostream& stream, const toml::Token& token);
std::ostream&	operator<<(std::ostream& stream, const toml::Token_Kind& kind);

int server();
Parse_Error	parse_request_headers(Request& request, const std::string& msg);
#endif // webserv_HPP
