/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   toml.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/16 16:14:31 by aindjare          #+#    #+#             */
/*   Updated: 2025/09/16 17:20:36 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ostream>
#include <iostream>

#include <stdint.h>

#include "types.hpp"
#include "strconv.hpp"
#include "terminal.hpp"
#include "toml.hpp"

/* Matchers */

static byte	to_lower(byte b) {
	return ((b >= 'A' && b <= 'Z') ? (b ^ 32) : b);
};
static bool	is_digit(byte d) {
	return (d >= '0' && d <= '9');
};
static bool	is_alphabet(byte b) {
	byte	a = to_lower(b);
	return (a >= 'a' && a <= 'z');
};
static bool	is_newline(byte n) {
	return (n == '\n');
}
static bool	is_string_delimiter(byte d) {
	return (is_newline(d) || d == '"');
}
static bool	is_whitespace(byte w) {
	return (w == ' ' || w <= '\t');
};
static bool	is_identifier_prefix(byte b) {
	return (is_alphabet(b) || b == '_');
};
static bool	is_identifier_infix(byte b) {
	return (is_identifier_prefix(b) || is_digit(b));
};

/* Location */

toml::location::location(void): index(0u), line(1u), column(1u) {
}
toml::location::~location(void) {
}
toml::location::location(const location& loc): index(loc.index), line(loc.line), column(loc.column) {
}
toml::location& toml::location::operator=(const toml::location& ref) {
	if (this != &ref) {
		index = ref.index;
		line = ref.line;
		column = ref.column;
	}
	return (*this);
}

toml::location::location(u32 index, u32 line, u32 column): index(index), line(line), column(column) {
}

void			toml::location::advance(byte b) {
	if (b == '\0') return ;

	index++;
	if (b != '\n') column++;
	else {
		line++;
		column = 1u;
	}
}

/* Token */

toml::token::token(void): text(), kind(toml::TOKEN_INVALID), loc() {
}
toml::token::~token(void) {
}
toml::token::token(const token& ref): text(ref.text), kind(ref.kind), loc(ref.loc) {
}
toml::token& toml::token::operator=(const token& ref) {
	if (this != &ref) {
		text = ref.text;
		kind = ref.kind;
		loc = ref.loc;
	}
	return (*this);
}

toml::token::token(token_kind kind): text(), kind(kind), loc() {
}

toml::token::token(string text, token_kind kind, const location& loc): text(text), kind(kind), loc(loc) {
}

/* Lexer */

toml::lexer::lexer(const string source): source(source), loc_curr(), loc_last(), line_begin_index() {
}
toml::lexer::~lexer(void) {
}

bool			toml::lexer::end(void) const {
	return (loc_curr.index >= source.len);
}

byte			toml::lexer::peek_byte(void) const {
	if (end()) return (0);
	return (source[loc_curr.index]);
}
toml::string	toml::lexer::peek_text(void) const {
	return (source.slice(loc_last.index, loc_curr.index));
}

byte			toml::lexer::next_byte(void) {
	byte	b = peek_byte();

	loc_curr.advance(b);
	if (b == '\n')
		line_begin_index = loc_curr.index;
	return (b);
}
toml::string	toml::lexer::next_text(void) {
	string	text = peek_text();

	loc_last = loc_curr;
	return (text);
}
toml::token		toml::lexer::next_token(token_kind kind) {
	location	loc = loc_last;
	string		text = next_text();
	return (token(text, kind, loc));
}

u32				toml::lexer::next_byte_while(toml::lexer::lexer_predicate predicate) {
	u32	count = 0u;
	while (!end()) {
		if (!predicate(peek_byte()))
			break ;
		next_byte();
	}
	return (count);
}
u32				toml::lexer::skip_byte_while(toml::lexer::lexer_predicate predicate) {
	u32	count = next_byte_while(predicate);

	next_text();
	return (count);
}
u32				toml::lexer::next_byte_while_not(toml::lexer::lexer_predicate predicate) {
	u32	count = 0u;
	while (!end()) {
		if (predicate(peek_byte()))
			break ;
		next_byte();
	}
	return (count);
}
u32				toml::lexer::skip_byte_while_not(toml::lexer::lexer_predicate predicate) {
	u32	count = next_byte_while_not(predicate);

	next_text();
	return (count);
}

void			toml::lexer::report_error(const char* note) {
	std::cerr << TERMINAL_COLOR_WHITE "(unimplemented toml path):";
	std::cerr << loc_last.line << ':' << loc_last.column << ": ";
	std::cerr << TERMINAL_NOTICE_ERROR ": " TERMINAL_STYLE_RESET;
	std::cerr << note << std::endl;

	i32		line_end_index = source
		.slice(line_begin_index)
		.find('\n');
	if (line_end_index == -1)
		line_end_index = (i32)source.len;

	string	line = source.slice(line_begin_index, (u32)line_end_index);
	for (u32 i = 0u; i < line.len; ++i) {
		byte	b = line[i];
		if (is_newline(b))
			std::cerr << std::endl << "\t|\t" << std::endl;
		else
			std::cerr << b;
	}
	std::cerr << std::endl;
}

bool			toml::lexer::process(token_array& tokens, const string source) {
	lexer	l(source);
	while (!l.end()) {
		l.skip_byte_while(is_whitespace);
		byte		b = l.next_byte();
		token_kind	k = TOKEN_INVALID;
	
		if (b == '\n') k = TOKEN_NEWLINE;
		else if (b == ',') k = TOKEN_COMMA;
		else if (b == '=') k = TOKEN_ASSIGN;
		else if (b == '{') k = TOKEN_TABLE_INLINE_OPEN;
		else if (b == '}') k = TOKEN_TABLE_INLINE_CLOSE;
		else if (b == '[') k = TOKEN_ARRAY_OPEN;
		else if (b == ']') k = TOKEN_ARRAY_CLOSE;
		else if (b == '#') {
			l.skip_byte_while_not(is_newline);
			continue ;
		}
		else if (b == '"') {
			l.next_byte_while_not(is_string_delimiter);
			if (l.peek_byte() == '"') l.next_byte();
			else {
				l.report_error("missing terminating \" character");
				return (false);
			}
			k = TOKEN_STRING;
		}
		else if (is_digit(b)) {
			l.next_byte_while(is_digit);
			k = TOKEN_NUMBER;
		}
		else if (is_identifier_prefix(b)) {
			l.next_byte_while(is_identifier_infix);
			k = TOKEN_IDENT;
			if (l.peek_text() == "true")
				k = TOKEN_TRUE;
			if (l.peek_text() == "false")
				k = TOKEN_FALSE;
		}
	
		token		t = l.next_token(k);
		tokens.push(t);
	}
	token	eof = l.next_token(TOKEN_EOF);
	tokens.push(eof);
	return (true);
}

/* Value */

toml::value::value(void): kind(VALUE_INVALID), data() {
}
toml::value::~value(void) {
}

toml::value::value(value_number v): kind(VALUE_NUMBER) {
	data.number = v;
};
toml::value::value(value_boolean v): kind(VALUE_BOOLEAN) {
	data.boolean = v;
};
toml::value::value(value_string* v): kind(VALUE_STRING) {
	data.string = v;
};
toml::value::value(value_array* v): kind(VALUE_ARRAY) {
	data.array = v;
};
toml::value::value(value_table* v): kind(VALUE_TABLE) {
	data.table = v;
};

void	toml::value::free(void) {
	switch (kind) {
		case VALUE_STRING: {
			data.string->free();
			delete data.string;
		} break ;
		case VALUE_ARRAY: {
			value_array&	array = *data.array;
			for (u32 i = 0u; i < array.len; ++i)
				array[i].free();
			
			array.free();
			delete data.array;
		} break ;
		case VALUE_TABLE: {
			value_table&	table = *data.table;
			toml::value_table::iterator	iter(table);
			for (bool ok = iter.next(); ok; ok = iter.next()) {
				toml::value& value = iter.item->value;
				value.free();
			}
			table.free();
			delete data.table;
		} break ;
		case VALUE_NUMBER:
		case VALUE_BOOLEAN:
		case VALUE_INVALID:
		default: { } break ;
	}
}

/* Parser */
toml::parser::parser(toml::token_array& tokens, toml::value_table& document): tokens(tokens), document(&document), scope(&document), index(), curr(), ok(true) {
}
toml::parser::~parser(void) {
}

bool		toml::parser::end(void) const {
	return (index >= tokens.len || tokens[index].kind == TOKEN_EOF || tokens[index].kind == TOKEN_INVALID);
};
toml::token	toml::parser::peek(void) const {
	if (end()) return (token(TOKEN_EOF));
	return (tokens[index]);
};

toml::token	toml::parser::next(void) {
	curr = peek();
	if (!end())
		index++;
	return (curr);
};

u32			toml::parser::next_while(toml::token_kind delimiter) {
	u32			count = 0u;
	while (!end()) {
		token	t = peek();
		if (t.kind != delimiter) break ;
		else next();
	}
	return (count);
}

u32			toml::parser::next_while_not(toml::token_kind delimiter) {
	u32			count = 0u;
	while (!end()) {
		token	t = peek();
		if (t.kind == delimiter) break ;
		else next();
	}
	return (count);
}
u32			toml::parser::next_while_not(toml::parser::parser_predicate predicate) {
	u32			count = 0u;
	while (!end()) {
		token	t = peek();
		if (!predicate(t.kind)) break ;
		else next();
	}
	return (count);
}

bool		toml::parser::accept(toml::token_kind kind) {
	token	t = peek();
	if (t.kind == kind)
		return (next(), true);
	return (false);
}

bool		toml::parser::expect(toml::token_kind kind) {
	if (!accept(kind)) {
		token		fail_tok = peek();
		location	fail_loc = fail_tok.loc;
		std::cerr << TERMINAL_COLOR_WHITE "(todo):" << fail_loc.line << ':' << fail_loc.column << ": " TERMINAL_NOTICE_ERROR ": " TERMINAL_STYLE_RESET;
		std::cerr << "expected `" << kind << "`, got " << fail_tok.text << " instead" << std::endl;
		return (false);
	}
	return (true);
}
bool		toml::parser::expect(toml::token_kind* kinds, u64 kinds_n) {
	for (u64 i = 0ul; i < kinds_n; ++i)
		if (accept(kinds[i])) return (true);
	token		fail_tok = peek();
	location	fail_loc = fail_tok.loc;
	std::cerr << TERMINAL_COLOR_WHITE "(todo):" << fail_loc.line << ':' << fail_loc.column << ": " TERMINAL_NOTICE_ERROR ": " TERMINAL_STYLE_RESET;
	std::cerr << "expected any of [";
	for (u64 i = 0; i < kinds_n; ++i) {
		std::cerr << '`' << kinds[i] << '`';
		if (i + 1 < kinds_n) std::cerr << ',' << ' ';
	}
	std::cerr << "], got " << fail_tok.text << " instead" << std::endl;
	return (false);
}

bool		toml::parser::expect_recover(toml::token_kind expect_kind, toml::token_kind recover_kind) {
	if (!expect(expect_kind)) {
		// u32		restore_index = index;
		// token	restore_curr = curr;
		next_while_not(recover_kind);
		// if (end()) {
		// 	index = restore_index;
		// 	curr = restore_curr;
		// }
		return (false);
	}
	return (true);
}
bool		toml::parser::expect_recover(toml::token_kind* expect_kinds, u64 expect_kinds_n, toml::token_kind recover_kind) {
	if (!expect(expect_kinds, expect_kinds_n)) {
		// u32		restore_index = index;
		// token	restore_curr = curr;
		next_while_not(recover_kind);
		// if (end()) {
		// 	index = restore_index;
		// 	curr = restore_curr;
		// }
		return (false);
	}
	return (true);
}

bool		toml::parser::process(toml::token_array& tokens, toml::value_table& document) {
	parser	p(tokens, document);
	return (parse_document(p));
};

namespace toml {
	bool	parse_value_boolean(parser& p, value* val, const token& tok) {
		if (tok.kind == TOKEN_TRUE || tok.kind == TOKEN_FALSE) {
			if (tok.kind == TOKEN_TRUE) { val->data.boolean = true; }
			if (tok.kind == TOKEN_FALSE) { val->data.boolean = false; }
			return (true);
		} else return (p.ok = false);
	}
	bool	parse_value_number(parser& p, value* val, const token& tok) {
		if (tok.kind == TOKEN_NUMBER) {
			i64	number;
			if (!strconv::parse_i64(tok.text, number)) {
				location	fail_loc = tok.loc;
				std::cerr << TERMINAL_COLOR_WHITE "(todo):" << fail_loc.line << ':' << fail_loc.column << ": " TERMINAL_NOTICE_ERROR ": " TERMINAL_STYLE_RESET;
				std::cerr << "invalid number " << tok.text << std::endl;
				return (p.ok = false);
			}
			return (val->kind = VALUE_NUMBER, val->data.number = number, true);
		} else return (p.ok = false);
	}
	bool	parse_value_string(parser& p, value* val, const token& tok) {
		if (tok.kind == TOKEN_STRING) {
			string*	text = new string;
			*text = tok.text.clone();
			return (val->kind = VALUE_STRING, val->data.string = text, true);
		} else return (p.ok = false);
	}
	bool	parse_value_array(parser& p, value* val, const token& tok_open) {
		if (tok_open.kind == TOKEN_ARRAY_OPEN) {
			token			tok;
			value_array*	array = new value_array;

			bool			ok = p.ok;
			bool			comma_seen = true;
			while (!p.end()) {
				p.next_while(TOKEN_NEWLINE);

				tok = p.peek();
				if (p.end() || tok.kind == TOKEN_ARRAY_CLOSE) break ;
				while (comma_seen && p.accept(TOKEN_COMMA)) {
					token		fail_tok = p.curr;
					location	fail_loc = fail_tok.loc;

					std::cerr << TERMINAL_COLOR_WHITE "(todo):" << fail_loc.line << ':' << fail_loc.column << ": " TERMINAL_NOTICE_ERROR ": " TERMINAL_STYLE_RESET;
					std::cerr << "expected value before " << fail_tok.text << " token" << std::endl;
					ok = false;
				}
				p.next_while(TOKEN_NEWLINE);

				tok = p.peek();
				if (p.end() || tok.kind == TOKEN_ARRAY_CLOSE) break ;

				value	element_val;
				if (!parse_value(p, &element_val, TOKEN_ARRAY_CLOSE)) break ;
				array->push(element_val);

				comma_seen = p.accept(TOKEN_COMMA);
				if (!comma_seen) break ;
			}
			p.expect(TOKEN_ARRAY_CLOSE);
			*val = value(array);
			return (p.ok = (ok && p.ok));
		}
		return (p.ok = false);
	}
	bool	parse_value_table_inline(parser& p, value* val, const token& tok_open) {
		if (tok_open.kind == TOKEN_TABLE_INLINE_OPEN) {
			token			tok;
			value_table*	table = new value_table;

			bool			ok = p.ok;
			bool			comma_seen = true;
			while (!p.end()) {
				p.next_while(TOKEN_NEWLINE);

				tok = p.peek();
				if (p.end() || tok.kind == TOKEN_TABLE_INLINE_CLOSE) break ;
				while (comma_seen && p.accept(TOKEN_COMMA)) {
					token		fail_tok = p.curr;
					location	fail_loc = fail_tok.loc;

					std::cerr << TERMINAL_COLOR_WHITE "(todo):" << fail_loc.line << ':' << fail_loc.column << ": " TERMINAL_NOTICE_ERROR ": " TERMINAL_STYLE_RESET;
					std::cerr << "expected key-value pair before " << fail_tok.text << " token" << std::endl;
					ok = false;
				}
				p.next_while(TOKEN_NEWLINE);

				tok = p.peek();
				if (p.end() || tok.kind == TOKEN_TABLE_INLINE_CLOSE) break ;

				if (!p.expect_recover(TOKEN_IDENT, TOKEN_TABLE_INLINE_CLOSE)) break ; // TODO(xenobas): this skips all further entries until end of dict :/
				if (!parse_keyval(p, table)) break ;
				comma_seen = p.accept(TOKEN_COMMA);
				if (!comma_seen) break ;
			}
			p.expect(TOKEN_TABLE_INLINE_CLOSE);
			*val = value(table);
			return (p.ok = (ok && p.ok));
		} else return (p.ok = false);
	}
	bool	parse_value(parser& p, value* val, token_kind recover_kind = TOKEN_NEWLINE) {
		static token_kind	kinds[] = { TOKEN_ARRAY_OPEN, TOKEN_TABLE_INLINE_OPEN, TOKEN_NUMBER, TOKEN_STRING, TOKEN_TRUE, TOKEN_FALSE };
		static const u64	kinds_n = sizeof(kinds) / sizeof(token_kind);

		if (!p.expect_recover(kinds, kinds_n, recover_kind)) return (false);

		token	tok = p.curr;
		if (tok.kind == TOKEN_TRUE || tok.kind == TOKEN_FALSE) return (parse_value_boolean(p, val, tok));
		if (tok.kind == TOKEN_NUMBER) return (parse_value_number(p, val, tok));
		if (tok.kind == TOKEN_STRING) return (parse_value_string(p, val, tok));
		if (tok.kind == TOKEN_ARRAY_OPEN) return (parse_value_array(p, val, tok));
		if (tok.kind == TOKEN_TABLE_INLINE_OPEN) return (parse_value_table_inline(p, val, tok));
		return (false);
	}
	bool	parse_table_open(parser& p) {
		if (!p.expect_recover(TOKEN_IDENT, TOKEN_NEWLINE)) return (false);
		token	tok = p.curr;

		if (!p.expect_recover(TOKEN_ARRAY_CLOSE, TOKEN_NEWLINE)) return (false);
		if (!p.expect_recover(TOKEN_ARRAY_CLOSE, TOKEN_NEWLINE)) return (false);

		value_table&		document = *p.document;
		if (!document.has(tok.text)) {
			value_array*	array = new value_array;
			value			list(array);
			document.set(tok.text, list);
		}

		value&				list = document[tok.text];
		value_array&		array = *list.data.array;
		if (list.kind != VALUE_ARRAY) {
			std::cerr << TERMINAL_COLOR_WHITE "(todo):" << 1 << ':' << 1 << ": " TERMINAL_NOTICE_ERROR ": " TERMINAL_STYLE_RESET;
			std::cerr << "cannot append table to non-array value `" << list.kind << '`' << std::endl;
			return (p.ok = false);
		}
		value_table*		slot = new value_table();
		array.push(value(slot));
		return (p.scope = slot, true);
	}
	bool	parse_keyval(parser& p, value_table* scope = nullptr) {
		value	val;
		token	key = p.curr;

		if (!p.expect_recover(TOKEN_ASSIGN, TOKEN_NEWLINE)) return (false);
		parse_value(p, &val);
		if (val.kind != VALUE_INVALID) {
			if (scope == nullptr) scope = p.scope;
			if (scope->has(key.text)) {
				std::cerr << TERMINAL_COLOR_WHITE "(todo):" << 1 << ':' << 1 << ": " TERMINAL_NOTICE_ERROR ": " TERMINAL_STYLE_RESET;
				std::cerr << "overwriting already existing key " << key.text << " is disallowed" << std::endl;
				return (val.free(), p.ok = false);
			}
			scope->set(key.text, val);
			return (true);
		} else return (p.ok);
	}
	bool	parse_statement(parser& p) {
		static token_kind	prefix[] = { TOKEN_ARRAY_OPEN, TOKEN_IDENT, TOKEN_NEWLINE };
		static const u64	prefix_n = sizeof(prefix) / sizeof(token_kind);

		static token_kind	delimit[] = { TOKEN_NEWLINE, TOKEN_EOF }; // NOTE(xenobas): be very careful not to use TOKEN_EOF in expect_recover
		static const u64	delimit_n = sizeof(delimit) / sizeof(token_kind);

		if (!p.expect_recover(prefix, prefix_n, TOKEN_NEWLINE)) return (false);

		token	tok = p.curr;
		if (tok.kind == TOKEN_ARRAY_OPEN) {
			if (!p.expect_recover(TOKEN_ARRAY_OPEN, TOKEN_NEWLINE)) return (false);
			if (!parse_table_open(p)) return (false);
			return (p.expect(delimit, delimit_n));
		}
		else if (tok.kind == TOKEN_IDENT) {
			if (!parse_keyval(p)) return (false);
			return (p.expect(delimit, delimit_n));
		}
		return (false);
	}
	bool	parse_document(parser& p) {
		while (!p.end())
			parse_statement(p);
		return (p.ok);
	}

	bool	process(const string source, toml::token_array& tokens, toml::value_table* scope) {
		if (!toml::lexer::process(tokens, source)) return (false);
		return (toml::parser::process(tokens, *scope));
	}
};
