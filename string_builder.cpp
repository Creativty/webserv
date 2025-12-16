/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   string_builder.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xenobas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/01 13:58:18 by xenobas           #+#    #+#             */
/*   Updated: 2025/12/16 14:16:29 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "string_builder.hpp"
#define STRING_BUILDER_STR(V) #V
#define STRING_BUILDER_XSTR(V) STRING_BUILDER_STR(V)

string_builder::string_builder(void): data() { }
string_builder::~string_builder(void) {
	data.free();
}

void		string_builder::write(char ch) {
	this->data.push(ch);
}
void		string_builder::write(const string_view& str) {
	this->data.push(str.len, (char*)str.text);
}
void		string_builder::write(const char* str) {
	i32	len = 0;
	while (str != NULL && str[len] != '\0') {
		++len;
	}

	this->data.push(len, (char*)str);
}

static void	write_number(string_builder& builder, i64 n) {
	if (n == 0)
		return ;
	write_number(builder, n / 10);
	builder.write(cast(char)('0' + cast(char)(n % 10)));
}
void		string_builder::write(i64 n) {
	if (n == 0) {
		this->data.push('0');
	} else if (n == I64_MIN) {
		this->write(STRING_BUILDER_STR(I64_MIN));
	} else if (n < 0) {
		this->data.push('-');
		this->write(-n);
	} else {
		write_number(*this, n);
	}
}

string_view	string_builder::to_string(void) const {
	if (this->data.len == 0)
		return (string_view());

	i32		len = this->data.len;
	char*	text = new char[len + 1]();
	for (i32 i = 0; i < len; ++i)
		text[i] = this->data[i];
	return (string_view((char*)text, len));
}
string_view	string_builder::to_view(void) const {
	i32		len = this->data.len;
	char*	text = this->data.data;
	return (string_view(text, len));
}
