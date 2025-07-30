/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _toml.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/30 15:26:41 by aindjare          #+#    #+#             */
/*   UpdateG: 2025/07/30 16:41:25 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <map>
#include <vector>
#include <deque>
#include <string>
#include <ostream>

struct String_View {
	const char	*data;
	size_t		len;

	String_View(void) {
		this->data = 0;
		this->len = 0;
	}
	String_View(const String_View& view) {
		*this = view;
	}
	String_View(const char *data) {
		*this = data;
	}
	String_View(const char *data, size_t len) {
		if (data == 0) len = 0;

		this->len = len;
		this->data = data;
	}
	String_View(const std::string& str) {
		this->len = str.size();
		this->data = str.c_str();
	}
	String_View&	operator=(const char *data) {
		this->len = 0;
		this->data = data;
		if (data != 0) {
			while (data[this->len])
				this->len++;
		}
		return (*this);
	}
	String_View&	operator=(const String_View& view) {
		this->len = view.len;
		this->data = view.data;
		return (*this);
	}
	String_View&	operator=(const std::string& str) {
		this->len = str.size();
		this->data = str.c_str();
		return (*this);
	}
	char			operator[](const size_t index) const {
		if (index < 0 || index >= this->len)
			return ('\0');
		return (this->data[index]);
	}
	String_View		drop(size_t	count) const {
		if (count > this->len) return String_View();
		return (String_View(&this->data[count], this->len - count));
	}
	String_View		take(size_t	count) const {
		if (count > this->len) count = this->len;
		return (String_View(this->data, count));
	}

	bool	has_suffix(const String_View& suffix) const {
		if (suffix.len > this->len) return (false);
		for (size_t i = this->len - suffix.len, j = 0; i < suffix.len; i++, j++)
			if ((*this)[i] != suffix[j])
				return (false);
		return (true);
	}
	bool	has_prefix(const String_View& prefix) const {
		if (prefix.len > this->len) return (false);
		for (size_t i = 0; i < prefix.len; i++)
			if ((*this)[i] != prefix[i])
				return (false);
		return (true);
	}
};

namespace toml {
	struct	Location {
		String_View	path;
		size_t		line;
		size_t		column;
		size_t		index;
		Location(): path(0), line(1), column(1), index(0) { }
		Location(const Location &instance): path(instance.path), line(instance.line), column(instance.column), index(instance.index) { }
		Location&	operator=(const Location& instance) {
			this->path = instance.path;
			this->line = instance.line;
			this->column = instance.column;
			this->index = instance.index;
			return (*this);
		}
	};

	struct	Type;
	typedef	std::map<std::string, Type>	Table;
	typedef	std::vector<Type>			List;
	enum	Type_Tag {
		TYPE_INVALID,
		TYPE_TABLE,
		TYPE_LIST,
		TYPE_BOOLEAN,
		TYPE_INTEGER,
		TYPE_FLOAT,
	};
	struct	Type {
		Type_Tag	tag;
		union {
			Table*			table;
			List*			list;
			bool			boolean;
			unsigned long	i64;
			double			f64;
		}			data;
	};

	enum	Token_Tag {
		TOKEN_INVALID,
		TOKEN_DOT,
		TOKEN_COMMENT,
		TOKEN_NEWLINE,
		TOKEN_IDENTIFIER,
	};
	struct	Token {
		Token_Tag	tag;
		String_View	view;
		Location	location;
		Token(const Token_Tag tag): tag(tag) { };
		Token(const Token_Tag tag, const String_View& view, const Location& location): tag(tag), view(view), location(location) { };
	};
	typedef std::deque<Token>	Tokens;

	struct	Parser {
		Tokens		tokens;
		size_t		token_idx;
		std::string	path;
	};

	enum	Parse_Error_Tag {
		PARSE_ERROR_NONE,
		PARSE_ERROR_INVALID_TOKEN,
	};
	struct	Parse_Error {
		Location			location;
		Parse_Error_Tag		tag;
		Parse_Error() {
			this->tag = PARSE_ERROR_NONE;
		};
		Parse_Error(Parse_Error_Tag tag) {
			this->tag = tag;
		};
		Parse_Error(const Parse_Error& error) {
			this->tag = error.tag;
			this->location = error.location;
		};
	};

	Tokens		tokenize(const std::string& data);
	Parse_Error	parse_data(Table& table, const std::string& data);
	Parse_Error	parse_tokens(Table& table, Tokens& tokens);
}

std::ostream&		operator<<(std::ostream& stream, const String_View& view) {
	stream << "String_View\"";
	for (size_t i = 0; i < view.len; i++) {
		switch (view[i]) {
		case '\0':
			stream << "\\0";
			break;
		case '\"':
			stream << "\\\"";
			break;
		case '\'':
			stream << "\\\'";
			break;
		case '\n':
			stream << "\\n";
			break;
		case '\r':
			stream << "\\r";
			break;
		case '\t':
			stream << "\\t";
			break;
		case '\f':
			stream << "\\f";
			break;
		case '\v':
			stream << "\\v";
			break;
		case '\b':
			stream << "\\b";
			break;
		default:
			stream << view[i];
		}
	}
	return (stream << "\"");
}

std::ostream&		operator<<(std::ostream& stream, const toml::Location& location) {
	return (stream << location.path << ':' << location.line << ':' << location.column);
}

std::ostream&		operator<<(std::ostream& stream, const toml::Token_Tag& tag) {
	switch (tag) {
		case toml::TOKEN_INVALID:
			return (stream << "invalid");
		case toml::TOKEN_NEWLINE:
			return (stream << "newline");
		case toml::TOKEN_COMMENT:
			return (stream << "comment");
		case toml::TOKEN_DOT:
			return (stream << "dot");
		case toml::TOKEN_IDENTIFIER:
			return (stream << "identifier");
	}
}

std::ostream&		operator<<(std::ostream& stream, const toml::Token& token) {
	return (stream << "Token{ " << token.tag << ", " << token.location << ", " << token.view << " }");
}

toml::Tokens		toml::tokenize(const std::string& data) {
	toml::Tokens	tokens;
	toml::Location	location;
	String_View		view(data);

	while (view.len)
	{
		size_t	index = location.index;
		if (view.has_prefix("\r\n")) { // Windows CRLF
			tokens.push_front(toml::Token(toml::TOKEN_NEWLINE, view.take(2), location));
			location.line++;
			location.index += 2;
			location.column = 1;
		} else if (view[0] == '\n' || view[0] == '.') { // Unix LF
			toml::Token_Tag	tag;
			if (view[0] == '\n') tag = toml::TOKEN_NEWLINE;
			if (view[0] == '.') tag = toml::TOKEN_DOT;
			tokens.push_front(toml::Token(tag, view.take(1), location));
			location.line++;
			location.index++;
			location.column = 1;
		} else if (view[0] == '#') {
			size_t	count = 0;
			while (view[count]) {
				if (view[count] == '\r' && view[count + 1] == '\n') break ;
				if (view[count] == '\n') break;
				count ++;
			}
			tokens.push_front(toml::Token(toml::TOKEN_COMMENT, view.take(count), location));
			location.index += count;
			location.column += count;
		} else if (view[0] == ' ' || view[0] == '\t') {
			size_t	count = 0;
			while (view[count] == ' ' || view[count] == '\t')
				count ++;
			location.index += count;
			location.column += count;
		} else { // Invalid
			tokens.push_front(toml::Token(toml::TOKEN_INVALID, view.take(1), location));
			location.index++;
			location.column++;
		}
		view = view.drop(location.index - index);
	}
	return (tokens);
}

toml::Parse_Error	toml::parse_data(toml::Table& table, const std::string& data) {
	toml::Tokens	tokens = toml::tokenize(data);
	(void)tokens;
	(void)table;
	return toml::PARSE_ERROR_NONE;
}

toml::Parse_Error	toml::parse_tokens(toml::Table& table, toml::Tokens& tokens) {
	(void)tokens;
	(void)table;
	return toml::PARSE_ERROR_NONE;
}

#ifdef TEST_TOML
#include <iostream>
std::string	read_entire_file(const std::string& path, bool *ok);

int	main(int argc, char **argv) {
	if (argc == 1) {
		std::cerr << argv[0] << ": ERROR: required argument <file> is missing." << std::endl;
		return (1);
	}
	for (int i = 1; i < argc; i++) {
		String_View	arg(argv[i]);
		if (!arg.has_suffix(".toml")) {
			std::cerr << argv[0] << ": ERROR: Skipping `" << argv[i] << "` since the extension is not `toml`." << std::endl;
			continue ;
		}
		bool		ok;
		std::string	data = read_entire_file(argv[i], &ok);
		if (!ok) {
			std::cerr << argv[0] << ": ERROR: Could not read file `" << argv[i] << "`." << std::endl;
			continue ;
		}
		toml::Tokens	tokens = toml::tokenize(data);
		for (size_t i = 0; i < tokens.size(); i++)
			std::cout << tokens[i] << std::endl;
	}
	return (0);
}
#endif
