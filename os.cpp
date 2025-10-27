/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   os.cpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 17:02:26 by aindjare          #+#    #+#             */
/*   Updated: 2025/10/27 10:36:19 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "base.hpp"
#include "string_view.hpp"
#include "dynamic_array.hpp"

#include <string>
#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static i32	cstring_len(cstring str) {
	i32	len = 0;
	if (str != 0) {
		while (str[len]) {
			len++;
		}
	}
	return (len);
}

b32		OS_read_file(const string_view _path, dynamic_array<byte>& out_data) {
	std::string		path = _path.to_string();
	std::fstream	stream(path.c_str(), std::ios::in | std::ios::binary);

	if (stream.good()) {
		out_data.clear();
		while (!stream.eof()) {
			byte	buff[512] = { 0 };
			stream.get((char*)buff, 512, '\0');

			i32		buff_len = cstring_len(cast(cstring)buff);
			out_data.push(buff_len, buff);
			if (buff_len != 512 - 1)
				out_data.push(0);
		}
	}
	return (stream.good());
}

b32		OS_stat_file(const string_view& _path, struct stat* buf = 0) {
	std::string		path = _path.to_string();


	struct stat	_buf;
	if (buf == 0)
		buf = &_buf;

	return (stat(path.c_str(), buf) == 0);
}

b32		OS_access_file(const string_view& _path, i32 flags = F_OK) {
	std::string	path = _path.to_string();
	return (access(path.c_str(), flags) == 0);
}
