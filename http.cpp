/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   http.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xenobas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/01 18:33:49 by xenobas           #+#    #+#             */
/*   Updated: 2025/11/03 17:48:45 by xenobas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

static b32			HTTP_request_read_internal(HTTP_Request& req);

HTTP_Request		HTTP_request_make(void) {
	HTTP_Request	req;

	req.method = WEBSERV_METHOD_INVALID;
	req.uri = WEBSERV_uri_make("");

	req.headers = HTTP_Headers();
	req.headers.case_insensitive = 1;

	req.chunked = 0;
	req.content_length = -1;

	req.body_offset = -1;
	req.body_length =  0;

	req.buff = dynamic_array<byte>();
	req.buff_cursor_index = 0;
	req.buff_cursor_stage = HTTP_REQUEST_CURSOR_STAGE_METHOD;
	return (req);
}
void				HTTP_request_delete(HTTP_Request& req) {
	req.buff.free();

	for (i32 i = 0; i < req.headers.cap; ++i) {
		HTTP_Headers::item	header = req.headers.items[i];
		if (!header.used()) {
			continue ;
		}

		header.value.free();
	}
	req.headers.free();

	req = HTTP_request_make();
}

b32					HTTP_request_is_closed(const HTTP_Request& req) {
	return (
		req.buff_cursor_stage == HTTP_REQUEST_CURSOR_STAGE_DONE
		|| req.buff_cursor_stage == HTTP_REQUEST_CURSOR_STAGE_ERROR
	);
}
b32					HTTP_request_is_error(const HTTP_Request& req) {
	return (req.buff_cursor_stage == HTTP_REQUEST_CURSOR_STAGE_ERROR);
}

b32					HTTP_request_read(HTTP_Request& req, const byte* data, i32 size) {
	if (HTTP_request_is_closed(req) || size == 0  || data == 0) {
		req.buff_cursor_stage = HTTP_REQUEST_CURSOR_STAGE_ERROR;
		return (0);
	}

	req.buff.push(size, data);
	return (HTTP_request_read_internal(req));
}
void				HTTP_request_close(HTTP_Request& req) {
	if (HTTP_request_is_closed(req)) {
		req.buff_cursor_stage = HTTP_REQUEST_CURSOR_STAGE_ERROR;
	} else {
		req.buff_cursor_stage = HTTP_REQUEST_CURSOR_STAGE_DONE;
	}
}

static b32			HTTP_parse_i64_range_check(i64 number, i64 sign, char d) {
	i64	v = cast(i64)(d - '0');
	switch (sign) {
	case +1: {
		if (I64_MAX / 10 < number || (I64_MAX / 10 == number && I64_MAX % 10 < v)) {
			return (false);
		}
	} break ;
	case -1: {
		if (-(I64_MIN / 10) < number || (-(I64_MIN / 10) == number && -(I64_MIN % 10) < v)) {
			return (false);
		}
	} break ;
	}
	return (sign == -1 || sign == +1);
}
static i64			HTTP_parse_i64(const string_view& str, i32 fallback = -1) {
	i64	n = 0;
	i64	s = 1;

	i32	i = 0;
	b32	fail = 0;
	if (str.len > 0) {
		if (str[i] == '-' || str[i] == '+') {
			if (str[i] == '-') {
				s = -1;
			}
			++i;
		}
	}
	while (i < str.len && !fail) {
		if (str[i] < '0' || str[i] > '9' || !HTTP_parse_i64_range_check(n, s, str[i])) {
			fail = 1;
			break ;
		}
		i64	v = cast(i64)(str[i] - '0');
		n = (n * 10l) + v;
		++i;
	}
	return (fail ? fallback : (n * s));
}

static void			HTTP_request_read_stage_method(HTTP_Request& req) {
	if (req.buff_cursor_index >= req.buff.len)
		return ;

	string_view	str((char*)&req.buff[req.buff_cursor_index], req.buff.len - req.buff_cursor_index);
	string_view	sep(" ");

	i32	idx_end = str.index(sep);
	if (idx_end == -1) {
		return ;
	}

	string_view	str_method = str.slice(0, idx_end);
	req.buff_cursor_index += idx_end + sep.len;
	
	req.method = WEBSERV_method_make(str_method);
	if (req.method == WEBSERV_METHOD_INVALID) {
		req.buff_cursor_stage = HTTP_REQUEST_CURSOR_STAGE_ERROR;
	} else {
		req.buff_cursor_stage = HTTP_REQUEST_CURSOR_STAGE_URI;
	}
}
static void			HTTP_request_read_stage_uri(HTTP_Request& req) {
	if (req.buff_cursor_index >= req.buff.len)
		return ;

	string_view	str((char*)&req.buff[req.buff_cursor_index], req.buff.len - req.buff_cursor_index);
	string_view	sep(" ");

	i32	idx_end = str.index(sep);
	if (idx_end == -1) {
		return ;
	}

	string_view	str_uri = str.slice(0, idx_end);
	req.buff_cursor_index += idx_end + sep.len;
	
	req.uri = WEBSERV_uri_decode(str_uri);
	if (!req.uri.ok) {
		req.buff_cursor_stage = HTTP_REQUEST_CURSOR_STAGE_ERROR;
	} else {
		req.buff_cursor_stage = HTTP_REQUEST_CURSOR_STAGE_VERSION;
	}
}
static void			HTTP_request_read_stage_version(HTTP_Request& req) {
	if (req.buff_cursor_index >= req.buff.len)
		return ;

	string_view	str((char*)&req.buff[req.buff_cursor_index], req.buff.len - req.buff_cursor_index);
	string_view	sep("\r\n");

	i32	idx_end = str.index(sep);
	if (idx_end == -1) {
		return ;
	}

	string_view	str_version = str.slice(0, idx_end);
	req.buff_cursor_index += idx_end + sep.len;

	if (WEBSERV_http_version_supported(str_version)) {
		req.buff_cursor_stage = HTTP_REQUEST_CURSOR_STAGE_HEADERS;
	} else {
		req.buff_cursor_stage = HTTP_REQUEST_CURSOR_STAGE_ERROR;
	}
}
static void			HTTP_request_read_stage_headers_special(HTTP_Request& req, const string_view& name, const string_view& value) {
	if (name.eq_insensitive("content-length")) {
		req.content_length = HTTP_parse_i64(value);
		if (req.content_length < 0) {
			req.content_length = -1;
		}
	}
	if (name.eq_insensitive("transfer-encoding")) {
		req.chunked = (value == "chunked");
	}
}
static void			HTTP_request_read_stage_headers(HTTP_Request& req) {
	if (req.buff_cursor_index >= req.buff.len)
		return ;

	string_view	str((char*)&req.buff[req.buff_cursor_index], req.buff.len - req.buff_cursor_index);
	string_view	sep("\r\n");

	i32	idx_end = str.index(sep);
	if (idx_end == -1) {
		return ;
	}

	string_view	str_header = str.slice(0, idx_end);
	req.buff_cursor_index += idx_end + sep.len;
	if (str_header == "") {
		req.buff_cursor_stage = req.chunked ? HTTP_REQUEST_CURSOR_STAGE_BODY_CHUNKED : HTTP_REQUEST_CURSOR_STAGE_BODY;
		return ;
	}

	i32	idx_name_end = str_header.index(":");
	if (idx_name_end == -1) {
		req.buff_cursor_stage = HTTP_REQUEST_CURSOR_STAGE_ERROR;
		return ;
	}
	
	string_view	str_name = str_header.slice(0, idx_name_end).trim_right();
	string_view	str_value = str_header.slice(idx_name_end + 1).trim_left();

	req.headers.set(str_name, string_view::alloc(str_value));
	HTTP_request_read_stage_headers_special(req, str_name, str_value);
}
static void			HTTP_request_read_stage_body(HTTP_Request& req) {
	i32	rem = req.buff.len - req.buff_cursor_index;
	req.body_length += rem;

	if (req.body_offset < 0) {
		req.body_offset = req.buff_cursor_index;
	}

	req.buff_cursor_index += rem;
	if (req.content_length >= 0 && req.buff_cursor_index - req.body_offset >= req.content_length) {
		HTTP_request_close(req);
	}
}

static b32			HTTP_request_read_internal(HTTP_Request& req) {
	if (req.buff_cursor_index >= req.buff.len) {
		return (1);
	}

	if (req.buff_cursor_stage == HTTP_REQUEST_CURSOR_STAGE_METHOD) {
		HTTP_request_read_stage_method(req);
	}
	if (req.buff_cursor_stage == HTTP_REQUEST_CURSOR_STAGE_URI) {
		HTTP_request_read_stage_uri(req);
	}
	if (req.buff_cursor_stage == HTTP_REQUEST_CURSOR_STAGE_VERSION) {
		HTTP_request_read_stage_version(req);
	}
	if (req.buff_cursor_stage == HTTP_REQUEST_CURSOR_STAGE_HEADERS) {
		HTTP_request_read_stage_headers(req);
	}
	if (req.buff_cursor_stage == HTTP_REQUEST_CURSOR_STAGE_BODY) {
		HTTP_request_read_stage_body(req);
	}
	return (!HTTP_request_is_error(req));
}

