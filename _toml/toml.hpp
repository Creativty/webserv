/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   toml.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xenobas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 21:53:06 by xenobas           #+#    #+#             */
/*   Updated: 2025/08/10 18:48:59 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TOML_HPP
#define TOML_HPP
#include <map>
#include <deque>
#include <string>
#include <vector>
#include <ostream>
#include "core.hpp"
#include "string_view.hpp"

struct TOML_Scanner {
	string_view&					source;
	u64								line;
	u64								offset;
	u64								current;

	TOML_Scanner(string_view& source);
	bool		done(void) const;
	string_view	lexeme(void) const;

	char		next(void);
	char		peek(void);
};

enum	TOML_Token_Tag {
	TOML_TOKEN_INVALID,

	TOML_TOKEN_EQUALS,
	TOML_TOKEN_COMMA,
	TOML_TOKEN_NEWLINE,
	TOML_TOKEN_SQUARE_OPEN,
	TOML_TOKEN_SQUARE_CLOSE,
	TOML_TOKEN_CURLY_OPEN,
	TOML_TOKEN_CURLY_CLOSE,
	TOML_TOKEN_EOF,

	TOML_TOKEN_IDENT,
	TOML_TOKEN_STRING,
	TOML_TOKEN_INTEGER,
	TOML_TOKEN_FLOAT,
	TOML_TOKEN_COMMENT,
	TOML_TOKEN_TRUE,
	TOML_TOKEN_FALSE,
};

enum	TOML_Token_Error {
	TOML_TOKEN_ERROR_NONE,
	TOML_TOKEN_ERROR_UNKNOWN,
	TOML_TOKEN_ERROR_NUMBER_MISSING,
	TOML_TOKEN_ERROR_UNTERMINATED,
};

std::ostream&	operator<<(std::ostream& stream, TOML_Token_Tag tag);
std::ostream&	operator<<(std::ostream& stream, TOML_Token_Error error);

struct TOML_Token {
	TOML_Token_Tag		tag;
	TOML_Token_Error	error;
	string_view			lexeme;
	u64					line;

	TOML_Token(void);
	TOML_Token(TOML_Token_Tag tag, const string_view& lexeme, u64 line, TOML_Token_Error error);
};

typedef std::deque<TOML_Token>	TOML_Tokens;

TOML_Tokens	toml_scan(string_view& source, string_view& path);

struct TOML_Value;

typedef	bool								TOML_Boolean;
typedef i64									TOML_Integer;
typedef f64									TOML_Float;
typedef std::string							TOML_String;
typedef std::deque<TOML_Value>				TOML_List;
typedef std::map<string_view, TOML_Value>	TOML_Table;

enum	TOML_Tag {
	TOML_TAG_INVALID,
	TOML_TAG_BOOLEAN,
	TOML_TAG_INTEGER,
	TOML_TAG_FLOAT,
	TOML_TAG_STRING,
	TOML_TAG_LIST,
	TOML_TAG_TABLE,
};

union	TOML_Data {
	TOML_Boolean	boolean;
	TOML_Integer	i64;
	TOML_Float		f64;
	TOML_String*	string;
	TOML_List*		list;
	TOML_Table*		table;
};

struct	TOML_Value {
	TOML_Tag	tag;
	TOML_Data	data;
	TOML_Token	token;

	TOML_Value(void);
	TOML_Value(TOML_Boolean value, TOML_Token token);
	TOML_Value(TOML_Integer value, TOML_Token token);
	TOML_Value(TOML_Float value, TOML_Token token);
	TOML_Value(TOML_String* value, TOML_Token token);
	TOML_Value(TOML_List* value, TOML_Token token);
	TOML_Value(TOML_Table* value, TOML_Token token);
};

TOML_Table	*toml_from_file(string_view& path);

#endif
