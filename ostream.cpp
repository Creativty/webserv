#include "types.hpp"
#include <ostream>

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
