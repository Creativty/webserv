/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   string_view.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/16 09:19:09 by aindjare          #+#    #+#             */
/*   Updated: 2025/11/01 15:23:26 by xenobas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <sstream>
#include <ostream>
#include <stdexcept>
#include "string_view.hpp"

static i32	cstring_len(const char* cstr) {
	if (cstr == 0)
		return (0);

	i32 len = 0;
	while (cstr[len])
		++len;
	return (len);
}

string_view::string_view(void): text(0), len(0) { };
string_view::~string_view(void) { };
string_view::string_view(const string_view& view): text(view.text), len(view.len) { };
string_view&			string_view::operator=(const string_view& view) {
	if (this != &view) {
		this->text = view.text;
		this->len = view.len;
	}
	return (*this);
};

string_view::string_view(const char* text): text((char*)text), len(cstring_len(text)) { };
string_view::string_view(char* text, i32 len): text(text), len(len) { };
string_view&			string_view::operator=(const char* text) {
	this->text = (char*)text;
	this->len = cstring_len(text);
	return (*this);
};

bool					string_view::operator==(const string_view& view) const {
	if (view.len != this->len)
		return (false);
	for (i32 i = 0; i < this->len; ++i) {
		if (view[i] != (*this)[i])
			return (false);
	}
	return (true);
}
bool					string_view::operator==(const char* text) const {
	return (string_view(text) == *this);
}
bool					string_view::operator!=(const string_view& view) const {
	return (!(*this == view));
}

char&					string_view::operator[](unsigned long long index) {
	if (index >= (unsigned long long)this->len)
		throw std::out_of_range("Index larger than length while indexing string_view");
	return (this->text[index]);
}
const char&				string_view::operator[](unsigned long long index) const {
	if (index >= (unsigned long long)this->len)
		throw std::out_of_range("Index larger than length while indexing string_view");
	return (this->text[index]);
}

char&					string_view::operator[](i32 index) {
	if (index < 0)
		throw std::out_of_range("Cannot use negative index while indexing string_view");
	return ((*this)[(unsigned long long)index]);
}
const char&				string_view::operator[](i32 index) const {
	if (index < 0)
		throw std::out_of_range("Cannot use negative index while indexing string_view");
	return ((*this)[(unsigned long long)index]);
}

string_view				string_view::slice(i32 begin, i32 end) const {
	if (begin < 0 || begin >= this->len)
		throw std::out_of_range("string_view::slice parameter begin is out of range");
	if (end < 0 || end > this->len)
		throw std::out_of_range("string_view::slice parameter end is out of range");
	if (end < begin)
		throw std::out_of_range("string_view::slice parameter end is invalid");
	return (string_view(&this->text[begin], end - begin));
}
string_view				string_view::slice(i32 begin) const {
	return (this->slice(begin, this->len));
}
std::string				string_view::to_string(void) const {
	std::ostringstream	stream;

	stream.write(this->text, this->len);
	return (stream.str());
}

std::ostream&			operator<<(std::ostream& stream, const string_view& view) {
	stream << '`';
	for (i32 i = 0; i < view.len; ++i) {
		if (view[i] == '\n')
			stream << "\\n";
		else
			stream << view[i];
	}
	stream << '\'';
	return (stream);
}

void					string_view::free(void) {
	if (this->text != 0) {
		delete[] this->text;
	}

	this->text = 0;
	this->len = 0;
};
string_view				string_view::alloc(const string_view& view) {
	if (!(bool)view)
		return (string_view());
	char*	text = new char[view.len];
	for (i32 i = 0; i < view.len; ++i)
		text[i] = view[i];
	return (string_view(text, view.len));
};

string_view::operator	bool(void) const {
	return (this->len > 0 && this->text != 0);
};

bool					string_view::has_prefix(const string_view& prefix) const {
	if (prefix.len > this->len)
		return (false);
	for (i32 i = 0; i < prefix.len; ++i) {
		if (this->text[i] != prefix[i])
			return (false);
	}
	return (true);
};
bool					string_view::has_suffix(const string_view& suffix) const {
	if (suffix.len > this->len)
		return (false);
	i32	offset = this->len - suffix.len; 
	for (i32 i = 0; i < suffix.len; ++i) {
		if (this->text[offset + i] != suffix[i])
			return (false);
	}
	return (true);
};
bool					string_view::has(const string_view& str) const {
	for (i32 i = 0; i < this->len - str.len; ++i) {
		bool	match = true;
		for (i32 j = 0; match && j < str.len; ++j) {
			if ((*this)[i + j] != str[j]) {
				match = false;
				break ;
			}
		}
		if (match) {
			return (true);
		}
	}
	return (false);
}
i32						string_view::index(const string_view& str) const {
	for (i32 i = 0; i < this->len; ++i) {
		i32	j = 0;
		while (j < str.len) {
			if ((*this)[i + j] != str[j]) {
				break ;
			}
			++j;
		}
		if (j == str.len) {
			return (i);
		}
	}
	return (-1);
}
i32						string_view::index(const string_view& str, i32 skip) const {
	i32	skip_count = 0;
	i32	skip_limit = skip;
	for (i32 i = 0; i < this->len; ++i) {
		i32	j = 0;
		while (j < str.len) {
			if ((*this)[i + j] != str[j]) {
				break ;
			}
			++j;
		}
		if (j == str.len) {
			++skip_count;
			if (skip_count >= skip_limit) {
				return (i);
			}
		}
	}
	return (-1);
}

b32						string_view::split_iter(const string_view& sep, string_view& slot) {
	if (this->len == 0) {
		return (0);
	}

	i32	index = this->index(sep);
	if (index == -1) {
		slot = *this;
		*this = string_view();
		return (1);
	}

	slot = this->slice(0, index);
	*this = this->slice(index + sep.len);
	return (1);
}
