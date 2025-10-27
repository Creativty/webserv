/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   toml.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 10:51:12 by aindjare          #+#    #+#             */
/*   Updated: 2025/10/27 18:27:29 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

static void			TOML_value_free(const TOML_Value& value) {
	switch (value.kind) {
	case TOML_VALUE_TABLE: {
		TOML_Table*		tbl = value.Table;
		for (i32 i = 0; i < tbl->cap; ++i) {
			TOML_Table::hash_table_item& item = tbl->items[i];
			if (item.used()) {
				TOML_value_free(item.value);
			}
		}
		tbl->destroy();
		delete tbl;
	} break;
	case TOML_VALUE_ARRAY: {
		TOML_Array*		arr = value.Array;
		for (i32 i = 0; i < arr->len; ++i) {
			TOML_value_free((*arr)[i]);
		}
		arr->free();
		delete arr;
	} break;
	case TOML_VALUE_STRING: {
		TOML_String*	str = value.String;
		str->free();
		delete str;
	} break;
	case TOML_VALUE_NUMBER:
	case TOML_VALUE_BOOLEAN:
	case TOML_VALUE_NIL:
	default: {
	} break;
	}
};

TOML_Document		TOML_make(const string_view& file) {
	TOML_Document	document;
	
	document.root = (TOML_Value){ TOML_VALUE_NIL, { 0, 1, 1, file }, { 0 } };
	document.errors = dynamic_array<TOML_Error>();

	document.parser = (TOML_Parser){ 0, { TOML_TOKEN_INVALID, "", { 0, 1, 1, file } }, &document.lexer, 0 };
	document.lexer = (TOML_Tokenizer){ dynamic_array<TOML_Token>(), { 0, 1, 1, file }, "" };

	document.file = file;
	document.bytes = dynamic_array<byte>();

	document.ok = false;
	return (document);
}
void				TOML_delete(TOML_Document& document) {
	TOML_value_free(document.root);
	document.lexer.tokens.free();
	document.bytes.free();
	document.errors.free();
}

static void			TOML_error(TOML_Document& document, TOML_Error_Kind kind) {
	TOML_Error	error;

	error.kind = kind;
	error.pos = (Position){ 0, 1, 1, document.file };

	error.str = "";
	error.token = (TOML_Token){ TOML_TOKEN_INVALID, "", error.pos };
	error.value = 0;

	document.errors.push(error);
}
static void			TOML_error(TOML_Document& document, TOML_Error_Kind kind, const Position& pos) {
	TOML_Error	error;

	error.kind = kind;
	error.pos = pos;

	error.str = "";
	error.token = (TOML_Token){ TOML_TOKEN_INVALID, "", error.pos };
	error.value = 0;

	document.errors.push(error);
}
static void			TOML_error(TOML_Document& document, TOML_Error_Kind kind, const TOML_Token& token, const string_view& str) {
	TOML_Error	error;

	error.kind = kind;
	error.pos = token.pos;

	error.str = str;
	error.token = token;
	error.value = 0;

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
static string_view	TOML_token_kind_string(const TOML_Token_Kind& kind) {
	switch (kind) {
		case TOML_TOKEN_EOF:
			return ("<EOF>");
		case TOML_TOKEN_EOL:
			return ("<EOL> or \\n");

		case TOML_TOKEN_EQUALS:
			return ("=");
		case TOML_TOKEN_COMMA:
			return (",");

		case TOML_TOKEN_OSQUARE:
			return ("[");
		case TOML_TOKEN_CSQUARE:
			return ("]");

		case TOML_TOKEN_OSQUARE_2X:
			return ("[[");
		case TOML_TOKEN_CSQUARE_2X:
			return ("]]");

		case TOML_TOKEN_OCURLY:
			return ("{");
		case TOML_TOKEN_CCURLY:
			return ("}");

		case TOML_TOKEN_IDENT:
			return ("identifier");

		case TOML_TOKEN_TRUE:
			return ("true");
		case TOML_TOKEN_FALSE:
			return ("false");

		case TOML_TOKEN_NUMBER:
			return ("number");
		case TOML_TOKEN_STRING:
			return ("string");
		case TOML_TOKEN_INVALID:
		default:
			return ("<INVALID>");
	}
}

static b32			TOML_match_digit(byte b) {
	return (b >= '0' && b <= '9');
}
static b32			TOML_match_ident(byte b, b32 digits = false) {
	if (digits && TOML_match_digit(b)) {
		return (1);
	} else if (b >= 'a' && b <= 'z') {
		return (1);
	} else if (b >= 'A' && b <= 'Z') {
		return (1);
	} else if (b >= '_') {
		return (1);
	}
	return (0);
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
	} else if (b == '}') {
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
		while (1) {
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
	} else if (TOML_match_ident(b, /* digits = */ 0)) {
		for (;;) {
			b = TOML_lexer_peek(lexer);
			if (TOML_match_ident(b, /* digits = */ 1)) {
				TOML_lexer_next(lexer);
				continue ;
			}
			break ;
		}

		kind = TOML_TOKEN_IDENT;
	} else if (TOML_match_digit(b)) {
		for (;;) {
			b = TOML_lexer_peek(lexer);
			if (TOML_match_digit(b)) {
				TOML_lexer_next(lexer);
				continue ;
			}
			break ;
		}

		kind = TOML_TOKEN_NUMBER;
	} else {
		TOML_error(document, TOML_ERROR_TOKEN_INVALID, pos);

		kind = TOML_TOKEN_INVALID;
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

	document.lexer.source = string_view((char*)document.bytes.data, document.bytes.len);
	while (TOML_lexer_walk_until_next(document.lexer)) {
		TOML_Token	token = TOML_lexer_token(document);
		document.lexer.tokens.push(token);
	}
	document.lexer.tokens.push(TOML_token_make(TOML_TOKEN_EOF, document.lexer.pos));
}

static b32			TOML_parser_done(TOML_Parser& parser) {
	if (parser.lexer->tokens.len > parser.index) {
		return (parser.lexer->tokens[parser.index].kind == TOML_TOKEN_EOF);
	}
	return (1);
}
static void			TOML_parser_advance(TOML_Parser& parser, bool ignore_eol = false) {
	while (!TOML_parser_done(parser)) {
		parser.token = parser.lexer->tokens[++parser.index];
		if (ignore_eol && parser.token.kind == TOML_TOKEN_EOL) {
			continue ;
		}
		break ;
	}
}
static b32			TOML_parser_expect(TOML_Document& document, TOML_Parser& parser, TOML_Token_Kind kind, string_view category = "") {
	TOML_Token	token = parser.token;
	if (token.kind != kind) {
		if (!category)
			category = TOML_token_kind_string(kind);
		TOML_error(document, TOML_ERROR_PARSER_EXPECT, token, category);
		return (0);
	}
	TOML_parser_advance(parser);
	return (1);
}

static TOML_Value	TOML_parse_value(TOML_Document& document, TOML_Parser& parser) {
	TOML_Token	token = parser.token;
	TOML_Value	value = { TOML_VALUE_NIL, token.pos, { 0 } };

	unused(document);
	if (token.kind == TOML_TOKEN_NUMBER) {
	} else if (token.kind == TOML_TOKEN_STRING) {
	} else if (token.kind == TOML_TOKEN_TRUE) {
	} else if (token.kind == TOML_TOKEN_FALSE) {
	} else {
	}
	return (value);
}
static void			TOML_parse_stmt_recovery(TOML_Parser& parser) {
	while (!TOML_parser_done(parser)) {
		if (parser.token.kind == TOML_TOKEN_EOL) {
			TOML_parser_advance(parser);
			break ;
		}
		TOML_parser_advance(parser);
	}
}
static void			TOML_parse_stmt(TOML_Document& document, TOML_Parser& parser) {
	TOML_Token	token = parser.token;
	if (token.kind == TOML_TOKEN_OSQUARE_2X) { // [[Array_of_Tables]]
		TOML_parser_advance(parser);
		return ;
	}
	if (token.kind == TOML_TOKEN_OSQUARE) { // [Table]
		TOML_parser_advance(parser);
		return ;
	}
	if (token.kind == TOML_TOKEN_IDENT) { // Key = Value
		TOML_parser_advance(parser);
		if (!TOML_parser_expect(document, parser, TOML_TOKEN_EQUALS)) {
			TOML_parse_stmt_recovery(parser);
			return ;
		}
		TOML_Value	value = TOML_parse_value(document, parser);
		if (!TOML_parser_expect(document, parser, TOML_TOKEN_EOL)) {
			return ;
		}

		string_view	key = token.str;
		if (parser.scope->Table->has(key)) {
			TOML_error(document, TOML_ERROR_PARSER_SCOPE_KEY_DUP, token, key);
			return ;
		}
		parser.scope->Table->set(key, value);

		return ;
	}

	TOML_error(document, TOML_ERROR_PARSER_EXPECT, token, "statement");
	TOML_parse_stmt_recovery(parser);
}

static void			TOML_step_parse(TOML_Document& document) {
	if (document.errors.len != 0) {
		return ;
	}

	document.root = (TOML_Value){ TOML_VALUE_TABLE, { 0, 1, 1, document.file }, { .Table = new TOML_Table } };
	if (document.lexer.tokens.len == 0) {
		return ;
	}

	document.parser.scope = &document.root;
	document.parser.token = document.lexer.tokens[0];
	while (!TOML_parser_done(document.parser)) {
		TOML_parse_stmt(document, document.parser);
	}
}

TOML_Document		TOML_parse_file(const string_view& file) {
	TOML_Document	document = TOML_make(file);

	TOML_step_load_bytes(document);
	if (document.errors.len == 0)
		CLI_debug("TOML loaded %d bytes.", document.bytes.len);

	TOML_step_tokenize(document);
	if (document.errors.len == 0)
		CLI_debug("TOML lexer collected %d tokens.", document.lexer.tokens.len);

	TOML_step_parse(document);
	if (document.errors.len == 0)
		CLI_debug("TOML parser collected %d instances.", document.root.Table->count);

	document.ok = (document.errors.len == 0);
	return (document);
}
