/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   string_builder.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xenobas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/01 13:58:18 by xenobas           #+#    #+#             */
/*   Updated: 2025/11/01 14:06:37 by xenobas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "string_builder.hpp"

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
string_view	string_builder::to_string(void) const {
	if (this->data.len == 0)
		return (string_view());

	i32		len = this->data.len;
	char*	text = new char[len + 1]();
	for (i32 i = 0; i < len; ++i)
		text[i] = this->data[i];
	return (string_view((char*)text, len));
}
