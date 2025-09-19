/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   http.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/19 16:29:15 by aindjare          #+#    #+#             */
/*   Updated: 2025/09/19 18:30:42 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "http.hpp"

static http::method_kind	http_method_from_string(type::string s) {
	if (s == "OPTIONS")
		return (http::METHOD_OPTIONS);
	if (s == "GET")
		return (http::METHOD_GET);
	if (s == "HEAD")
		return (http::METHOD_HEAD);
	if (s == "POST")
		return (http::METHOD_POST);
	if (s == "PUT")
		return (http::METHOD_PUT);
	if (s == "DELETE")
		return (http::METHOD_DELETE);
	if (s == "CONNECT")
		return (http::METHOD_CONNECT);
	if (s == "TRACE")
		return (http::METHOD_TRACE);
	return (http::METHOD_UNKNOWN);
}
static byte					http_byte_peek(http::request& req) {
	if (req._index >= req._bytes.len)
		return (0);
	return (req._bytes[req._index]);
}
static byte					http_byte_next(http::request& req) {
	byte	b = http_byte_peek(req);
	if (b != 0)
		req._index++;
	return (b);
}
static bool					http_bytes_done(http::request& req) {
	return (http_byte_peek(req) == 0);
}

static bool			parse_protocol_method(http::request& req) {
	u32	begin = req._index;
	while (!http_bytes_done(req) && http_byte_peek(req) != ' ')
		http_byte_next(req);
	u32	end = req._index;

	req.method.string = type::string(&req._bytes[begin], end - begin);
	req.method.kind = http_method_from_string(req.method.string);
	return (req._fail = (req.method.kind == http::METHOD_UNKNOWN));
}
static bool			parse_protocol_uri(http::request& req) {
	u32	begin = req._index;
	while (!http_bytes_done(req) && http_byte_peek(req) != ' ')
		http_byte_next(req);
	u32	end = req._index;

	req.uri = type::string(&req._bytes[begin], end - begin);
	return (req._fail = (req.uri.len == 0u));
}
static bool			parse_protocol_version(http::request& req) {
	u32	begin = req._index;
	while (!http_bytes_done(req) && http_byte_peek(req) != '\r' && http_byte_peek(req) != '\n')
		http_byte_next(req);
	u32	end = req._index;

	req.version = type::string(&req._bytes[begin], end - begin);
	return (req._fail = (req.version != "HTTP/1.1" || http_byte_peek(req) != '\r'));
}

static bool			parse_protocol(http::request& req) {
	/* Request-Line */
	if (parse_protocol_method(req)) return (true); /* Method */
	if (http_byte_next(req) != ' ') return (true); /* SP */
	if (parse_protocol_uri(req)) return (true); /* Request-URI */
	if (http_byte_next(req) != ' ') return (true); /* SP */
	if (parse_protocol_version(req)) return (true); /* HTTP-Version */
	if (http_byte_next(req) != '\r') return (true); /* CR */
	if (http_byte_next(req) != '\n') return (true); /* LF */
	return (false);
}
static bool			parse_core(http::request& req) {
	if (parse_protocol(req)) return (true);
	return (false);
}
static bool			parse_multipart(http::request& req) {
	return (req._fail);
}
static bool			parse_buffer(http::request& req) {
	return (req._fail);
}

http::request::request(void) {
	this->_index = 0u;
	this->_bytes = type::dynamic_array<byte>();
	this->_done = false;
	this->_fail = false;
	this->_multipart = false;

	this->method.kind = http::METHOD_UNKNOWN;
	this->method.string = type::string();
	this->uri = type::string();
	this->version = type::string();

	this->headers = http::headers();
	this->body = type::dynamic_array<byte>();
}
http::request::~request(void) {
}
http::request::request(const http::request& req) {
	*this = req;
}
http::request&	http::request::operator=(const http::request& req) {
	if (this != &req) {
		this->_index = req._index;
		this->_bytes = req._bytes;
		this->_done = req._done;
		this->_fail = req._fail;
		this->_multipart = req._multipart;

		this->method.kind = req.method.kind;
		this->method.string = req.method.string;
		this->uri = req.uri;
		this->version = req.version;

		this->headers = req.headers;
		this->body = req.body;
	}
	return (*this);
}

void			http::request::free(void) {
	this->_bytes.free();
	this->body.free();
	*this = http::request();
}
bool			http::request::parse(http::request& req, byte* bytes, u32 n) {
	if (req._done || req._fail || bytes == NULL || n == 0u)
		return (!req._fail);
	for (u32 i = 0u; i < n; ++i)
		req._bytes.push(bytes[i]);
	if (req._index == 0u) req._fail = parse_core(req);
	else if (req._multipart) req._fail = parse_multipart(req);
	else req._fail = parse_buffer(req);
	return (!req._fail);
}
