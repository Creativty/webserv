/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 16:01:16 by aindjare          #+#    #+#             */
/*   Updated: 2025/07/15 17:29:20 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef   webserv_HPP
#define   webserv_HPP
#include  <vector>
#include  <string>
#include  <ostream>
#include  <cstdlib>
#define   cast
#define   nullptr NULL

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
#endif // webserv_HPP
