/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   string_view.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/16 09:19:09 by aindjare          #+#    #+#             */
/*   Updated: 2025/10/28 18:41:32 by xenobas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <sstream>
#include <ostream>
#include <stdexcept>
#include "string_view.hpp"

static int	cstring_len(const char* cstr) {
	if (cstr == 0)
		return (0);

	int len = 0;
	while (cstr[len])
		++len;
	return (len);
}

string_view::string_view(void): text(0), len(0) { };
string_view::~string_view(void) { };
string_view::string_view(const string_view& view): text(view.text), len(view.len) { };
string_view&	string_view::operator=(const string_view& view) {
	if (this != &view) {
		this->text = view.text;
		this->len = view.len;
	}
	return (*this);
};

string_view::string_view(const char* text): text((char*)text), len(cstring_len(text)) { };
string_view::string_view(char* text, int len): text(text), len(len) { };
string_view&	string_view::operator=(const char* text) {
	this->text = (char*)text;
	this->len = cstring_len(text);
	return (*this);
};

bool	string_view::operator==(const string_view& view) const {
	if (view.len != this->len)
		return (false);
	for (int i = 0; i < this->len; ++i) {
		if (view[i] != (*this)[i])
			return (false);
	}
	return (true);
}
bool	string_view::operator==(const char* text) const {
	return (string_view(text) == *this);
}
bool	string_view::operator!=(const string_view& view) const {
	return (!(*this == view));
}

char&		string_view::operator[](unsigned long long index) {
	if (index >= (unsigned long long)this->len)
		throw std::out_of_range("Index larger than length while indexing string_view");
	return (this->text[index]);
}
const char&	string_view::operator[](unsigned long long index) const {
	if (index >= (unsigned long long)this->len)
		throw std::out_of_range("Index larger than length while indexing string_view");
	return (this->text[index]);
}

char&		string_view::operator[](int index) {
	if (index < 0)
		throw std::out_of_range("Cannot use negative index while indexing string_view");
	return ((*this)[(unsigned long long)index]);
}
const char&	string_view::operator[](int index) const {
	if (index < 0)
		throw std::out_of_range("Cannot use negative index while indexing string_view");
	return ((*this)[(unsigned long long)index]);
}

bool			string_view::has_prefix(const string_view& prefix) const {
	if (prefix.len > this->len)
		return (false);
	for (int i = 0; i < prefix.len; ++i) {
		if (this->text[i] != prefix[i])
			return (false);
	}
	return (true);
};
bool			string_view::has_suffix(const string_view& suffix) const {
	if (suffix.len > this->len)
		return (false);
	int	offset = this->len - suffix.len; 
	for (int i = 0; i < suffix.len; ++i) {
		if (this->text[offset + i] != suffix[i])
			return (false);
	}
	return (true);
};

string_view		string_view::slice(int begin, int end) const {
	if (begin < 0 || begin >= this->len)
		throw std::out_of_range("string_view::slice parameter begin is out of range");
	if (end < 0 || end > this->len)
		throw std::out_of_range("string_view::slice parameter end is out of range");
	if (end < begin)
		throw std::out_of_range("string_view::slice parameter end is invalid");
	return (string_view(&this->text[begin], end - begin));
}
std::string		string_view::to_string(void) const {
	std::ostringstream	stream;

	stream.write(this->text, this->len);
	return (stream.str());
}

std::ostream&	operator<<(std::ostream& stream, const string_view& view) {
	stream << '`';
	for (int i = 0; i < view.len; ++i) {
		if (view[i] == '\n')
			stream << "\\n";
		else
			stream << view[i];
	}
	stream << '\'';
	return (stream);
}

void		string_view::free(void) {
	if (this->text != 0) {
		delete[] this->text;
	}

	this->text = 0;
	this->len = 0;
};
string_view	string_view::alloc(const string_view& view) {
	if (!(bool)view)
		return (string_view());
	char*	text = new char[view.len];
	for (int i = 0; i < view.len; ++i)
		text[i] = view[i];
	return (string_view(text, view.len));
};

string_view::operator bool(void) const {
	return (this->len > 0 && this->text != 0);
};
