/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   toml.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 15:31:05 by aindjare          #+#    #+#             */
/*   Updated: 2025/09/16 17:03:31 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef   TOML_HPP
#define   TOML_HPP
#include <iostream>
#include "types.hpp"

namespace toml {
	typedef type::string				string;

	struct location {
		u32	index;
		u32 line;
		u32	column;

		location(void);
		location(u32, u32, u32);
		location(const location&);
		~location(void);
		location&	operator=(const location&);

		void		advance(byte);
	};

	enum token_kind {
		TOKEN_INVALID,

		TOKEN_ASSIGN,
		TOKEN_COMMA,
		TOKEN_NEWLINE,
		TOKEN_TABLE_INLINE_OPEN,
		TOKEN_TABLE_INLINE_CLOSE,
		TOKEN_ARRAY_OPEN,
		TOKEN_ARRAY_CLOSE,

		TOKEN_STRING,
		TOKEN_NUMBER,
		TOKEN_IDENT,
		TOKEN_TRUE,
		TOKEN_FALSE,

		TOKEN_EOF,
	};
	struct token {
		string		text;
		token_kind	kind;
		location	loc;

		token(void);
		~token(void);
		token(const token&);
		token&	operator=(const token&);

		token(token_kind);
		token(string, token_kind, const location&);
	};
	typedef type::dynamic_array<token>	token_array;

	struct lexer {
		typedef bool (*lexer_predicate)(byte);
		const string	source;
		location		loc_curr;
		location		loc_last;
		u32				line_begin_index;

		lexer(const string);
		~lexer(void);

		bool			end(void) const;

		byte			peek_byte(void) const;
		string			peek_text(void) const;

		byte			next_byte(void);
		string			next_text(void);
		token			next_token(token_kind);

		u32				next_byte_while(lexer_predicate);
		u32				skip_byte_while(lexer_predicate);
		u32				next_byte_while_not(lexer_predicate);
		u32				skip_byte_while_not(lexer_predicate);

		void			report_error(const char*);

		static bool		process(token_array&, const string);
	};

	enum value_kind {
		VALUE_INVALID,

		VALUE_NUMBER,
		VALUE_BOOLEAN,
		VALUE_STRING,
		VALUE_ARRAY,
		VALUE_TABLE,
	};
	struct value;

	typedef i64							value_number;
	typedef bool						value_boolean;
	typedef type::string				value_string;
	typedef type::dynamic_array<value>	value_array;
	typedef type::hash_map<value>		value_table;
	
	struct value {
		value_kind			kind;
		union {
			value_number	number;
			value_boolean	boolean;
			value_string*	string;
			value_array*	array;
			value_table*	table;
		}					data;

		value(void);
		~value(void);

		value(value_number);
		value(value_boolean);
		value(value_string*);
		value(value_array*);
		value(value_table*);

		void				free(void);
	};
	struct parser {
		typedef bool (*parser_predicate)(token_kind);
		token_array&	tokens;
		value_table*	document;
		value_table*	scope;
		u32				index;
		token			curr;
		bool			ok;

		parser(token_array&, value_table&);
		~parser(void);

		bool			end(void) const;
		token			peek(void) const;

		token			next(void);

		u32				next_while(token_kind);

		u32				next_while_not(token_kind);
		u32				next_while_not(parser_predicate);

		bool			accept(token_kind);

		bool			expect(token_kind);
		bool			expect(token_kind*, u64);

		bool			expect_recover(token_kind, token_kind);
		bool			expect_recover(token_kind*, u64, token_kind);

		static bool		process(token_array&, value_table&);
	};

	bool	parse_value(parser&, value*, token_kind);
	bool	parse_keyval(parser&, value_table*);
	bool	parse_document(parser&);

	bool	process(const string, token_array&, toml::value_table*);
};

std::ostream&	operator<<(std::ostream&, const toml::token_kind&);
std::ostream&	operator<<(std::ostream&, const toml::value_kind&);
std::ostream&	operator<<(std::ostream&, const toml::token&);
std::ostream&	operator<<(std::ostream&, const toml::value&);
#endif // TOML_HPP
