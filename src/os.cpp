/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   os.cpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 17:02:26 by aindjare          #+#    #+#             */
/*   Updated: 2025/12/20 14:46:31 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "base.hpp"
#include "string_view.hpp"
#include "string_builder.hpp"
#include "dynamic_array.hpp"

#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#define PATH_CAP 1024

b32			OS_read_file(const string_view path, string_view& text) {
	byte	path_cstr[PATH_CAP + 1] = { 0 };
	MEM_copy((byte*)path.text, path.len, path_cstr, PATH_CAP);

	i32		fd = ::open((char*)path_cstr, O_RDONLY);
	if (fd == -1) {
		return (0);
	}

	string_builder	b;
	b32				ok = 1;
	while (ok) {
		char	buff[PATH_CAP + 1] = { 0 };
		i32		buff_n = cast(i32)read(fd, buff, PATH_CAP);

		if (buff_n == 0) {
			break ;
		}
		if (buff_n  < 0) {
			ok = 0;
			break ;
		}

		string_view	buff_str(buff, buff_n);
		b.write(buff_str);
	}

	text = b.to_string();
	return (::close(fd), ok);
}
b32			OS_write_file(const string_view& path, const string_view& content, i32 flags) {
	byte	path_cstr[PATH_CAP + 1] = { 0 };
	MEM_copy((byte*)path.text, path.len, path_cstr, PATH_CAP);

	i32		fd = ::open((char*)path_cstr, flags | O_WRONLY, 0755);
	if (fd == -1) {
		return (0);
	}

	b32		ok_write		= 1;
	i32		chunk_capacity	= 1024 * 8; /* 8KiB */
	for (i32 i = 0; i < content.len; i += chunk_capacity) {
		i32	chunk_size = (chunk_capacity < (content.len - i)) ? chunk_capacity : (content.len - i);
		i64	ret_write = ::write(fd, &content[i], cast(u64)chunk_size);
		if (ret_write == -1) {
			ok_write = 0;
			break ;
		}
	}

	::close(fd);
	return (ok_write);
}
b32			OS_delete_file(const string_view& path) {
	byte	path_cstr[PATH_CAP + 1] = { 0 };
	MEM_copy((byte*)path.text, path.len, path_cstr, PATH_CAP);

	struct stat	stat;
	b32			stat_ok = (::stat((const char*)path_cstr, &stat) == 0);
	if (!stat_ok || ((stat.st_mode & S_IFMT) != S_IFREG)) {
		return (0);
	}

	return (::unlink((const char*)path_cstr) == 0);
}
b32			OS_stat_file(const string_view& _path, struct stat* buf = 0) {
	std::string		path = _path.to_string();

	struct stat	_buf;
	if (buf == 0)
		buf = &_buf;

	return (::stat(path.c_str(), buf) == 0);
}
b32			OS_access_file(const string_view& _path, i32 flags = F_OK) {
	std::string	path = _path.to_string();
	return (::access(path.c_str(), flags) == 0);
}

i64			OS_file_size(const char* path) {
	if (path == NULL) {
		return (-1);
	}

	struct stat	stat_buf; MEM_zero(stat_buf);
	i32			stat_ret = ::stat(path, &stat_buf);
	if (stat_ret == -1) {
		return (-1);
	}

	return (cast(i64)stat_buf.st_size);
}

b32			OS_test_file_read(const string_view& path, b32 strict_regular = 0) {
	struct stat	stat;

	char		path_cstr[PATH_CAP + 1] = { 0 };
	MEM_copy(cast(byte*)path.text, path.len, cast(byte*)path_cstr, PATH_CAP);

	b32			stat_ok = (::stat(path_cstr, &stat) == 0);
	if (!stat_ok || ((stat.st_mode & S_IFMT) != S_IFREG) || (strict_regular && S_ISREG(stat.st_mode))) {
		return (0);
	}

	b32			access_ok = (::access(path_cstr, F_OK | R_OK) == 0);
	return (access_ok);
}
b32			OS_test_file_write(const string_view& path, b32 strict_regular = 0) {
	struct stat	stat;

	char		path_cstr[PATH_CAP + 1] = { 0 };
	MEM_copy(cast(byte*)path.text, path.len, cast(byte*)path_cstr, PATH_CAP);

	b32			stat_ok = (::stat(path_cstr, &stat) == 0);
	if (!stat_ok || ((stat.st_mode & S_IFMT) != S_IFREG) || (strict_regular && S_ISREG(stat.st_mode))) {
		return (0);
	}

	b32			access_ok = (::access(path_cstr, F_OK | W_OK) == 0);
	return (access_ok);
}
b32			OS_test_file_exists(const string_view& path, b32 strict_regular = 0) {
	struct stat	stat;

	char		path_cstr[PATH_CAP + 1] = { 0 };
	MEM_copy(cast(byte*)path.text, path.len, cast(byte*)path_cstr, PATH_CAP);

	b32			stat_ok = (::stat(path_cstr, &stat) == 0);
	if (!stat_ok || ((stat.st_mode & S_IFMT) != S_IFREG) || (strict_regular && S_ISREG(stat.st_mode))) {
		return (0);
	}

	b32			access_ok = (::access(path_cstr, F_OK) == 0);
	return (access_ok);
}
b32			OS_test_dir_read(const string_view& path, b32 strict_regular = 0) {
	struct stat	stat;

	char		path_cstr[PATH_CAP + 1] = { 0 };
	MEM_copy((byte*)path.text, path.len, (byte*)path_cstr, PATH_CAP);

	b32			stat_ok = (::stat(path_cstr, &stat) == 0);
	if (!stat_ok || !S_ISDIR(stat.st_mode) || (strict_regular && !S_ISREG(stat.st_mode))) {
		return (0);
	}

	b32			access_ok = (::access(path_cstr, F_OK | R_OK) == 0);
	return (access_ok);
}
b32			OS_test_dir_read_write(const string_view& path, b32 strict_regular = 0) {
	struct stat	stat;

	char		path_cstr[PATH_CAP + 1] = { 0 };
	MEM_copy((byte*)path.text, path.len, (byte*)path_cstr, PATH_CAP);

	b32			stat_ok = (::stat(path_cstr, &stat) == 0);
	if (!stat_ok || !S_ISDIR(stat.st_mode) || (strict_regular && !S_ISREG(stat.st_mode))) {
		return (0);
	}

	b32			access_ok = (::access(path_cstr, F_OK | R_OK | W_OK) == 0);
	return (access_ok);
}

/* DESCRIPTION(xenobas): Returns the timestamp in milliseconds */
i64			OS_timestamp_now(void) {
	struct timespec	tp;
	i32				ret = ::clock_gettime(CLOCK_MONOTONIC, &tp);
	if (ret == 0) {
		return (cast(i64)(tp.tv_sec * 1000) + cast(i64)(tp.tv_nsec / 1000000));
	}
	return (-1);
}
