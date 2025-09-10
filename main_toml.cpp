#include <ostream>
#include <iostream>

#include <stdint.h>
#define nullptr 0

#define TERMINAL_COLOR_RED "\x1b[1;31m"
#define TERMINAL_COLOR_GREEN "\x1b[1;32m"
#define TERMINAL_COLOR_WHITE "\x1b[1;37m"
#define TERMINAL_COLOR_YELLOW "\x1b[1;33m"
#define TERMINAL_STYLE_RESET "\x1b[0m"

#define TERMINAL_NOTICE_ERROR TERMINAL_COLOR_RED "error"

/// TYPES
typedef uint8_t		byte;

typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef uint64_t	u64;

typedef int8_t		i8;
typedef int16_t		i16;
typedef int32_t		i32;
typedef int64_t		i64;

namespace type {
	typedef const char*	cstring;
	u32	cstring_len(cstring str) {
		if (str == NULL) return (0u);

		u32	len = 0;
		while (str[len]) len++;
		return (len);
	}

	struct string {
		byte*	data;
		u32		len;
		
		string(void): data(nullptr), len(0u) { };
		string(cstring other): data((byte*)other), len(cstring_len(other)) { };
		string(cstring other, u32 len): data((byte*)other), len(len) {
			u32	other_len = cstring_len(other);
			if (other_len < len) len = other_len;
		};
		string(byte* other, u32 len): data(other), len(len) { };
		string(const string& other): data(other.data), len(other.len) { };
		~string(void) { };

		i32		find_index(byte needle) const {
			if (needle == '\0' || data == nullptr || len == 0u) return (-1);
			for (i32 i = 0; (u32)i < len; i++)
				if (data[i] == needle) return (i);
			return (-1);
		}
		i32		find_index(const string& needle) const {
			if (needle.len > len) return (-1);
			for (i32 i = 0; (u32)i < len; i++) {
				u32		delta = 0;
				bool	match = true;
				while (delta < needle.len && (u32)i + delta < len && match) {
					match = (needle[delta] == data[(u32)i + delta]);
					delta++;
				}
				if (delta == needle.len && match) return (i);
			}
			return (-1);
		}
		bool	contains(const string& needle) const {
			return (find_index(needle) >= 0);
		}
		string	slice(u32 start, u32 end) const {
			if (end < start) end = start;
			u32		slice_len = end - start;
			byte*	slice_data = (start > len) ? nullptr : &data[start];
			return (string(slice_data, slice_len));
		}
		string	slice(u32 start) const {
			return (slice(start, len));
		}

		string	clone(void) const {
			string	copy(*this);
			copy.data = new byte[copy.len]();
			for (u32 i = 0u; i < len * (u32)sizeof(char); ++i)
				copy.data[i] = data[i];
			return (copy);
		}
		void	free(void) {
			delete[] data;

			len = 0u;
			data = nullptr;
		}

		operator	bool() const {
			if (data == nullptr) return (false);
			if (len == 0u) return (false);
			return (true);
		};
		string&		operator=(const string& other) {
			if (&other != this) {
				data = other.data;
				len = other.len;
			}
			return (*this);
		};
		string&		operator=(cstring other) {
			*this = string(other);
			return (*this);
		};
		byte&		operator[](u64 index) {
			if (index >= (u64)len) throw std::runtime_error("Out of bounds string access");
			return (data[index]);
		}
		const byte&	operator[](u64 index) const {
			if (index >= (u64)len) throw std::runtime_error("Out of bounds string access");
			return (data[index]);
		}

		bool		operator==(cstring rhs) const {
			return (*this == string(rhs));
		}
		bool		operator==(const string& rhs) const {
			return (rhs.len == len && find_index(rhs) == 0);
		}
	};
	
	template <typename V>
	struct dynamic_array {
		byte*	items;
		u32		len;
		u32		cap;

		dynamic_array(void): items(nullptr), len(0u), cap(0u) { };
		~dynamic_array(void) { };
		dynamic_array(const dynamic_array& other): items(other.items), len(other.len), cap(other.cap) { };
		dynamic_array&	operator=(const dynamic_array& other) {
			if (this != &other) {
				items = other.items;
				len = other.len;
				cap = other.cap;
			}
			return (*this);
		}
		V&			operator[](u64 index) {
			return (((V*)items)[index]);
		}
		const V&	operator[](u64 index) const {
			return (((V*)items)[index]);
		}

		void	clear(void) {
			len = 0u;
		}
		void	free(void) {
			delete[] items;

			len = 0u;
			cap = 0u;
			items = nullptr;
		}
		void	resize(u32 cap_new) {
			byte*	items_old = items;
			byte*	items_new = new byte[cap_new * sizeof(V)]();
			for (u32 i = 0; i < cap * (u32)sizeof(V); ++i)
				items_new[i] = items[i];
			
			cap = cap_new;
			items = items_new;
			delete[] items_old;
		}
		V&	push(const V& item) {
			if (len >= cap) resize((cap > 0u) ? (cap * 2u) : 8u);
			V&	value = ((V*)items)[len];
			return (++len, value = item);
		}
	};

	template <typename V>
	struct hash_map {
		struct item {
			string	key;
			V		value;
		};
		struct iterator {
			hash_map&	parent;
			item*		item_curr;
			u32			index;
			iterator(hash_map& parent): parent(parent), item_curr(nullptr), index(0u) { };
			~iterator(void) { }
			bool	next(void) {
				while (index < (size_t)parent.items.cap) {
					size_t	i = index++;
					if ((bool)parent.items[i].key) {
						item_curr = &parent.items[i];
						return (true);
					}
				}
				return (false);
			}
		};
		static u64 fnv_1a(const string& key) {
			u64	hash = 14695981039346656037ul;
			for (u32 i = 0; i < key.len; i++) {
				hash ^= (u64)(byte)(key[i]);
				hash ^= 1099511628211ul;
			}
			return (hash);
		}

		dynamic_array<item>	items;
		u32					count;
		hash_map(void): items(), count(0u) { };
		~hash_map(void) { };
		hash_map(const hash_map& other): items(other.items), count(other.count) { };

		hash_map&	operator=(const hash_map& other) {
			if (this != &other) {
				items = other.items;
				count = other.count;
			}
			return (*this);
		}
		V&			operator[](const string& key) {
			if (count == 0) throw std::runtime_error("empty hash_map");
			unsigned long	hash = fnv_1a(key);
			u64				index = (u64)(hash & ((u64)items.cap - 1ul));
			while ((bool)items[index].key) {
				if (items[index].key == key)
					return (items[index].value);
				if (++index >= (u64)items.cap) index = 0u;
			}
			throw std::runtime_error("invalid key");
		}
		const V&	operator[](const string& key) const {
			if (count == 0) throw std::runtime_error("empty hash_map");
			unsigned long	hash = fnv_1a(key);
			u64				index = (u64)(hash & ((u64)items.cap - 1ul));
			while ((bool)items[index].key) {
				if (items[index].key == key)
					return (items[index].value);
				if (++index >= (u64)items.cap) index = 0u;
			}
			throw std::runtime_error("invalid key");
		}
		void		resize(u32 cap_new) {
			items.resize(cap_new);
		}
		void		free(void) {
			iterator	iter(*this);
			for (bool ok = iter.next(); ok; ok = iter.next()) {
				item&	item = *iter.item_curr;
				item.key.free();
			}
			count = 0u;

			items.free();
		}
		iterator	iter(void) {
			return (iterator(*this));
		};
		V&			set(const string& key, const V& value) {
			if (!(bool)key) throw std::runtime_error("invalid key");
			if (count >= items.cap / 2) resize((items.cap == 0u) ? 8u : (items.cap * 2u));

			u64	hash = fnv_1a(key);
			u64	index = (u64)(hash & ((u64)items.cap - 1ul));
			while ((bool)(items[index].key)) {
				if (items[index].key == key) {
					items[index].value = value;
					return (items[index].value);
				}
				if (++index >= items.cap) index = 0u;
			}
			items[index] = (item){
				.key = key.clone(),
				.value = value,
			};
			count++;
			return (items[index].value);
		};

		bool		has(const string& key) const {
			if (count == 0u) return (false);

			unsigned long	hash = fnv_1a(key);
			u64				index = (u64)(hash & ((u64)items.cap - 1ul));
			while ((bool)items[index].key) {
				if (items[index].key == key)
					return (true);
				if (++index >= (u64)items.cap) index = 0u;
			}
			return (false);
		}
	};
};
std::ostream&	operator<<(std::ostream& stream, const type::string& string) {
	if (string.data == nullptr) return (stream << "(null)");

	stream << '"';
	for (u32 i = 0u; i < string.len; ++i) {
		switch (string.data[i]) {
			case '\a': {
				stream << "\\a";
			} break;
			case '\b': {
				stream << "\\b";
			} break;
			case '\t': {
				stream << "\\t";
			} break;
			case '\n': {
				stream << "\\n";
			} break;
			case '\0': {
				stream << "\\0";
			} break;
			default: {
				stream << string.data[i];
			} break;
		}
	}
	stream << '"';
	return (stream);
}

namespace strconv {
	bool	parse_i64(const type::string& string, i64& n) {
		n = 0l;
		i64	s = 1l;
		u32	i = 0ul;
		if (!(bool)string) return (false);
		if (string.len > 0 && (string[i] == '-' || string[i] == '+')) {
			if (string[i] == '-')
				s = -1l;
			i++;
		}
		while (i < string.len) {
			byte	digit = string[i];
			if (digit < '0' || digit > '9') break ;
			n = (n * 10l) + (i64)(digit - '0');
			i++;
		}
		bool ok = (string.data != nullptr && i == string.len);
		n = ok ? n * s : 0l;
		return (ok);
	}
};

/// TOML: Forward declarations
namespace toml {
	// NOTE(xenobas): `POV: you cannot forward declare enums in C...`
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
	enum value_kind {
		VALUE_INVALID,

		VALUE_NUMBER,
		VALUE_BOOLEAN,
		VALUE_STRING,
		VALUE_ARRAY,
		VALUE_TABLE,
	};

	struct token;
	struct value;
	struct parser;

	typedef i64							value_number;
	typedef bool						value_boolean;
	typedef type::string				value_string;
	typedef type::dynamic_array<value>	value_array;
	typedef type::hash_map<value>		value_table;

	bool	parse_value(parser& p, value* val, token_kind recover_kind);
	bool	parse_key_value(parser& p, value_table* scope);
	bool	parse_document(parser& p);
};
std::ostream&	operator<<(std::ostream& stream, const toml::token_kind& kind);
std::ostream&	operator<<(std::ostream& stream, const toml::value_kind& kind);
std::ostream&	operator<<(std::ostream& stream, const toml::token& token);
std::ostream&	operator<<(std::ostream& stream, const toml::value& value);

// TOML: Implementation
namespace toml {
	struct token;
	typedef type::string				string;
	typedef type::dynamic_array<token>	dynamic_tokens;

	byte	to_lower(byte b) {
		return ((b >= 'A' && b <= 'Z') ? (b ^ 32) : b);
	};
	bool	is_digit(byte d) {
		return (d >= '0' && d <= '9');
	};
	bool	is_alphabet(byte b) {
		byte	a = to_lower(b);
		return (a >= 'a' && a <= 'z');
	};
	bool	is_newline(byte n) {
		return (n == '\n');
	}
	bool	is_string_delimiter(byte d) {
		return (is_newline(d) || d == '"');
	}
	bool	is_whitespace(byte w) {
		return (w == ' ' || w <= '\t');
	};
	bool	is_identifier_prefix(byte b) {
		return (is_alphabet(b) || b == '_');
	};
	bool	is_identifier_infix(byte b) {
		return (is_identifier_prefix(b) || is_digit(b));
	};

	struct location {
		u32	index;
		u32	line;
		u32	column;
		location(void): index(0u), line(1u), column(1) { };
		location(u32 index, u32 line, u32 column): index(index), line(line), column(column) { };
		location(const location& other): index(other.index), line(other.line), column(other.column) { };
		location&	operator=(const location& other) {
			if (this != &other) {
				index = other.index;
				line = other.line;
				column = other.column;
			}
			return (*this);
		};

		void		advance(byte b) {
			if (b == '\0') return ;

			index++;
			if (b != '\n') column++;
			else {
				line++;
				column = 1u;
			}
		}
	};

	struct token {
		string		text;
		token_kind	kind;
		location	loc;
		token(void): text(), kind(TOKEN_INVALID), loc() { };
		token(token_kind kind): text(), kind(kind), loc() { };
		token(string text, token_kind kind, const location& loc): text(text), kind(kind), loc(loc) { };
		token(const token& other): text(other.text), kind(other.kind), loc(other.loc) { };
		token&	operator=(const token& other) {
			if (this != &other) {
				text = other.text;
				kind = other.kind;
				loc = other.loc;
			}
			return (*this);
		};
		~token(void) { };
	};
	struct lexer {
		const string	source;
		location		loc_curr;
		location		loc_last;
		u32				line_begin_index;
		lexer(const string source): source(source), loc_curr(), loc_last(), line_begin_index() { };
		~lexer(void) { };
		bool	end(void) const {
			return (loc_curr.index >= source.len);
		}
		byte	peek_byte(void) const {
			if (loc_curr.index >= source.len) return (0);
			return (source[loc_curr.index]);
		};
		byte	next_byte(void) {
			byte	b = peek_byte();
			loc_curr.advance(b);
			if (b == '\n') line_begin_index = loc_curr.index;
			return (b);
		};
		string	peek_text(void) {
			return (source.slice(loc_last.index, loc_curr.index));
		}
		string	next_text(void) {
			string	text = peek_text();
			return (loc_last = loc_curr, text);
		}
		token	next_token(token_kind kind) {
			location	loc(loc_last);
			return (token(next_text(), kind, loc));
		}
		u32		next_byte_while(bool (*predicate)(byte)) {
			u32			count = 0u;
			while (!end()) {
				byte	b = peek_byte();
				if (!predicate(b)) break ;
				next_byte();
			}
			return (count);
		};
		u32		skip_byte_while(bool (*predicate)(byte)) {
			u32	count = next_byte_while(predicate);
			return (next_text(), count);
		}
		u32		next_byte_while_not(bool (*predicate)(byte)) {
			u32			count = 0u;
			while (!end()) {
				byte	b = peek_byte();
				if (predicate(b)) break ;
				next_byte();
			}
			return (count);
		};
		u32		skip_byte_while_not(bool (*predicate)(byte)) {
			u32	count = next_byte_while_not(predicate);
			return (next_text(), count);
		}
		void	report_error(const char* note) {
			std::cerr << TERMINAL_COLOR_WHITE "(todo):" << loc_last.line << ':' << loc_last.column << ": " TERMINAL_NOTICE_ERROR ": " TERMINAL_STYLE_RESET << note << std::endl;

			i32		line_end_index = source.slice(line_begin_index).find_index('\n');
			if (line_end_index == -1) line_end_index = (i32)source.len;
			string	line = source.slice(line_begin_index, line_end_index);
			std::cerr << "\t|\t";
			for (u32 i = 0; i < line.len; ++i) {
				byte	b = line[i];
				if (is_newline(b)) std::cerr << std::endl << "\t|\t" << line << std::endl;
				else std::cerr << b;
			}
			std::cerr << std::endl;
		}

		static bool	process(dynamic_tokens& tokens, const string source) {
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
	};

	struct value {
		value_kind	kind;
		union {
			value_number	number;
			value_boolean	boolean;
			value_string*	string;
			value_array*	array;
			value_table*	table;
		}			data;
		value(void): kind(VALUE_INVALID), data() { };
		value(value_number number): kind(VALUE_NUMBER) { data.number = number; };
		value(value_boolean boolean): kind(VALUE_BOOLEAN) { data.boolean = boolean; };
		value(value_string* string): kind(VALUE_STRING) { data.string = string; };
		value(value_array* array): kind(VALUE_ARRAY) { data.array = array; };
		value(value_table* table): kind(VALUE_TABLE) { data.table = table; };
		~value() { };

		void	free(void) {
			if (kind == VALUE_STRING) {
				data.string->free();
				delete data.string;
			}
			else if (kind == VALUE_ARRAY) {
				value_array&	array = *data.array;
				for (u32 i = 0u; i < array.len; ++i)
					array[i].free();
				array.free();
				delete data.array;
			}
			else if (kind == VALUE_TABLE) {
				value_table&	table = *data.table;
				toml::value_table::iterator	iter(table);
				for (bool ok = iter.next(); ok; ok = iter.next()) {
					toml::value& value = iter.item_curr->value;
					value.free();
				}
				table.free();
				delete data.table;
			}
		};
	};

	struct parser {
		dynamic_tokens&	tokens;
		value_table*	document;
		value_table*	scope;
		u32				index;
		token			curr;
		bool			ok;

		parser(dynamic_tokens& tokens, value_table& document): tokens(tokens), document(&document), scope(&document), index(0u), curr(), ok(true) { };
		~parser(void) { };

		bool	end(void) const {
			return (index >= tokens.len || tokens[index].kind == TOKEN_EOF || tokens[index].kind == TOKEN_INVALID);
		};
		token	peek(void) const {
			if (end()) return (token(TOKEN_EOF));
			return (tokens[index]);
		};
		token	next(void) {
			curr = peek();
			if (end()) return (curr);
			return (++index, curr);
		};
		u32		next_while(token_kind delimiter) {
			u32			count = 0u;
			while (!end()) {
				token	t = peek();
				if (t.kind != delimiter) break ;
				else next();
			}
			return (count);
		}
		u32		next_while_not(token_kind delimiter) {
			u32			count = 0u;
			while (!end()) {
				token	t = peek();
				if (t.kind == delimiter) break ;
				else next();
			}
			return (count);
		}
		u32		next_while_not(bool	(*predicate)(token_kind)) {
			u32			count = 0u;
			while (!end()) {
				token	t = peek();
				if (!predicate(t.kind)) break ;
				else next();
			}
			return (count);
		}
		bool	accept(token_kind kind) {
			token	t = peek();
			if (t.kind == kind)
				return (next(), true);
			return (false);
		}
		bool	expect(token_kind kind) {
			if (!accept(kind)) {
				token		fail_tok = peek();
				location	fail_loc = fail_tok.loc;
				std::cerr << TERMINAL_COLOR_WHITE "(todo):" << fail_loc.line << ':' << fail_loc.column << ": " TERMINAL_NOTICE_ERROR ": " TERMINAL_STYLE_RESET;
				std::cerr << "expected `" << kind << "`, got " << fail_tok.text << " instead" << std::endl;
				return (false);
			}
			return (true);
		}
		bool	expect(token_kind* kinds, u64 kinds_n) {
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
		bool	expect_recover(token_kind expect_kind, token_kind recover_kind) {
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
		bool	expect_recover(token_kind* expect_kinds, u64 expect_kinds_n, token_kind recover_kind) {
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

		static bool	process(dynamic_tokens& tokens, value_table& document) {
			parser	p(tokens, document);
			return (parse_document(p));
		};
	};

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
				if (!parse_key_value(p, table)) break ;
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
	bool	parse_key_value(parser& p, value_table* scope = nullptr) {
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
			if (!parse_key_value(p)) return (false);
			return (p.expect(delimit, delimit_n));
		}
		return (false);
	}
	bool	parse_document(parser& p) {
		while (!p.end())
			parse_statement(p);
		return (p.ok);
	}

	bool	process(const string source, toml::dynamic_tokens& tokens, toml::value_table* scope) {
		if (!toml::lexer::process(tokens, source)) return (false);
		return (toml::parser::process(tokens, *scope));
	}
};
std::ostream&	operator<<(std::ostream& stream, const toml::token_kind& kind) {
	switch (kind) {
		case toml::TOKEN_INVALID: return (stream << "token::invalid");
		case toml::TOKEN_ASSIGN: return (stream << "token::assign");
		case toml::TOKEN_COMMA: return (stream << "token::comma");
		case toml::TOKEN_NEWLINE: return (stream << "token::newline");
		case toml::TOKEN_TABLE_INLINE_OPEN: return (stream << "token::table_inline_open");
		case toml::TOKEN_TABLE_INLINE_CLOSE: return (stream << "token::table_inline_close");
		case toml::TOKEN_ARRAY_OPEN: return (stream << "token::array_open");
		case toml::TOKEN_ARRAY_CLOSE: return (stream << "token::array_close");
		case toml::TOKEN_STRING: return (stream << "token::string");
		case toml::TOKEN_NUMBER: return (stream << "token::number");
		case toml::TOKEN_IDENT: return (stream << "token::identifier");
		case toml::TOKEN_TRUE: return (stream << "token::true");
		case toml::TOKEN_FALSE: return (stream << "token::false");
		case toml::TOKEN_EOF: return (stream << "token::eof");
		default: return (stream << "token::unknown(" << (i32)kind << ')');
	}
}
std::ostream&	operator<<(std::ostream& stream, const toml::value_kind& kind) {
	switch (kind) {
		case toml::VALUE_INVALID: return (stream << "value::invalid");
		case toml::VALUE_NUMBER: return (stream << "value::number");
		case toml::VALUE_BOOLEAN: return (stream << "value::boolean");
		case toml::VALUE_STRING: return (stream << "value::string");
		case toml::VALUE_ARRAY: return (stream << "value::array");
		case toml::VALUE_TABLE: return (stream << "value::table");
		default: return (stream << "value::unknown");
	}
}
std::ostream&	operator<<(std::ostream& stream, const toml::token& token) {
	return (stream << '{' << ' ' << token.kind << ',' << ' ' << token.text << ' ' << '}');
}
std::ostream&	operator<<(std::ostream& stream, const toml::value& value) {
	switch (value.kind) {
		case toml::VALUE_INVALID: return (stream << "value::invalid");
		case toml::VALUE_NUMBER: return (stream << "value::number(" << value.data.number << ')');
		case toml::VALUE_BOOLEAN: return (stream << "value::boolean(" << (value.data.boolean ? "true" : "false") << ')');
		case toml::VALUE_STRING: return (stream << "value::string(" << *value.data.string << ')');
		case toml::VALUE_ARRAY: {
			stream << "value::array[";
			for (u32 i = 0u; i < value.data.array->len; ++i) {
				stream << (*value.data.array)[i];
				if (i + 1 < value.data.array->len)
					stream << ',' << ' ';
			}
			stream << ']';
			return (stream);
		} break ;
		case toml::VALUE_TABLE: {
			toml::value_table::iterator	iter = value.data.table->iter();
			stream << "value::table{ ";
			for (bool ok = iter.next(); ok;) {
				const toml::string& key = iter.item_curr->key;
				const toml::value& value = iter.item_curr->value;
				stream << key << " = " << value;

				ok = iter.next();
				if (ok) stream << ',' << ' ';
			}
			stream << ' ' << '}';
			return (stream);
		} break ;
		default: return (stream << "value::unknown");
	}
}

i32	main(i32 argc, type::cstring* argv) {
	((void)argc, (void)argv);
	const toml::string		source = "[[webserv]]";
	toml::dynamic_tokens	tokens;
	toml::value				scope(new toml::value_table);
	if (!toml::process(source, tokens, scope.data.table))
		std::cerr << "An error has occurred during parsing..." << std::endl;
	std::cout << scope << std::endl;
	scope.free();
	tokens.free();
	return (0);
}
