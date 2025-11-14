/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   http.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xenobas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/01 18:33:49 by xenobas           #+#    #+#             */
/*   Updated: 2025/11/14 16:23:02 by xenobas          ###   ########.fr       */
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

	req.chunks = dynamic_array<HTTP_Chunk>();
	req.chunk.index = -1;
	req.chunk.size = 0;

	req.buff = dynamic_array<byte>();
	req.buff_index = 0;
	req.buff_stage = HTTP_REQUEST_STAGE_METHOD;
	return (req);
}
void				HTTP_request_delete(HTTP_Request& req) {
	WEBSERV_uri_delete(req.uri);

	for_table_begin(req.headers, HTTP_Headers, header) {
		header.value.free();
	} for_table_end ;
	req.headers.free();

	req.chunks.free();
	req.buff.free();

	req = HTTP_request_make();
}
void				HTTP_request_debug(HTTP_Request& req) {
	printf("=== HTTP_Request  BEGIN ===\n");
		{ /* Parsing status */
			char	buff[64] = { 0 };
			if (HTTP_request_is_error(req)) {
				snprintf(buff, 64, "has failed");
			} else if (HTTP_request_is_closed(req)) {
				snprintf(buff, 64, "is done");
			} else {
				snprintf(buff, 64, "is waiting for more bytes, thus incomplete");
			}
			printf("    Parsing %s\n", buff);
		}
		{ /* Method */
			char	buff[64] = { 0 };
			switch (req.method) {
			case WEBSERV_METHOD_PUT:
				snprintf(buff, 64, "PUT");
				break ;
			case WEBSERV_METHOD_POST:
				snprintf(buff, 64, "POST");
				break ;
			case WEBSERV_METHOD_GET:
				snprintf(buff, 64, "GET");
				break ;
			case WEBSERV_METHOD_COUNT:
				snprintf(buff, 64, "COUNT/UNREACHABLE");
				break ;
			case WEBSERV_METHOD_INVALID:
			default:
				snprintf(buff, 64, "UNKNOWN %d", req.method);
				break ;
			}
			printf("    Method %s\n", buff);
		}
		{ /* URI */
			const WEBSERV_URI& uri = req.uri;

			{ /* Path */
				printf("    Path");
				if (uri.path.len == 0) {
					printf(" is empty");
				}
				printf("\n");
				for	(i32 i = 0; i < uri.path.len; ++i) {
					printf("        \"%.*s\"\n", uri.path[i].len, uri.path[i].text);
				}
			}

			{ /* Query */
				printf("    Query");
				if (uri.query.count == 0) {
					printf(" is empty");
				}
				printf("\n");

				for_table_begin(uri.query, const hash_table<string_view>, param) {
					printf("        \"%.*s\" = \"%.*s\"\n", param.key.len, param.key.text, param.value.len, param.value.text);
				} for_table_end ;
			}
		}
		{ /* Headers */
			const HTTP_Headers& headers = req.headers;

			printf("    Headers \n");
			for_table_begin(headers, const HTTP_Headers, header) {
				printf("        \"%.*s\" = \"%.*s\"\n", header.key.len, header.key.text, header.value.len, header.value.text);
			} for_table_end ;
		}
		{ /* Body */
			char	buff[64] = { 0 };
			if (req.content_length >= 0) {
				snprintf(buff, 64, "content length %ld bytes", req.content_length);
			} else {
				snprintf(buff, 64, "connection-close-bound bytes");
			}

			for (i32 i = 0; i < req.chunks.len; ++i) {
				const HTTP_Chunk&	chunk = req.chunks[i];
				printf("chunk containing %d %s starts at %d", chunk.size, chunk.size == 1 ? "byte" : "bytes", chunk.index);
			}
		}
	printf("=== HTTP_Request   END  ===\n");
}

b32					HTTP_request_is_closed(const HTTP_Request& req) {
	return (
		req.buff_stage == HTTP_REQUEST_STAGE_DONE
		|| req.buff_stage == HTTP_REQUEST_STAGE_ERROR
	);
}
b32					HTTP_request_is_error(const HTTP_Request& req) {
	return (req.buff_stage == HTTP_REQUEST_STAGE_ERROR);
}
static b32			HTTP_request_is_stage_body(const HTTP_Request& req) {
	return (req.buff_stage == HTTP_REQUEST_STAGE_BODY || req.buff_stage == HTTP_REQUEST_STAGE_CHUNK);
}

b32					HTTP_request_read(HTTP_Request& req, const byte* data, i32 size) {
	if (HTTP_request_is_closed(req) || size == 0  || data == 0) {
		req.buff_stage = HTTP_REQUEST_STAGE_ERROR;
		return (0);
	}

	req.buff.push(size, data);
	return (HTTP_request_read_internal(req));
}
void				HTTP_request_close(HTTP_Request& req) {
	if (HTTP_request_is_closed(req) /* Already closed */
		|| !HTTP_request_is_stage_body(req) /* Waiting for essential header bytes */
		|| (req.content_length >= 0 && req.chunk.size != req.content_length) /* Mismatching expected length and received length */ ) {
		req.buff_stage = HTTP_REQUEST_STAGE_ERROR;
	} else {
		req.buff_stage = HTTP_REQUEST_STAGE_DONE;
	}
}

static b32			HTTP_parse_i64_check(i64 number, i64 sign, i64 v, i64 base = 10) {
	switch (sign) {
	case +1: {
		if (I64_MAX / base < number || (I64_MAX / base == number && I64_MAX % base < v)) {
			return (false);
		}
	} break ;
	case -1: {
		if (-(I64_MIN / base) < number || (-(I64_MIN / base) == number && -(I64_MIN % base) < v)) {
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
		i64	v = cast(i64)(str[i] - '0');
		if (str[i] < '0' || str[i] > '9' || !HTTP_parse_i64_check(n, s, v)) {
			fail = 1;
			break ;
		}
		n = (n * 10l) + v;
		++i;
	}
	return (fail ? fallback : (n * s));
}

static b32			HTTP_hex_match(char c) {
	if (c >= 'A' && c <= 'Z') {
		c ^= (2 << 4);
	}
	return ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'));
}
static u8			HTTP_hex_value(char c) {
	if (c >= 'A' && c <= 'Z') {
		c ^= (2 << 4);
	}
	if (c >= 'a' && c <= 'z') {
		return (cast(u8)(c - 'a' + 10));
	}
	if (c >= '0' && c <= '9') {
		return (cast(u8)(c - '0' +  0));
	}
	return (0u);
}
static i64			HTTP_parse_i64_hex(const string_view& str, i32 fallback = -1) {
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
		i64	v = HTTP_hex_value(str[i]);
		if (!HTTP_hex_match(str[i]) || !HTTP_parse_i64_check(n, s, v, /* base = */ 16)) {
			fail = 1;
			break ;
		}
		n = (n * 16l) + v;
		++i;
	}
	return (fail ? fallback : (n * s));
}

static void			HTTP_request_read_stage_method(HTTP_Request& req) {
	if (req.buff_index >= req.buff.len)
		return ;

	string_view	str((char*)&req.buff[req.buff_index], req.buff.len - req.buff_index);
	string_view	sep(" ");

	i32	idx_end = str.index(sep);
	if (idx_end == -1) {
		return ;
	}

	string_view	str_method = str.slice(0, idx_end);
	req.buff_index += idx_end + sep.len;
	
	req.method = WEBSERV_method_make(str_method);
	if (req.method == WEBSERV_METHOD_INVALID) {
		req.buff_stage = HTTP_REQUEST_STAGE_ERROR;
	} else {
		req.buff_stage = HTTP_REQUEST_STAGE_URI;
	}
}
static void			HTTP_request_read_stage_uri(HTTP_Request& req) {
	if (req.buff_index >= req.buff.len)
		return ;

	string_view	str((char*)&req.buff[req.buff_index], req.buff.len - req.buff_index);
	string_view	sep(" ");

	i32	idx_end = str.index(sep);
	if (idx_end == -1) {
		return ;
	}

	string_view	str_uri = str.slice(0, idx_end);
	req.buff_index += idx_end + sep.len;
	
	req.uri = WEBSERV_uri_decode(str_uri);
	if (!req.uri.ok) {
		req.buff_stage = HTTP_REQUEST_STAGE_ERROR;
	} else {
		req.buff_stage = HTTP_REQUEST_STAGE_VERSION;
	}
}
static void			HTTP_request_read_stage_version(HTTP_Request& req) {
	if (req.buff_index >= req.buff.len)
		return ;

	string_view	str((char*)&req.buff[req.buff_index], req.buff.len - req.buff_index);
	string_view	sep("\r\n");

	i32	idx_end = str.index(sep);
	if (idx_end == -1) {
		return ;
	}

	string_view	str_version = str.slice(0, idx_end);
	req.buff_index += idx_end + sep.len;

	if (WEBSERV_http_version_supported(str_version)) {
		req.buff_stage = HTTP_REQUEST_STAGE_HEADERS;
	} else {
		req.buff_stage = HTTP_REQUEST_STAGE_ERROR;
	}
}
static void			HTTP_request_read_stage_headers_infix(HTTP_Request& req, const string_view& name, const string_view& value) {
	if (name.eq_insensitive("content-length")) {
		req.content_length = HTTP_parse_i64(value);
		if (req.content_length < 0) {
			req.content_length = -1;
		}
		return ;
	}
	if (name.eq_insensitive("transfer-encoding")) {
		req.chunked = value.eq_insensitive("chunked");
		if (!req.chunked) {
			req.buff_stage = HTTP_REQUEST_STAGE_ERROR;
		}
		return ;
	}
}
static void			HTTP_request_read_stage_headers_postfix(HTTP_Request& req) {
	if (req.chunked && req.content_length >= 0) {
		req.chunked = 0;
	}
}
static void			HTTP_request_read_stage_headers(HTTP_Request& req) {
	if (req.buff_index >= req.buff.len)
		return ;

	string_view	str((char*)&req.buff[req.buff_index], req.buff.len - req.buff_index);
	string_view	sep("\r\n");

	i32	idx_end = str.index(sep);
	if (idx_end == -1) {
		return ;
	}

	string_view	str_header = str.slice(0, idx_end);
	req.buff_index += idx_end + sep.len;
	if (str_header == "") {
		HTTP_request_read_stage_headers_postfix(req);
		req.buff_stage = req.chunked ? HTTP_REQUEST_STAGE_CHUNK : HTTP_REQUEST_STAGE_BODY;
		return ;
	}

	i32	idx_name_end = str_header.index(":");
	if (idx_name_end == -1) {
		req.buff_stage = HTTP_REQUEST_STAGE_ERROR;
		return ;
	}
	
	string_view	str_name = str_header.slice(0, idx_name_end).trim_right();
	string_view	str_value = str_header.slice(idx_name_end + 1).trim_left();

	req.headers.set(str_name, string_view::alloc(str_value));
	HTTP_request_read_stage_headers_infix(req, str_name, str_value);
}
static void			HTTP_request_read_stage_body(HTTP_Request& req) {
	i32	rem = req.buff.len - req.buff_index;
	req.chunk.size += rem;

	if (req.chunk.index < 0) {
		req.chunk.index = req.buff_index;
	}

	req.buff_index += rem;
	if (req.content_length >= 0 && req.buff_index - req.chunk.index >= req.content_length) {
		HTTP_request_close(req);
	}
}
static void			HTTP_request_read_stage_chunk(HTTP_Request& req) {
	if (req.buff_index >= req.buff.len)
		return ;

	string_view	str((char*)&req.buff[req.buff_index], req.buff.len - req.buff_index);
	string_view	sep("\r\n");

	i32	idx_end = str.index(sep);
	if (idx_end == -1) {
		return ;
	}

	string_view	str_chunk_size = str.slice(0, idx_end);
	if (str_chunk_size.len == 0) {
		req.buff_stage = HTTP_REQUEST_STAGE_ERROR;
		return ;
	}
	for (i32 i = 0; i < str_chunk_size.len; ++i) {
		if (!HTTP_hex_match(str_chunk_size[i])) {
			req.buff_stage = HTTP_REQUEST_STAGE_ERROR;
			return ;
		}
	}

	req.buff_index += idx_end + sep.len;

	req.chunk.size = cast(i32)HTTP_parse_i64_hex(str_chunk_size);
	req.chunk.index = req.buff_index;
	if (req.chunk.size == 0) {
		HTTP_request_close(req);
		return ;
	}

	req.buff_stage = HTTP_REQUEST_STAGE_CHUNK_DATA;
}
static void			HTTP_request_read_stage_chunk_data(HTTP_Request& req) {
	if (req.buff_index >= req.buff.len)
		return ;

	i32	buff_rem = req.buff.len - req.buff_index;
	if (buff_rem >= req.chunk.size) {
		req.chunks.push(req.chunk);

		req.buff_index += req.chunk.size;
		req.buff_stage  = HTTP_REQUEST_STAGE_CHUNK;
	}
}

static b32			HTTP_request_read_internal(HTTP_Request& req) {
	if (req.buff_index >= req.buff.len) {
		return (1);
	}

	if (req.buff_stage == HTTP_REQUEST_STAGE_METHOD) {
		HTTP_request_read_stage_method(req);
	}
	if (req.buff_stage == HTTP_REQUEST_STAGE_URI) {
		HTTP_request_read_stage_uri(req);
	}
	if (req.buff_stage == HTTP_REQUEST_STAGE_VERSION) {
		HTTP_request_read_stage_version(req);
	}
	if (req.buff_stage == HTTP_REQUEST_STAGE_HEADERS) {
		HTTP_request_read_stage_headers(req);
	}
	if (req.buff_stage == HTTP_REQUEST_STAGE_BODY) {
		HTTP_request_read_stage_body(req);
	}
	if (req.buff_stage == HTTP_REQUEST_STAGE_CHUNK) {
		HTTP_request_read_stage_chunk(req);
	}
	if (req.buff_stage == HTTP_REQUEST_STAGE_CHUNK_DATA) {
		HTTP_request_read_stage_chunk_data(req);
	}
	return (!HTTP_request_is_error(req));
}
