/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   uri.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/27 14:13:43 by aindjare          #+#    #+#             */
/*   Updated: 2025/07/27 14:20:39 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* REFERENCES:
 * - https://en.wikipedia.org/wiki/Percent-encoding
 * - https://datatracker.ietf.org/doc/html/rfc3986#section-2.1
 */

#include "webserv.hpp"

static bool char_is_hex(char c) {
	if (c >= '0' && c <= '9') return (true);
	if (c >= 'a' && c <= 'f') return (true);
	if (c >= 'A' && c <= 'F') return (true);
	return (false);
}

static char	char_to_lower(char c) {
	if (c >= 'A' && c <= 'Z')
		return (c - ('A' - 'a'));
	return (c);
}

static int	int_from_hex(char c) { // most signficat byte, least significant byte
	c = char_to_lower(c);
	if (c >= '0' && c <= '9') return (c - '0');
	if (c >= 'a' && c <= 'f') return (c - 'a' + 10);
	return (0);
}

std::string	decode_uri_path(const std::string& src) {
	std::string	dst, tmp;

	for (size_t i = 0; i < src.size();) {
		switch (src[i]) {
			case '%': {
				tmp = src.substr(i + 1);
				if (tmp.size() < 2 || !char_is_hex(tmp[0]) || !char_is_hex(tmp[1])) {
					dst += '%';
					i++;
					continue ;
				}
				dst += (char)((int_from_hex(tmp[0]) << (4 * 1)) | (int_from_hex(tmp[1]) << (4 * 0)));
				i += 3;
			} break;
			case '+': {
				dst += ' ';
				i++;
			} break;
			default: {
				dst += src[i++];
			} break;
		}
	}
	return (dst);
}

#ifdef TEST_URI
#include <iostream>
int	main(int argc, const char **argv) {
	for (int i = 1; i < argc; i++) {
		std::string	arg = argv[i];
		std::string	res = decode_uri_path(arg);
		std::cout << '`' << arg << '`' << " -> " << '`' << res << '`' << std::endl;
	}
}
#endif
