#ifndef STRING_VIEW_HPP
#define STRING_VIEW_HPP
#include <string>
#include <ostream>
#include "core.hpp"

struct string_view {
	const char	*data;
	u64			len;

	string_view();
	string_view(const char* str);
	string_view(const char* str, u64 len);
	string_view(const std::string& str);
	string_view(const string_view& sv);

	string_view& operator=(const char* str);
	string_view& operator=(const std::string& str);
	string_view& operator=(const string_view& other);

	char operator[](u64 index) const;

	bool operator==(const string_view& other) const;
	bool operator!=(const string_view& other) const;

	string_view	slice(u64 offset, u64 count) const;
	std::string	string(void) const;
};

std::ostream&	operator<<(std::ostream& stream, string_view sv);
std::string		string_view_fmt(void);
extern bool		stream_literal;

#endif
