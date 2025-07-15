/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   toml.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 16:08:32 by aindjare          #+#    #+#             */
/*   Updated: 2025/07/15 17:29:12 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include <iostream>
#include <algorithm>

/* DISCLAIMER:
 * Our TOML subset, does not support UTF8, and heavily limits the freedom provided
 * by the original spec, therefore do not expect everything to match.
 * TOML is used not as a dynamic configuration language,
 * but as a structured data description format.
 */

toml::Token::Token() {
	std::string	str(nullptr, 0lu);
	this->text = str;
	this->kind = toml::TOKEN_INVALID;
	this->line = 1;
	this->column = 1;
}

toml::Token::Token(const char *text, int len, toml::Token_Kind kind, int line, int column) {
	std::string	str(text, (size_t)len);

	this->text = str;
	this->kind = kind;
	this->line = line;
	this->column = column;
}

std::ostream&	operator<<(std::ostream& stream, const toml::Token& token) {
	if (token.kind == toml::TOKEN_EOL)
		return (stream << "Token{ " << token.line << ":" << token.column << ": <eol> }");
	if (token.kind == toml::TOKEN_EOF)
		return (stream << "Token{ " << token.line << ":" << token.column << ": <eof> }");
	return (stream << "Token{ " << token.line << ":" << token.column << ": " << token.text << " }");
}

std::ostream&	operator<<(std::ostream& stream, const toml::Token_Kind& kind) {
	switch (kind) {
	case toml::TOKEN_INVALID:
		return (stream << "invalid");
	case toml::TOKEN_COMMENT:
		return (stream << "comment");
	case toml::TOKEN_EOL:
		return (stream << "end of line");
	case toml::TOKEN_EOF:
		return (stream << "end of file");
	case toml::TOKEN_SEPARATOR:
		return (stream << "comma");
	case toml::TOKEN_ASSIGN:
		return (stream << "equals");
	case toml::TOKEN_DICT_OPEN:
		return (stream << "dictionary open");
	case toml::TOKEN_DICT_CLOSE:
		return (stream << "dictionary close");
	case toml::TOKEN_TABLE_OPEN:
		return (stream << "table open");
	case toml::TOKEN_TABLE_CLOSE:
		return (stream << "table close");
	case toml::TOKEN_STRING:
		return (stream << "string");
	case toml::TOKEN_NUMBER:
		return (stream << "number");
	case toml::TOKEN_IDENTIFIER:
		return (stream << "identifier");
	default:
		return (stream << "unknown");
	}
}

/* Lexer */
toml::Tokens	toml::lex(const char *source) {
	toml::Tokens	tokens;
	int				i = 0, line = 1, column = 1;

	if (source == nullptr)
		return (tokens);
	while (source[i] != '\0') {
		while (source[i] == ' ' || source[i] == '\t') {
			column++;
			i++;
		}
		char				peek = source[i];
		toml::Token_Kind	kind = toml::TOKEN_INVALID;
		int					len  = 1;
		if (peek == '#') { // Comment
			kind = toml::TOKEN_COMMENT;
			len = 1;
			while (source[i + len] != '\0' && source[i + len] != '\n') {
				len++;
			}
		} else if (peek == '\n' || peek == ',' || peek == '=' || peek == '{' || peek == '}' || peek == '[' || peek == ']') {
			switch (peek) {
			case '\n': { kind = toml::TOKEN_EOL; } break ;
			case ',': { kind = toml::TOKEN_SEPARATOR; } break ;
			case '=': { kind = toml::TOKEN_ASSIGN; } break ;
			case '{': { kind = toml::TOKEN_DICT_OPEN; } break ;
			case '}': { kind = toml::TOKEN_DICT_CLOSE; } break ;
			case '[': { kind = toml::TOKEN_TABLE_OPEN; } break ;
			case ']': { kind = toml::TOKEN_TABLE_CLOSE; } break ;
			}
			len = 1;
		} else if (peek == '"') { // String
			/* NOTE(XENOBAS): 
			 * Supported quote escaping, means that API's that depend on strings must unescape them.
			 */
			kind = toml::TOKEN_STRING;
			len = 1;
			bool	terminated = false;
			for (bool escape = false; source[i + len]; len++) {
				if (source[i + len] == '"' && !escape) { terminated = true; len++; break; }
				else if (source[i + len] == '\n') break;
				else if (source[i + len] == '\\') escape = !escape;
				else escape = false;
			}
			if (!terminated) {
				if (source[i + len] == '\0')
					(void)0; // TODO(XENOBAS): Unterminated string found EOF
				if (source[i + len] == '\n')
					(void)0; // TODO(XENOBAS): Unsupported multiline string
			}
		} else if (peek >= '0' && peek <= '9') { // Number
			kind = toml::TOKEN_NUMBER;
			len = 1;
			while (source[i + len] && source[i + len] >= '0' && source[i + len] <= '9')
				len++;
		} else if (peek >= 'a' && peek <= 'z') { // Identifier
			kind = toml::TOKEN_IDENTIFIER;
			len = 1;
			while (source[i + len] && source[i + len] >= 'a' && source[i + len] <= 'z')
				len++;
		}
		tokens.elems.push_back(Token(&source[i], len, kind, line, column));
		for (int j = 0; j < len; j++) {
			if (source[i + j] == '\n') {
				line++;
				column = 1;
			} else column++;
		}
		i += len;
	}
	if (source[i] == '\0') {
		Token	token(&source[i], 1, TOKEN_EOF, line, column);
		tokens.elems.push_back(token);
	}
	std::reverse(tokens.elems.begin(), tokens.elems.end());
	return (tokens);
}

/* Parser */
#if 1
static toml::Token peek(toml::Tokens& tokens) {
	toml::Token	match(nullptr, 0, toml::TOKEN_INVALID, 1, 1);
	if (!tokens.elems.empty())
		match = tokens.elems.back();
	return (match);
}
#endif

static bool	error_expect(toml::Token_Kind kind, toml::Token token) {
	std::string	repr = token.text;
	std::cerr << "ERROR: Expected " << kind << " got " << token.kind;
	if (token.kind != toml::TOKEN_EOL && token.kind != toml::TOKEN_EOF)
		std::cerr << " \"" << token.text << "\"";
	std::cerr << "." << std::endl;
	if (token.kind != toml::TOKEN_INVALID)
		std::cerr << "\tconfig.toml:" << token.line << ":" << token.column << std::endl;
	return (false);
}

static bool	error_expect(const std::string& text, toml::Token token) {
	std::cerr << "ERROR: Expected " << toml::TOKEN_IDENTIFIER << " \"" << text << "\" got " << token.kind;
	if (token.kind != toml::TOKEN_EOL && token.kind != toml::TOKEN_EOF)
		std::cerr << " \"" << token.text << "\"";
	std::cerr << "." << std::endl;
	if (token.kind != toml::TOKEN_INVALID)
		std::cerr << "\tconfig.toml:" << token.line << ":" << token.column << std::endl;
	return (false);
}

#if 0
static bool	error(const std::string& text, toml::Token token = toml::Token()) {
	std::cerr << "ERROR: " << text << std::endl;
	if (token.kind != toml::TOKEN_INVALID)
		std::cerr << "\tconfig.toml:" << token.line << ":" << token.column << std::endl;
	return (false);
}
#endif

static bool accept(toml::Tokens& tokens, toml::Token* token, toml::Token_Kind kind) {
	if (tokens.elems.empty()) return (false);

	toml::Token& match = tokens.elems.back();
	if (token != nullptr) *token = match;
	if (match.kind != kind) return (false);
	if (match.kind != toml::TOKEN_EOF)
		tokens.elems.pop_back();
	return (true);
}

static bool accept(toml::Tokens& tokens, toml::Token_Kind kind) {
	if (tokens.elems.empty()) return (false);

	toml::Token& match = tokens.elems.back();
	if (match.kind != kind) return (false);
	if (match.kind != toml::TOKEN_EOF)
		tokens.elems.pop_back();
	return (true);
}

static bool accept(toml::Tokens& tokens, toml::Token* token, const std::string& text) {
	if (tokens.elems.empty()) return (false);

	toml::Token& match = tokens.elems.back();
	if (token != nullptr) *token = match;
	if (match.text != text) return (false);
	if (match.kind != toml::TOKEN_EOF)
		tokens.elems.pop_back();
	return (true);
}

static void	skip_eol_and_comments(toml::Tokens& tokens) {
	while (true) {
		if (!accept(tokens, nullptr, toml::TOKEN_COMMENT) && !accept(tokens, nullptr, toml::TOKEN_EOL))
			break ;
	}
}

static bool parse_instance_header(toml::Tokens& tokens) {
	toml::Token		token;
	if (accept(tokens, &token, toml::TOKEN_TABLE_OPEN)) {
		if (!accept(tokens, &token, toml::TOKEN_TABLE_OPEN)) return (error_expect(toml::TOKEN_TABLE_OPEN, token));
		if (!accept(tokens, &token, "server")) return (error_expect("server", token));
		if (!accept(tokens, &token, toml::TOKEN_TABLE_CLOSE)) return (error_expect(toml::TOKEN_TABLE_CLOSE, token));
		if (!accept(tokens, &token, toml::TOKEN_TABLE_CLOSE)) return (error_expect(toml::TOKEN_TABLE_CLOSE, token));
		accept(tokens, &token, toml::TOKEN_COMMENT);
		if (!accept(tokens, &token, toml::TOKEN_EOL)) return (error_expect(toml::TOKEN_EOL, token));
		return (true);
	}
	return (false);
}

/* NOTE(XENOBAS): This does not support table of composables (tables, dicts) */
bool	parse_table(toml::Tokens& tokens, std::vector<toml::Token>& table, toml::Token_Kind kind) {
	toml::Token	elem;
	toml::Token	token;

	if (!accept(tokens, &token, toml::TOKEN_TABLE_OPEN)) return (error_expect(toml::TOKEN_TABLE_OPEN, token));
	while (!tokens.elems.empty()) {
		skip_eol_and_comments(tokens);
		if (accept(tokens, &elem, kind)) {
			table.push_back(elem);
			if (!accept(tokens, toml::TOKEN_SEPARATOR)) break ;
		} else break ;
	}
	skip_eol_and_comments(tokens);
	if (!accept(tokens, &token, toml::TOKEN_TABLE_CLOSE)) return (error_expect(toml::TOKEN_TABLE_CLOSE, token)); // error: unterminated table
	return (true);
}

static bool parse_instance_entry(toml::Tokens& tokens, toml::Config& config, bool* err) {
	toml::Token	ident;
	toml::Token	token;

	(void)config;
	if (accept(tokens, &ident, toml::TOKEN_IDENTIFIER)) {
		if (!accept(tokens, &token, toml::TOKEN_ASSIGN)) return (*err = true, error_expect(toml::TOKEN_ASSIGN, token)); // error: expected = after key
		// TODO(XENOBAS): Check for duplicate entries
		if (ident.text == "port") {
			toml::Token	value; if (!accept(tokens, &value, toml::TOKEN_NUMBER)) return (*err = true, error_expect(toml::TOKEN_NUMBER, token)); // error: expected number for `port` key
			std::cout << "port " << value << std::endl;
			// TODO(XENOBAS): Shove it into config as a parsed integer between [0..65535]
		} else if (ident.text == "name") {
			toml::Token	value; if (!accept(tokens, &value, toml::TOKEN_STRING)) return (*err = true, error_expect(toml::TOKEN_STRING, token)); // error: expected string for `name`
			std::cout << "name " << value << std::endl;
			// TODO(XENOBAS): Shove it into config as a parsed string
		} else if (ident.text == "host") {
			std::vector<toml::Token>	table;
			if (!parse_table(tokens, table, toml::TOKEN_NUMBER)) return (*err = true, false);
			std::cout << "host { " << std::endl;
			for (size_t i = 0; i < table.size(); i++)
				std::cout << table[i] << std::endl;
			std::cout << "} " << std::endl;
			// TODO(XENOBAS): Shove it into config as a parse array of length 4 of numbers between [0..255]
		} else return (*err = true, error_expect("<key>", ident)); // error: unrecognized key
		accept(tokens, toml::TOKEN_COMMENT);
		if (!accept(tokens, &token, toml::TOKEN_EOL)) return (*err = true, error_expect(toml::TOKEN_EOL, token)); // error: expected eol delimiter
		return (true);
	} else {
		token = peek(tokens);
		if (token.kind == toml::TOKEN_TABLE_OPEN || token.kind == toml::TOKEN_EOF)
			return (false);
		return (error_expect(toml::TOKEN_IDENTIFIER, token));
	}
}

static bool parse_instance(toml::Tokens& tokens, toml::Configs& configs) {
	toml::Config	config;

	skip_eol_and_comments(tokens);
	if (!parse_instance_header(tokens)) return (false);

	bool error = false;
	while (!error) {
		skip_eol_and_comments(tokens);
		if (!parse_instance_entry(tokens, config, &error))
			break ;
	}
	if (error) return (false);
	return (configs.push_back(config), true);
}

toml::Configs	toml::parse(toml::Tokens& tokens) {
	toml::Configs	configs;

	while (!tokens.elems.empty()) {
		if (!parse_instance(tokens, configs))
			break ;
	}
	return (configs);
}

