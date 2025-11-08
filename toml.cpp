/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   toml.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/27 10:51:12 by aindjare          #+#    #+#             */
/*   Updated: 2025/11/08 15:11:04 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

static void			TOML_value_free(const TOML_Value& value) {
	if (value.Table == 0) { /* NOTE(xenobas): This catches all zeroed variants */
		return ;
	}

	switch (value.kind) {
	case TOML_VALUE_TABLE: {
		TOML_Table&		tbl = *value.Table;
		for_table_begin(tbl, TOML_Table, entry) {
			TOML_value_free(entry.value);
		} for_table_end ;
		tbl.free();

		delete value.Table;
	} break;
	case TOML_VALUE_ARRAY_TABLES:
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
	
	document.root = (TOML_Value){ TOML_VALUE_NIL, { 0, 1, 1, file }, { .Table = 0 } };
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
static void			TOML_error(TOML_Document& document, TOML_Error_Kind kind, const TOML_Token& token) {
	TOML_Error	error;

	error.kind = kind;
	error.pos = token.pos;

	error.str = "";
	error.token = token;
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
static void			TOML_error(TOML_Document& document, TOML_Error_Kind kind, const Position& pos, const TOML_Token& token, const string_view& str) {
	TOML_Error	error;

	error.kind = kind;
	error.pos = pos;

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
			return ("<end of file>");
		case TOML_TOKEN_EOL:
			return ("\\n");

		case TOML_TOKEN_EQUALS:
			return ("=");
		case TOML_TOKEN_COMMA:
			return (",");

		case TOML_TOKEN_OSQUARE:
			return ("[");
		case TOML_TOKEN_CSQUARE:
			return ("]");

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
	b32		comment = 0;
	while (lexer.source.len > lexer.pos.index) {
		b = TOML_lexer_peek(lexer);
		if (b == '#') {
			comment = 1;
		}
		if (comment) {
			if (b == '\n') {
				break ;
			}
		} else {
			if (!TOML_match_whitespace(b)) {
				break ;
			}
		}
		TOML_lexer_next(lexer);
	}
	return (b != 0 && lexer.source.len > lexer.pos.index);
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
	} else if (b == ']') {
		kind = TOML_TOKEN_CSQUARE;
	} else if (b == '"') {
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
	} else if (TOML_match_digit(b) || b == '-') {
		b32	is_neg = (b == '-');
		i32	count_trailing = 0;
		for (;;) {
			b = TOML_lexer_peek(lexer);
			if (TOML_match_digit(b)) {
				count_trailing += 1;

				TOML_lexer_next(lexer);
				continue ;
			}
			break ;
		}

		if (!is_neg || count_trailing > 0) {
			kind = TOML_TOKEN_NUMBER;
		}
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
static void			TOML_parser_advance(TOML_Parser& parser, b32 ignore_eol = 0) {
	while (!TOML_parser_done(parser)) {
		parser.token = parser.lexer->tokens[++parser.index];
		if (!ignore_eol || parser.token.kind != TOML_TOKEN_EOL) {
			break ;
		}
	}
}
static b32			TOML_parser_expect(TOML_Document& document, TOML_Parser& parser, TOML_Token_Kind kind, string_view category = "", b32 ignore_eol = 0) {
	TOML_Token	token = parser.token;
	if (kind == TOML_TOKEN_EOL) {
		if (token.kind != kind && token.kind != TOML_TOKEN_EOF) {
			if (!category)
				category = TOML_token_kind_string(kind);
			TOML_error(document, TOML_ERROR_PARSER_EXPECT, token, category);
			return (0);
		}
	} else {
		while (ignore_eol && parser.token.kind == TOML_TOKEN_EOL) {
			TOML_parser_advance(parser);
		}
		token = parser.token;
		if (token.kind != kind) {
			if (!category)
				category = TOML_token_kind_string(kind);
			TOML_error(document, TOML_ERROR_PARSER_EXPECT, token, category);
			return (0);
		}
	}
	TOML_parser_advance(parser);
	return (1);
}
static b32			TOML_parser_accept(TOML_Parser& parser, TOML_Token_Kind kind) {
	TOML_Token	token = parser.token;
	if (token.kind == kind) {
		if (kind != TOML_TOKEN_EOF) {
			parser.token = parser.lexer->tokens[++parser.index];
		}
		return (1);
	}
	return (0);
}

static TOML_Value	TOML_parse_value(TOML_Document& document, TOML_Parser& parser);

static b32			TOML_parse_number_range_check(i64 number, i64 sign, byte d) {
	i64	v = cast(i64)(d - '0');
	switch (sign) {
	case +1: {
		if (I64_MAX / 10 < number || (I64_MAX / 10 == number && I64_MAX % 10 < v)) {
			return (false);
		}
	} break ;
	case -1: {
		if (-(I64_MIN / 10) < number || (-(I64_MIN / 10) == number && -(I64_MIN % 10) < v)) {
			return (false);
		}
	} break ;
	}
	return (sign == -1 || sign == +1);
}
static TOML_Value	TOML_parse_number(TOML_Document& document, TOML_Parser& parser) {
	TOML_Token	token = parser.token;
	TOML_Value	nil = { TOML_VALUE_NIL, token.pos, { .Table = 0 } };
	if (!TOML_parser_expect(document, parser, TOML_TOKEN_NUMBER)) {
		return (nil);
	}

	string_view	str = token.str;

	i64			sign = +1;
	i32			index = 0;
	i64			number = 0;

	if (str.len > 0 && str[0] == '-') {
		sign = -1;
		index++;
	}
	while (index < str.len) {
		byte	b = (byte)str[index];
		if (b < '0' || b > '9') {
			TOML_error(document, TOML_ERROR_PARSER_NUMBER_CHAR, token);
			return (nil);
		}
		if (!TOML_parse_number_range_check(number, sign, b)) {
			TOML_error(document, TOML_ERROR_PARSER_NUMBER_RANGE, token);
			return (nil);
		}

		i64		n = cast(i64)(b - '0');
		number = (number * 10l) + n;
		index++;
	}
	return ((TOML_Value){ TOML_VALUE_NUMBER, token.pos, { .Number = number * sign } });
}
static TOML_Value	TOML_parse_string(TOML_Document& document, TOML_Parser& parser) {
	TOML_Token	token = parser.token;
	TOML_Value	nil = { TOML_VALUE_NIL, token.pos, { .Table = 0 } };
	if (!TOML_parser_expect(document, parser, TOML_TOKEN_STRING)) {
		return (nil);
	}

	string_view		str = token.str;
	if (str.len < 2 || !str.has_prefix("\"") || !str.has_suffix("\"")) {
		TOML_error(document, TOML_ERROR_PARSER_STRING_QUOTES, token);
		return (nil);
	}

	/* NOTE(xenobas): this is completely and utterly retarded */
	string_view		slice = str.slice(1, str.len - 1);
	string_view*	string = new string_view(string_view::alloc(slice));
	return ((TOML_Value){ TOML_VALUE_STRING, token.pos, { .String = string } });
}
static TOML_Value	TOML_parse_boolean(TOML_Document& document, TOML_Parser& parser) {
	TOML_Token	token = parser.token;
	TOML_parser_advance(parser);

	TOML_Value	nil = { TOML_VALUE_NIL, token.pos, { .Table = 0 } };
	if (token.kind != TOML_TOKEN_TRUE && token.kind != TOML_TOKEN_FALSE) {
		TOML_error(document, TOML_ERROR_PARSER_EXPECT, token, "boolean");
		return (nil);
	}

	b32	boolean = (token.kind == TOML_TOKEN_TRUE);
	return ((TOML_Value){ TOML_VALUE_BOOLEAN, token.pos, { .Boolean = boolean } });
}
static void			TOML_parse_array_recover(TOML_Parser& parser, b32 comma_delimits = 1) {
	while (!TOML_parser_done(parser)) {
		TOML_Token	token = parser.token;
		if (token.kind == TOML_TOKEN_CSQUARE || token.kind == TOML_TOKEN_IDENT || (comma_delimits && token.kind == TOML_TOKEN_COMMA)) {
			break ;
		}
		TOML_parser_advance(parser);
	}
}
static TOML_Value	TOML_parse_array(TOML_Document& document, TOML_Parser& parser) {
	TOML_Token	open = parser.token;
	TOML_Value	nil = { TOML_VALUE_NIL, open.pos, { .Table = 0 } };
	if (!TOML_parser_expect(document, parser, TOML_TOKEN_OSQUARE)) {
		return (nil);
	}

	TOML_Array	array;
	b32			comma_break = 0;
	while (!TOML_parser_done(parser)) {
		TOML_Token	eol_maybe = parser.token;
		if (eol_maybe.kind == TOML_TOKEN_EOL) {
			TOML_parser_advance(parser, /* ignore_eol = */ 1);
		}

		TOML_Token	csquare_maybe = parser.token;
		if (csquare_maybe.kind == TOML_TOKEN_CSQUARE) {
			break ;
		}

		TOML_Value	value = TOML_parse_value(document, parser);
		if (value.kind == TOML_VALUE_NIL) {
			TOML_parse_array_recover(parser);
		} else {
			array.push(value);
		}

		eol_maybe = parser.token;
		if (eol_maybe.kind == TOML_TOKEN_EOL) {
			TOML_parser_advance(parser, /* ignore_eol = */ 1);
		}

		TOML_Token	comma = parser.token;
		if (comma.kind != TOML_TOKEN_COMMA) {
			comma_break = true;
			break ;
		} else {
			TOML_parser_advance(parser, /* ignore_eol = */ 1);
		}
	}

	TOML_Token	close = parser.token;
	if (!TOML_parser_expect(document, parser, TOML_TOKEN_CSQUARE, /* category = */ "", /* ignore_eol = */ 1)) {
		if (comma_break) {
			TOML_parse_array_recover(parser, /* comma_delimits = */ 0);
			if (parser.token.kind == TOML_TOKEN_CSQUARE) {
				TOML_parser_advance(parser);
			}
		}
		array.free();
		return (nil);
	}
	return ((TOML_Value){ TOML_VALUE_ARRAY, open.pos, { .Array = new TOML_Array(array) } });
}
static void			TOML_parse_table_recover(TOML_Parser& parser, b32 comma_delimits = 1) {
	while (!TOML_parser_done(parser)) {
		TOML_Token	token = parser.token;
		if (token.kind == TOML_TOKEN_CCURLY || token.kind == TOML_TOKEN_IDENT || (comma_delimits && token.kind == TOML_TOKEN_COMMA)) {
			break ;
		}
		TOML_parser_advance(parser);
	}
}
static TOML_Value	TOML_parse_table(TOML_Document& document, TOML_Parser& parser) {
	TOML_Token	open = parser.token;
	TOML_Value	nil = { TOML_VALUE_NIL, open.pos, { .Table = 0 } };
	if (!TOML_parser_expect(document, parser, TOML_TOKEN_OCURLY)) {
		return (nil);
	}

	TOML_Table	table;
	while (!TOML_parser_done(parser)) {
		TOML_Token	token = parser.token;
		if (token.kind == TOML_TOKEN_EOL) {
			TOML_parser_advance(parser, /* ignore_eol = */ 1);
			token = parser.token;
		}
		if (token.kind == TOML_TOKEN_CCURLY) {
			break ;
		}
		TOML_Token	ident = parser.token;
		b32			pair_ok = 0;
		if (TOML_parser_expect(document, parser, TOML_TOKEN_IDENT, /* category = */ "", /* ignore_eol = */ 1)) {
			if (TOML_parser_expect(document, parser, TOML_TOKEN_EQUALS)) {
				TOML_Value	value = TOML_parse_value(document, parser);

				pair_ok = (value.kind != TOML_VALUE_NIL);
				/* TODO(xenobas): Do duplicate check */
				if (pair_ok) {
					table.set(ident.str, value);
				}
			}
		}
		if (!pair_ok) {
			TOML_parse_table_recover(parser);
		}
		if (!TOML_parser_accept(parser, TOML_TOKEN_COMMA)) {
			break ;
		}
	}

	TOML_Token	close = parser.token;
	if (!TOML_parser_expect(document, parser, TOML_TOKEN_CCURLY, /* category = */ "", /* ignore_eol = */ 1)) {
		table.destroy();
		return (nil);
	}
	return ((TOML_Value){ TOML_VALUE_TABLE, open.pos, { .Table = new TOML_Table(table) } });
}
static TOML_Value	TOML_parse_value(TOML_Document& document, TOML_Parser& parser) {
	TOML_Token	token = parser.token;
	if (token.kind == TOML_TOKEN_NUMBER) {
		return (TOML_parse_number(document, parser));
	} else if (token.kind == TOML_TOKEN_STRING) {
		return (TOML_parse_string(document, parser));
	} else if (token.kind == TOML_TOKEN_TRUE || token.kind == TOML_TOKEN_FALSE) {
		return (TOML_parse_boolean(document, parser));
	} else if (token.kind == TOML_TOKEN_OSQUARE) {
		return (TOML_parse_array(document, parser));
	} else if (token.kind == TOML_TOKEN_OCURLY) {
		return (TOML_parse_table(document, parser));
	}
	TOML_error(document, TOML_ERROR_PARSER_EXPECT, token, "value");
	return ((TOML_Value){ TOML_VALUE_NIL, token.pos, { 0 } });
}
static void			TOML_parse_stmt_recovery(TOML_Parser& parser) {
	while (!TOML_parser_done(parser)) {
		if (parser.token.kind == TOML_TOKEN_EOL) {
			break ;
		}
		TOML_parser_advance(parser);
	}
}
static void			TOML_parse_stmt(TOML_Document& document, TOML_Parser& parser) {
	TOML_Token	token = parser.token;
	if (TOML_parser_accept(parser, TOML_TOKEN_OSQUARE)) { // [Table]
		b32			array_of_tables = TOML_parser_accept(parser, TOML_TOKEN_OSQUARE); // [[Array of Tables]]
		TOML_Token	token_key = parser.token;
		if (!TOML_parser_expect(document, parser, TOML_TOKEN_IDENT)) {
			TOML_parse_stmt_recovery(parser);
			TOML_parser_expect(document, parser, TOML_TOKEN_EOL);
			return ;
		}
		if (!TOML_parser_expect(document, parser, TOML_TOKEN_CSQUARE)) {
			TOML_parse_stmt_recovery(parser);
			TOML_parser_expect(document, parser, TOML_TOKEN_EOL);
			return ;
		}
		if (array_of_tables && !TOML_parser_expect(document, parser, TOML_TOKEN_CSQUARE)) {
			TOML_parse_stmt_recovery(parser);
			TOML_parser_expect(document, parser, TOML_TOKEN_EOL);
			return ;
		}
		if (!TOML_parser_expect(document, parser, TOML_TOKEN_EOL)) {
			TOML_parse_stmt_recovery(parser);
			if (parser.token.kind == TOML_TOKEN_EOL) {
				TOML_parser_advance(parser);
			}
			return ;
		}

		if (!array_of_tables) {
			TOML_error(document, TOML_ERROR_PARSER_UNSUPPORTED, token.pos, token_key, "[table]");
			return ;
		}
		
		/* Pick root value, and make sure it is an array, and insert a value in it */
		TOML_Value&	root = document.root;
		// new Table
		b32 set_scope = 1;
		if (root.Table->has(token_key.str)) {
			TOML_Value&	container = root.Table->get(token_key.str);
			if (container.kind == TOML_VALUE_ARRAY_TABLES) {
				TOML_Value	table = { TOML_VALUE_TABLE, token.pos, { .Table = new TOML_Table } };
				container.Array->push(table);
			} else {
				set_scope = 0;
				TOML_error(document, TOML_ERROR_PARSER_SCOPE_KEY_DUP, token.pos, token_key, token_key.str);
			}
		} else {
			TOML_Value	container = { TOML_VALUE_ARRAY_TABLES, token.pos, { .Array = new TOML_Array } };
			TOML_Value	table = { TOML_VALUE_TABLE, token.pos, { .Table = new TOML_Table } };

			container.Array->push(table);
			root.Table->set(token_key.str, container);
		}

		if (set_scope) {
			TOML_Value&	container = root.Table->get(token_key.str);
			parser.scope = &(*container.Array)[container.Array->len - 1];
		}
		return ;
	}
	if (token.kind == TOML_TOKEN_IDENT) { // Key = Value
		TOML_parser_advance(parser);
		if (!TOML_parser_expect(document, parser, TOML_TOKEN_EQUALS)) {
			TOML_parse_stmt_recovery(parser);
			TOML_parser_expect(document, parser, TOML_TOKEN_EOL);
			return ;
		}

		TOML_Value	value = TOML_parse_value(document, parser);
		if (value.kind == TOML_VALUE_NIL) {
			TOML_parse_stmt_recovery(parser);
			TOML_parser_expect(document, parser, TOML_TOKEN_EOL);
			return ;
		}
		if (!TOML_parser_expect(document, parser, TOML_TOKEN_EOL)) {
			TOML_parse_stmt_recovery(parser);
			if (parser.token.kind == TOML_TOKEN_EOL) {
				TOML_parser_advance(parser);
			}
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
	if (token.kind == TOML_TOKEN_EOL) {
		TOML_parser_advance(parser, /* ignore_eol = */ 1);
		return ;
	}

	TOML_error(document, TOML_ERROR_PARSER_EXPECT, token, "statement");
	TOML_parse_stmt_recovery(parser);
}

static void			TOML_value_debug(const TOML_Value& value, i32 indent = 0, string_view key = "") {
	printf("%*s%.*s", indent * 2, "", key.len, key.text);
	if (key) {
		printf(" = ");
	}

	switch (value.kind) {
	case TOML_VALUE_BOOLEAN:
		printf("%s\n", value.Boolean ? "true" : "false");
		break ;
	case TOML_VALUE_NUMBER:
		printf("%ld\n", value.Number);
		break ;
	case TOML_VALUE_STRING: {
		if (value.String == 0) {
			printf("nil\n");
			return ;
		}
		const TOML_String&	str = *value.String;
		printf("\"%.*s\"\n", str.len, str.text);
	} break ;
	case TOML_VALUE_ARRAY_TABLES:
	case TOML_VALUE_ARRAY: {
		if (value.Array == 0) {
			printf("nil\n");
			return ;
		}
		const TOML_Array&	arr = *value.Array;
		printf("%s %d elements\n", value.kind == TOML_VALUE_ARRAY ? "array" : "tables array", arr.len);
		for (i32 i = 0; i < arr.len; ++i) {
			TOML_value_debug(arr[i], indent + 1);
		}
	} break ;
	case TOML_VALUE_TABLE: {
		if (value.Table == 0) {
			printf("nil\n");
			return ;
		}

		const TOML_Table&		tbl = *value.Table;

		printf("table %d entries\n", tbl.count);
		for_table_begin(tbl, const TOML_Table, entry) {
			TOML_value_debug(entry.value, indent + 1, entry.key);
		} for_table_end ;
	} break ;
	case TOML_VALUE_NIL:
	default:
		printf("nil\n");
	}
}

static void			TOML_step_parse(TOML_Document& document) {
	unused(TOML_value_debug); /* NOTE(xenobas): To avoid unused static function error */

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
