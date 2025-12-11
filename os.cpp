/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   os.cpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 17:02:26 by aindjare          #+#    #+#             */
/*   Updated: 2025/12/11 17:37:52 by xenobas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "base.hpp"
#include "string_view.hpp"
#include "string_builder.hpp"
#include "dynamic_array.hpp"

#include <string>
#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

static void	MEM_copy(byte* src, i32 src_len, byte* dst, i32 dst_len) {
	if (src == 0 || dst == 0) {
		return ;
	}

	for (i32 i = 0; i < src_len && i < dst_len; ++i) {
		dst[i] = src[i];
	}
}

b32			OS_read_file(const string_view path, string_view& text) {
	byte	path_cstr[512] = { 0 };
	MEM_copy((byte*)path.text, path.len, path_cstr, 511);

	i32		fd = open((char*)path_cstr, O_RDONLY);
	if (fd == -1) {
		return (0);
	}

	string_builder	b;
	b32				ok = 1;
	while (ok) {
		char	buff[512] = { 0 };
		i32		buff_n = cast(i32)read(fd, buff, 511);
		if (buff_n <= 0) {
			ok = (buff_n == 0);
			break ;
		}

		string_view	buff_str(buff, buff_n);
		b.write(buff_str);
	}

	text = b.to_string();
	return (close(fd), ok);
}
b32			OS_stat_file(const string_view& _path, struct stat* buf = 0) {
	std::string		path = _path.to_string();


	struct stat	_buf;
	if (buf == 0)
		buf = &_buf;

	return (stat(path.c_str(), buf) == 0);
}
b32			OS_access_file(const string_view& _path, i32 flags = F_OK) {
	std::string	path = _path.to_string();
	return (access(path.c_str(), flags) == 0);
}

b32			OS_test_file_read(const string_view& path, b32 strict_regular = 0) {
	struct stat	stat;

	char		path_cstr[512] = { 0 };
	MEM_copy(cast(byte*)path.text, path.len, cast(byte*)path_cstr, 511);

	b32			stat_ok = (::stat(path_cstr, &stat) == 0);
	if (!stat_ok || (strict_regular && S_ISREG(stat.st_mode))) {
		return (0);
	}

	b32			access_ok = (access(path_cstr, F_OK | R_OK));
	return (access_ok);
}
b32			OS_test_dir_read(const string_view& path, b32 strict_regular = 0) {
	struct stat	stat;

	char		path_cstr[512] = { 0 };
	MEM_copy(cast(byte*)path.text, path.len, cast(byte*)path_cstr, 511);

	b32			stat_ok = (::stat(path_cstr, &stat) == 0);
	if (!stat_ok || !S_ISDIR(stat.st_mode) || (strict_regular && S_ISREG(stat.st_mode))) {
		return (0);
	}

	b32			access_ok = (access(path_cstr, F_OK | R_OK) == 0);
	return (access_ok);
}
b32			OS_test_dir_read_write(const string_view& path, b32 strict_regular = 0) {
	struct stat	stat;

	char		path_cstr[512] = { 0 };
	MEM_copy(cast(byte*)path.text, path.len, cast(byte*)path_cstr, 511);

	b32			stat_ok = (::stat(path_cstr, &stat) == 0);
	if (!stat_ok || !S_ISDIR(stat.st_mode) || (strict_regular && S_ISREG(stat.st_mode))) {
		return (0);
	}

	b32			access_ok = (access(path_cstr, F_OK | R_OK | W_OK) == 0);
	return (access_ok);
}
