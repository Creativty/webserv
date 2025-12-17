/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cstring.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/13 17:03:06 by aindjare          #+#    #+#             */
/*   Updated: 2025/12/16 14:09:27 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "base.hpp"

i32	cstring_write_u64(char* buff, i32 cap, u64 val) {
	i32	end = 0;
	u64	val_end = val;
	do {
		end++;
		val_end = val_end / 10;
	} while (val_end > 0);

	if (end + 1 >= cap) {
		return (0);
	}

	i32	idx = 0;
	while (idx < end) {
		buff[end - idx - 1] = '0' + cast(char)(val % 10ul);
		val = val / 10ul;
		++idx;
	}
	buff[idx] = '\0';
	return (idx);
}
i32	cstring_length(const char* str) {
	i32	len = 0;
	while (str && str[len]) {
		++len;
	}
	return (len);
}
