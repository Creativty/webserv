#include "core.hpp"
#include "string_view.hpp"

bool	stream_literal = false;

static size_t cstr_len(const char *str) {
	size_t len = 0;
	while (str[len] != '\0')
		++len;
	return (len);
}

static bool cstr_equal(const char *lhs, const char *rhs, size_t len) {
	for (size_t i = 0; i < len; ++i) {
		if (lhs[i] != rhs[i])
			return (false);
	}
	return (true);
}

string_view::string_view(): data(nullptr), len(0) { }
string_view::string_view(const char* str): data(str), len(cstr_len(str)) { }
string_view::string_view(const char* str, size_t len): data(str), len(len) { }
string_view::string_view(const std::string& str): data(str.c_str()), len(str.length()) { }
string_view::string_view(const string_view& sv): data(sv.data), len(sv.len) { };

char	string_view::operator[](size_t index) const {
	if (index < this->len)
		return (this->data[index]);
	return ('\0');
}
string_view& string_view::operator=(const char* str) {
	this->data = str;
	this->len = cstr_len(str);
	return (*this);
}
string_view& string_view::operator=(const std::string& str) {
	this->data = str.c_str();
	this->len = str.length();
	return (*this);
}
string_view& string_view::operator=(const string_view& other) {
	this->len = other.len;
	this->data = other.data;
	return (*this);
}
bool string_view::operator==(const string_view& other) const {
	if (this->len != other.len)
		return (false);
	return cstr_equal(this->data, other.data, this->len);
}
bool string_view::operator!=(const string_view& other) const {
	return (!(*this == other));
}

std::ostream&	operator<<(std::ostream& stream, string_view sv) {
	if (stream_literal) {
		stream << '"';
		for (size_t i = 0; i < sv.len; i++) {
			if (sv[i] == '\n')
				stream << "\\n";
			else if (sv[i] == '\b')
				stream << "\\b";
			else if (sv[i] == '\t')
				stream << "\\t";
			else if (sv[i] == '\v')
				stream << "\\v";
			else if (sv[i] == '\r')
				stream << "\\r";
			else
				stream << sv[i];
		}
		stream << '"';
		stream_literal = false;
	} else {
		for (size_t i = 0; i < sv.len; i++)
			stream << sv[i];
	}
	return (stream);
}

std::string	string_view_fmt(void) {
	stream_literal = true;
	return ("");
}

string_view	string_view::slice(u64 offset, u64 count) const {
	if (offset >= len) return (string_view());
	if (count > len) count = len - offset;
	if (offset + count > len) return (string_view());
	return (string_view(&data[offset], count));
}

std::string	string_view::string(void) const {
	std::string	str(data, len);
	return (str);
}
