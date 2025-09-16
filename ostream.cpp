#include <ostream>

#include "types.hpp"
std::ostream&	operator<<(std::ostream& stream, const type::string& str) {
	if (str.data == nullptr) return (stream << "(null)");

	stream << '`';
	for (u32 i = 0u; i < str.len; ++i) {
		switch (str.data[i]) {
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
				stream << str.data[i];
			} break;
		}
	}
	stream << '\'';
	return (stream);
}

#include "toml.hpp"
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
			toml::value_table::iterator	iter = toml::value_table::iterator(*value.data.table);
			stream << "value::table{ ";
			for (bool ok = iter.next(); ok;) {
				const toml::string& key = iter.item->key;
				const toml::value& value = iter.item->value;
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
