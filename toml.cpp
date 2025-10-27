/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   toml.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 10:51:12 by aindjare          #+#    #+#             */
/*   Updated: 2025/10/27 13:34:57 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

TOML_Document		TOML_make(const string_view& file) {
	TOML_Document	document;
	
	document.tokens = dynamic_array<TOML_Token>();
	document.errors = dynamic_array<TOML_Error>();

	document.file = file;
	document.bytes = dynamic_array<byte>();
	return (document);
}
void				TOML_delete(TOML_Document& document) {
	document.errors.free();
	document.tokens.free();
	document.bytes.free();
}

static void			TOML_error(TOML_Document& document, TOML_Error_Kind kind) {
	TOML_Error	error;

	error.kind = kind;
	error.pos.index = 0;
	error.pos.row = 0;
	error.pos.col = 0;
	error.pos.file = document.file;

	document.errors.push(error);
}
static void			TOML_error(TOML_Document& document, TOML_Error_Kind kind, const Position& pos) {
	TOML_Error	error;

	error.kind = kind;
	error.pos = pos;

	document.errors.push(error);
}

static void			TOML_step_load_bytes(TOML_Document& document) {
	if (document.errors.len != 0) {
		return ;
	}

	b32	ok = OS_read_file(document.file, document.bytes);
	if (!ok) {
		TOML_error(document, TOML_ERROR_LOAD_BYTES);
	}
}

static TOML_Token	TOML_token_make(TOML_Token_Kind kind, const string_view& str, const Position& pos) {
	TOML_Token	token;

	token.kind = kind;
	token.str = str;
	token.pos = pos;
	return (token);
}
static TOML_Token	TOML_token_make(TOML_Token_Kind kind, const Position& pos) {
	return (TOML_token_make(kind, "", pos));
}

static b32			TOML_match_digit(byte b) {
	return (b >= '0' && b <= '9');
}
static b32			TOML_match_ident(byte b, b32 digits = false) {
	if (digits && TOML_match_digit(b)) {
		return (true);
	} else if (b >= 'a' && b <= 'z') {
		return (true);
	} else if (b >= 'A' && b <= 'Z') {
		return (true);
	} else if (b >= '_') {
		return (true);
	}
	return (false);
}
static b32			TOML_match_whitespace(byte b) {
	return (b == ' ' || b == '\t');
}

static byte			TOML_lexer_peek(TOML_Tokenizer& lexer) {
	if (lexer.source.len > lexer.pos.index) {
		return ((byte)lexer.source[lexer.pos.index]);
	}
	return (0);
}
static byte			TOML_lexer_next(TOML_Tokenizer& lexer) {
	if (lexer.source.len > lexer.pos.index) {
		byte	b = (byte)lexer.source[lexer.pos.index++];

		lexer.pos.row++;
		if (b == '\n') {
			lexer.pos.col++;
			lexer.pos.row = 1;
		}
		return (b);
	}
	return (0);
}
static b32			TOML_lexer_walk_until_next(TOML_Tokenizer& lexer) {
	byte	b = 0;
	while (lexer.source.len > lexer.pos.index) {
		b = TOML_lexer_peek(lexer);
		if (!TOML_match_whitespace(b)) {
			break ;
		}
		TOML_lexer_next(lexer);
	}
	return (b != 0);
}
static TOML_Token	TOML_lexer_token(TOML_Document& document) {
	TOML_Tokenizer&	lexer = document.lexer;

	Position		pos = lexer.pos;
	TOML_Token_Kind	kind = TOML_TOKEN_INVALID;
	byte			b = TOML_lexer_next(lexer);
	if (b == '\n') {
		kind = TOML_TOKEN_EOL;
	} else if (b == '{') {
		kind = TOML_TOKEN_OCURLY;
	} else if (b == '{') {
		kind = TOML_TOKEN_CCURLY;
	} else if (b == '=') {
		kind = TOML_TOKEN_EQUALS;
	} else if (b == ',') {
		kind = TOML_TOKEN_COMMA;
	} else if (b == '[') {
		kind = TOML_TOKEN_OSQUARE;
		byte		b2 = TOML_lexer_peek(lexer);
		if (b2 == b) {
			TOML_lexer_next(lexer);

			kind = TOML_TOKEN_OSQUARE_2X;
		}
	} else if (b == ']') {
		kind = TOML_TOKEN_CSQUARE;
		byte		b2 = TOML_lexer_peek(lexer);
		if (b2 == b) {
			TOML_lexer_next(lexer);

			kind = TOML_TOKEN_CSQUARE_2X;
		}
	} else if (b == '"') {
		b = TOML_lexer_next(lexer);
		while (true) {
			b = TOML_lexer_peek(lexer);
			if (b == '"' || b == '\n' || b == 9) {
				break ;
			}
			TOML_lexer_next(lexer);
		}
		if (b == '"') {
			TOML_lexer_next(lexer);
		} else {
			TOML_error(document, TOML_ERROR_TOKEN_UNTERMINATED, pos);
		}

		kind = TOML_TOKEN_STRING;
	} else if (TOML_match_ident(b, /* digits = */ false)) {
		while (true) {
			b = TOML_lexer_peek(lexer);
			if (TOML_match_ident(b, /* digits = */ true)) {
				TOML_lexer_next(lexer);
				continue ;
			}
			break ;
		}

		kind = TOML_TOKEN_IDENT;
	} else if (TOML_match_digit(b)) {
		while (true) {
			b = TOML_lexer_peek(lexer);
			if (TOML_match_digit(b)) {
				TOML_lexer_next(lexer);
				continue ;
			}
			break ;
		}

		kind = TOML_TOKEN_NUMBER;
	}

	string_view	str = lexer.source.slice(pos.index, lexer.pos.index);
	if (str == "true") {
		kind = TOML_TOKEN_TRUE;
	} else if (str == "false") {
		kind = TOML_TOKEN_FALSE;
	}
	return (TOML_token_make(kind, str, pos));
}

static void			TOML_step_tokenize(TOML_Document& document) {
	if (document.errors.len != 0) {
		return ;
	}

	string_view	source = string_view((char*)document.bytes.data, document.bytes.len);
	document.lexer = (TOML_Tokenizer){ { 0, 1, 1, document.file }, source };

	while (TOML_lexer_walk_until_next(document.lexer)) {
		TOML_Token	token = TOML_lexer_token(document);
		document.tokens.push(token);
	}
	document.tokens.push(TOML_token_make(TOML_TOKEN_EOF, document.lexer.pos));
}

static void			TOML_step_parse(TOML_Document& document) {
	if (document.errors.len != 0) {
		return ;
	}

	unused(document);
}

TOML_Document		TOML_parse_file(const string_view& file) {
	TOML_Document	document = TOML_make(file);

	TOML_step_load_bytes(document);
	TOML_step_tokenize(document);
	TOML_step_parse(document);

	document.ok = (document.errors.len == 0);
	return (document);
}
