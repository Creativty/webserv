/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   toml.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xenobas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 21:52:08 by xenobas           #+#    #+#             */
/*   Updated: 2025/08/10 18:55:28 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "core.hpp"
#include "toml.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

TOML_Token::TOML_Token(void): tag(TOML_TOKEN_INVALID), error(TOML_TOKEN_ERROR_UNKNOWN), lexeme(""), line(1) { }
TOML_Token::TOML_Token(TOML_Token_Tag tag, const string_view& lexeme, u64 line, TOML_Token_Error error = TOML_TOKEN_ERROR_NONE): tag(tag), error(error), lexeme(lexeme), line(line) { }

TOML_Scanner::TOML_Scanner(string_view& source): source(source), line(1), offset(0), current(0) { }

static std::string	toml_load_file(string_view& path, bool *err) {
	std::string			path_std = path.string();
	std::ifstream		stream(path_std.c_str());
	if (!stream.is_open())
		return (*err = true, "couldn't load file");

	std::ostringstream	scontents;
	scontents << stream.rdbuf();
	return (*err = false, stream.close(), scontents.str());
}

TOML_Table	*toml_parse(TOML_Tokens& tokens);
TOML_Table	*toml_from_file(string_view& path) {
	bool		err    = false;
	std::string	contents = toml_load_file(path, &err);
	if (err) return (nullptr);

	string_view	source(contents);
	TOML_Tokens	tokens = toml_scan(source, path);

	bool		scan_error = false;
	for (u64 i = 0; i < tokens.size(); i++) {
		if (tokens[i].error != TOML_TOKEN_ERROR_NONE || tokens[i].tag == TOML_TOKEN_INVALID) {
			scan_error = true;
			std::cerr << tokens[i].line << ": " << tokens[i].tag << ' ' << string_view_fmt() << tokens[i].lexeme << std::endl;
			if (tokens[i].tag == TOML_TOKEN_INVALID)
				std::cerr << "\tError: Invalid token" << std::endl;
			else
				std::cerr << "\tError: " << tokens[i].error << std::endl;
		}
	}
	if (scan_error) return (nullptr);
	return (toml_parse(tokens));
}

static bool	is_digit_decimal(char ch) {
	return (ch >= '0' && ch <= '9');
}

static bool	is_digit_octal(char ch) {
	return (ch >= '0' && ch <= '7');
}

static bool	is_digit_binary(char ch) {
	return (ch == '0' || ch == '1');
}

static bool	is_digit_hexadecimal(char ch) {
	return ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F'));
}

TOML_Token	toml_scan_error(TOML_Scanner& scanner, TOML_Token_Tag tag, u64 line, TOML_Token_Error error) {
	string_view	lexeme = scanner.lexeme();
	while (scanner.peek()) {
		char	ch = scanner.peek();
		if (ch == '\n' || ch == '\t' || ch == ' ' || ch == '\r')
			break ;
		if (ch == '"' || ch == '#' || ch == ',' || ch == '=' || ch == ']' || ch == '[' || ch == '{' || ch == '}')
			break ;
		scanner.next();
	}
	return (TOML_Token(tag, lexeme, line, error));
}

bool		toml_scan_number(TOML_Scanner& scanner, bool (*is_digit)(char)) {
	bool	number_needed = false;
	while (scanner.peek()) {
		char	ch = scanner.peek();
		if (ch == '_') {
			if (number_needed)
				break ;
			number_needed = true;
		} else if (is_digit(ch)) {
			number_needed = false;
		} else {
			break ;
		}
		scanner.next();
	}
	return (!number_needed);
}

TOML_Token	toml_scan_token(TOML_Scanner& scanner) {
	u64		line = scanner.line;
	char	ch = scanner.next();
	if (ch == '=') {
		string_view	lexeme = scanner.lexeme();
		return (TOML_Token(TOML_TOKEN_EQUALS, lexeme, line));
	} else if (ch == ',') {
		string_view	lexeme = scanner.lexeme();
		return (TOML_Token(TOML_TOKEN_COMMA, lexeme, line));
	} else if (ch == '\r' && scanner.peek() == '\n') {
		scanner.next();
		string_view	lexeme = scanner.lexeme();
		return (TOML_Token(TOML_TOKEN_NEWLINE, lexeme, line));
	} else if (ch == '\n') {
		string_view	lexeme = scanner.lexeme();
		return (TOML_Token(TOML_TOKEN_NEWLINE, lexeme, line));
	} else if (ch == '[') {
		string_view	lexeme = scanner.lexeme();
		return (TOML_Token(TOML_TOKEN_SQUARE_OPEN, lexeme, line));
	} else if (ch == ']') {
		string_view	lexeme = scanner.lexeme();
		return (TOML_Token(TOML_TOKEN_SQUARE_CLOSE, lexeme, line));
	} else if (ch == '{') {
		string_view	lexeme = scanner.lexeme();
		return (TOML_Token(TOML_TOKEN_CURLY_OPEN, lexeme, line));
	} else if (ch == '}') {
		string_view	lexeme = scanner.lexeme();
		return (TOML_Token(TOML_TOKEN_CURLY_CLOSE, lexeme, line));
	} else if (ch == '#') {
		char	last_ch = scanner.peek();
		while (scanner.peek()) {
			if (scanner.peek() == '\n') {
				if (last_ch == '\r')
					scanner.current--;
				break ;
			}
			last_ch = scanner.peek();
			scanner.next();
		}
		string_view	lexeme = scanner.lexeme();
		return (TOML_Token(TOML_TOKEN_COMMENT, lexeme, line));
	} else if (ch >= 'a' && ch <= 'z') {
		while ((scanner.peek() >= 'a' && scanner.peek() <= 'z') || is_digit_decimal(scanner.peek()))
			scanner.next();
		TOML_Token_Tag	tag = TOML_TOKEN_IDENT;
		string_view		lexeme = scanner.lexeme();
		if (lexeme == "true")
			tag = TOML_TOKEN_TRUE;
		if (lexeme == "false")
			tag = TOML_TOKEN_FALSE;
		return (TOML_Token(tag, lexeme, line));
	} else if (is_digit_decimal(ch) || ch == '+' || ch == '-') {
		TOML_Token_Tag	tag = TOML_TOKEN_INTEGER;
		if (ch == '+' || ch == '-') {
			if (!is_digit_decimal(scanner.peek()))
				return (toml_scan_error(scanner, tag, line, TOML_TOKEN_ERROR_NUMBER_MISSING));
		}
		if (ch == '0' && scanner.peek() == 'x') { // Hexadecimal
			scanner.next();
			if (!toml_scan_number(scanner, is_digit_hexadecimal))
				return (toml_scan_error(scanner, tag, line, TOML_TOKEN_ERROR_NUMBER_MISSING));
		} else if (ch == '0' && scanner.peek() == 'o') {
			scanner.next();
			if (!toml_scan_number(scanner, is_digit_octal))
				return (toml_scan_error(scanner, tag, line, TOML_TOKEN_ERROR_NUMBER_MISSING));
		} else if (ch == '0' && scanner.peek() == 'b') {
			scanner.next();
			if (!toml_scan_number(scanner, is_digit_binary))
				return (toml_scan_error(scanner, tag, line, TOML_TOKEN_ERROR_NUMBER_MISSING));
		} else {
			if (!toml_scan_number(scanner, is_digit_decimal))
				return (toml_scan_error(scanner, tag, line, TOML_TOKEN_ERROR_NUMBER_MISSING));
			if (scanner.peek() == '.') {
				tag = TOML_TOKEN_FLOAT;
				scanner.next();
				toml_scan_number(scanner, is_digit_decimal);
			}
			if (scanner.peek() == 'e' || scanner.peek() == 'E') {
				tag = TOML_TOKEN_FLOAT;
				scanner.next();
				if (scanner.peek() == '-' || scanner.peek() == '+') {
					scanner.next();
					if (!is_digit_decimal(scanner.peek()))
						return (toml_scan_error(scanner, tag, line, TOML_TOKEN_ERROR_NUMBER_MISSING));
				}
				if (!toml_scan_number(scanner, is_digit_decimal))
					return (toml_scan_error(scanner, tag, line, TOML_TOKEN_ERROR_NUMBER_MISSING));
			}
		} 
		string_view	lexeme = scanner.lexeme();
		return (TOML_Token(tag, lexeme, line));
	} else if (ch == '"') {
		char	last_ch = scanner.peek();
		while (scanner.peek()) {
			char	ch = scanner.peek();
			if (scanner.peek() == '\n') {
				if (last_ch == '\r')
					scanner.current--;
				break ;
			}
			if (ch == '"')
				break ;
			last_ch = ch;
			scanner.next();
		}
		if (scanner.peek() == '"')
			scanner.next();
		else
			return (toml_scan_error(scanner, TOML_TOKEN_STRING, line, TOML_TOKEN_ERROR_UNTERMINATED));
		string_view	lexeme = scanner.lexeme();
		return (TOML_Token(TOML_TOKEN_STRING, lexeme, line));
	} else {
		string_view	lexeme = scanner.lexeme();
		return (TOML_Token(TOML_TOKEN_INVALID, lexeme, line));
	}
}

TOML_Tokens	toml_scan(string_view& source, string_view& path) {
	TOML_Tokens		tokens;
	TOML_Scanner	scanner(source);

	(void)path;
	while (!scanner.done()) {
		while (scanner.peek() == ' ' || scanner.peek() == '\t')
			scanner.next();
		scanner.offset = scanner.current;
		TOML_Token	token = toml_scan_token(scanner);
		tokens.push_back(token);
	}
	tokens.push_back(TOML_Token(TOML_TOKEN_EOF, "", scanner.line));
	return (tokens);
}

struct TOML_Parser {
	i64				current;
	i32				errors;
	TOML_Tokens&	tokens;
	TOML_Parser(TOML_Tokens& tokens);

	bool		done(void) const;
	TOML_Token	peek(void) const;
	TOML_Token	next(void);

	void		skip_unsemantic(void);
	void		skip_until_newline(void);
};

TOML_Parser::TOML_Parser(TOML_Tokens& tokens): current(0l), errors(0), tokens(tokens) { }
bool		TOML_Parser::done(void) const {
	if (cast(u64)current >= tokens.size()) return (true);
	if (tokens[cast(u64)current].tag == TOML_TOKEN_EOF) return (true);
	if (tokens[cast(u64)current].tag == TOML_TOKEN_INVALID) return (true);
	return (false);
}
TOML_Token	TOML_Parser::peek(void) const {
	if (done())
		return (TOML_Token());
	return (tokens[cast(u64)current]);
}
TOML_Token	TOML_Parser::next(void) {
	if (done())
		return (TOML_Token());
	return (tokens[cast(u64)current++]);
}

void		TOML_Parser::skip_unsemantic(void) {
	while (true) {
		TOML_Token	token = peek();
		if (token.tag != TOML_TOKEN_NEWLINE && token.tag != TOML_TOKEN_COMMENT)
			break ;
		next();
	}
}
void		TOML_Parser::skip_until_newline(void) {
	while (true) {
		TOML_Token	token = peek();
		if (token.tag == TOML_TOKEN_NEWLINE || token.tag == TOML_TOKEN_EOF || token.tag == TOML_TOKEN_INVALID)
			break ;
		next();
	}
}

TOML_Value::TOML_Value(void) {
	this->tag = TOML_TAG_INVALID;
	this->token = TOML_Token();
	this->data.string = nullptr;
}
TOML_Value::TOML_Value(TOML_Boolean value, TOML_Token token) {
	this->tag = TOML_TAG_BOOLEAN;
	this->token = token;
	this->data.boolean = value;
}
TOML_Value::TOML_Value(TOML_Integer value, TOML_Token token) {
	this->tag = TOML_TAG_INTEGER;
	this->token = token;
	this->data.i64 = value;
}
TOML_Value::TOML_Value(TOML_Float value, TOML_Token token) {
	this->tag = TOML_TAG_FLOAT;
	this->token = token;
	this->data.f64 = value;
}
TOML_Value::TOML_Value(TOML_String* value, TOML_Token token) {
	this->tag = TOML_TAG_STRING;
	this->token = token;
	this->data.string = value;
}
TOML_Value::TOML_Value(TOML_List* value, TOML_Token token) {
	this->tag = TOML_TAG_LIST;
	this->token = token;
	this->data.list = value;
}
TOML_Value::TOML_Value(TOML_Table* value, TOML_Token token) {
	this->tag = TOML_TAG_TABLE;
	this->token = token;
	this->data.table = value;
}

TOML_Value	toml_parse_value(TOML_Parser& parser) {
	(void)parser;
	TOML_Token	token = parser.next();
	if (token.tag == TOML_TOKEN_INTEGER)
		return (TOML_Value(cast(i64)0l, token));
	else if (token.tag == TOML_TOKEN_FLOAT)
		return (TOML_Value(cast(f64)0.0, token));
	else if (token.tag == TOML_TOKEN_SQUARE_OPEN)
		return (toml_parse_list(parser));
	else if (token.tag == TOML_TOKEN_CURLY_OPEN)
		return (toml_parse_table(parser));
	return (TOML_Value());
}

void		toml_parse_stuff(TOML_Parser& parser, TOML_Table* parent) {
	(void)parent;
	parser.skip_unsemantic();
	TOML_Token	token = parser.next();
	if (token.tag == TOML_TOKEN_EOF || token.tag == TOML_TOKEN_INVALID)
		return ;
	if (token.tag == TOML_TOKEN_IDENT) {
		TOML_Token	key = token;
		TOML_Token	equals = parser.next();
		if (equals.tag != TOML_TOKEN_EQUALS) {
			std::cerr << "TOML: Syntax Error: Expected \"=\", Got " << string_view_fmt() << equals.lexeme << " instead." << std::endl;
			std::cerr << "\tKey: " << string_view_fmt() << key.lexeme << std::endl;
			parser.errors++;
			parser.skip_until_newline();
			return ;
		}
		TOML_Value	value = toml_parse_value(parser);
	}
}

TOML_Table	*toml_parse(TOML_Tokens& tokens) {
	TOML_Parser	parser(tokens);

	TOML_Table	*table = new TOML_Table; // NOTE(XENOBAS): This table is the global scope, in other table is a sub table.
	while (!parser.done()) {
		toml_parse_stuff(parser, table);
		parser.next();
	}
	return (table);
}

bool		TOML_Scanner::done(void) const {
	return (current >= source.len);
}

string_view	TOML_Scanner::lexeme(void) const {
	return (source.slice(offset, current - offset));
}

char		TOML_Scanner::peek(void) {
	if (current >= source.len)
		return ('\0');
	return (source[cast(u64)current]);
}

char		TOML_Scanner::next(void) {
	char	c = source[cast(u64)current];
	if (c != '\0')
		current++;
	if (c == '\n')
		line++;
	return (c);
}

std::ostream&	operator<<(std::ostream& stream, TOML_Token_Tag	tag) {
	switch (tag) {
		case TOML_TOKEN_INVALID:
			return (stream << "TOML_TOKEN_INVALID");
		case TOML_TOKEN_EQUALS:
			return (stream << "TOML_TOKEN_EQUALS");
		case TOML_TOKEN_COMMA:
			return (stream << "TOML_TOKEN_COMMA");
		case TOML_TOKEN_NEWLINE:
			return (stream << "TOML_TOKEN_NEWLINE");
		case TOML_TOKEN_SQUARE_OPEN:
			return (stream << "TOML_TOKEN_SQUARE_OPEN");
		case TOML_TOKEN_SQUARE_CLOSE:
			return (stream << "TOML_TOKEN_SQUARE_CLOSE");
		case TOML_TOKEN_CURLY_OPEN:
			return (stream << "TOML_TOKEN_CURLY_OPEN");
		case TOML_TOKEN_CURLY_CLOSE:
			return (stream << "TOML_TOKEN_CURLY_CLOSE");
		case TOML_TOKEN_EOF:
			return (stream << "TOML_TOKEN_EOF");
		case TOML_TOKEN_IDENT:
			return (stream << "TOML_TOKEN_IDENT");
		case TOML_TOKEN_STRING:
			return (stream << "TOML_TOKEN_STRING");
		case TOML_TOKEN_INTEGER:
			return (stream << "TOML_TOKEN_INTEGER");
		case TOML_TOKEN_FLOAT:
			return (stream << "TOML_TOKEN_FLOAT");
		case TOML_TOKEN_COMMENT:
			return (stream << "TOML_TOKEN_COMMENT");
		case TOML_TOKEN_TRUE:
			return (stream << "TOML_TOKEN_TRUE");
		case TOML_TOKEN_FALSE:
			return (stream << "TOML_TOKEN_FALSE");
		default:
			return (stream << "TOML_TOKEN_UNREACHABLE");
	}
}

std::ostream&	operator<<(std::ostream& stream, TOML_Token_Error error) {
	switch (error) {
		case TOML_TOKEN_ERROR_UNKNOWN:
			return (stream << "Unexpected token");
		case TOML_TOKEN_ERROR_NUMBER_MISSING:
			return (stream << "Number component is missing");
		case TOML_TOKEN_ERROR_UNTERMINATED:
			return (stream << "Unterminated string");
		case TOML_TOKEN_ERROR_NONE:
			return (stream << "No error");
		default:
			return (stream << "Unknown error");
	}
}
