/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   strconv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/16 16:15:30 by aindjare          #+#    #+#             */
/*   Updated: 2025/09/16 16:15:46 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "strconv.hpp"

bool	strconv::parse_i64(const type::string& string, i64& n) {
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
