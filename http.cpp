/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   http.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sennakhl <sennakhl@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 17:21:41 by aindjare          #+#    #+#             */
/*   Updated: 2025/07/20 16:19:55 by sennakhl         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* REFERENCES:
* - https://www.rfc-editor.org/rfc/rfc1945
* - https://github.com/laytan/odin-http
*/

#include "webserv.hpp"

std::ostream&	operator<<(std::ostream& stream, const HTTP_Method& method) {
	switch (method) {
		case HTTP_METHOD_GET:
			return (stream << "GET");
		case HTTP_METHOD_POST:
			return (stream << "POST");
		case HTTP_METHOD_HEAD:
			return (stream << "HEAD");
		case HTTP_METHOD_DELETE:
			return (stream << "DELETE");
		case HTTP_METHOD_INVALID:
			return (stream << "INVALID");
		default:
			return (stream << "ESPECIALLYINVALID");
	}
}

HTTP_Method	parse_method(const std::string& text) {
	if (text == "GET") return (HTTP_METHOD_GET);
	if (text == "POST") return (HTTP_METHOD_POST);
	if (text == "HEAD") return (HTTP_METHOD_HEAD);
	if (text == "DELETE") return (HTTP_METHOD_DELETE);
	return (HTTP_METHOD_INVALID);
}

std::ostream&	operator<<(std::ostream& stream, const Parse_Error& error) {
	switch (error) {
		case PARSE_ERROR_NONE:
			return (stream << "Parse_Error{ Success }");
		case PARSE_ERROR_HEADER_MISSING_KEY:
			return (stream << "Parse_Error{ Header entry has no field name }");
		case PARSE_ERROR_NOT_DELIMITED:
			return (stream << "Parse_Error{ Could not find \"\\r\\n\" }");
		case PARSE_ERROR_NOT_ENOUGH_FIELDS:
			return (stream << "Parse_Error{ Not enough fields }");
		case PARSE_ERROR_SUCCESSIVE_SPACE:
			return (stream << "Parse_Error{ Successive spaces found }");
		case PARSE_ERROR_UNSUPPORTED_VERSION:
			return (stream << "Parse_Error{ Unsupported version }");
		case PARSE_ERROR_UNSUPPORTED_METHOD:
			return (stream << "Parse_Error{ Unsupported method }");
		case PARSE_ERROR_HEADER_SECTION_DELIMITER:
			return (stream << "Parse_Error{ Headers section is not delimited with \"\\r\\n\" }");
		case PARSE_ERROR_HEADER_ENTRY_INVALID_VALUE:
			return (stream << "Parse_Error{ Invalid character, most likely a CTL }");
		case PARSE_ERROR_HEADER_ENTRY_DELIMITER:
			return (stream << "Parse_Error{ Header entry is not delimited with \"\\r\\n\" }");
		case PARSE_ERROR_HEADER_ENTRY_INCOMPLETE:
			return (stream << "Parse_Error{ Header entry is missing ':' }");
		default:
			return (stream << "Parse_Error{ Unknown " << error << " }");
	}
}

Parse_Error	parse_request_line(Request& request, const std::string& msg) {
	size_t	offset = 0, next_space = 0;
	if (msg.find("  ") != std::string::npos) return (PARSE_ERROR_SUCCESSIVE_SPACE);

	if ((next_space = msg.find(' ', next_space)) == std::string::npos) return (PARSE_ERROR_NOT_ENOUGH_FIELDS);
	std::string	method = msg.substr(offset, next_space);
	request.method = parse_method(method);
	if (request.method == HTTP_METHOD_INVALID) return (PARSE_ERROR_UNSUPPORTED_METHOD);

	offset = next_space + 1;
	if ((next_space = msg.find(' ', offset)) == std::string::npos) return (PARSE_ERROR_NOT_ENOUGH_FIELDS);
	request.uri = msg.substr(offset, next_space - offset);

	offset = next_space + 1;
	if ((next_space = msg.find("\r\n", offset)) == std::string::npos) return (PARSE_ERROR_NOT_DELIMITED);
	request.version = msg.substr(offset, next_space - offset);
	if (request.version != "HTTP/1.0" && request.version != "HTTP/1.1") return (PARSE_ERROR_UNSUPPORTED_VERSION);

	return (PARSE_ERROR_NONE);
}

static bool	is_char(int c) {
	return (c >= 0 && c <= 127);
}

static bool is_tspecial(char c) {
	// tspecials      = '(' | ')' | '<' | '>' | '@' | ',' | ';' | ':' | '\' | '"' | '/' | '[' | ']' | '?' | '=' | '{' | '}' | SP | HT
	static const char SET[] = { '(',  ')',  '<',  '>',  '@', ',',  ';',  ':',  '\\',  '"',  '/',  '[',  ']',  '?',  '=',  '{',  '}',  ' ',  '\t' };
	for (size_t i = 0; i < sizeof(SET) / sizeof(SET[0]); i++)
		if (SET[i] == c)
			return (true);
	return (false);
}

static bool	is_ctl(char c) {
	// CTL            = <any US-ASCII control character (octets 0 - 31) and DEL (127)>
	return (c == 127 || (c >= 0 && c <= 31));
}

Parse_Error	parse_request_headers(Request& request, const std::string& msg) {
	(void)request;
	std::string	rest = msg;
	for (size_t begin = 0; !msg.substr(begin).empty();) {
		rest = msg.substr(begin);
		if (rest.find("\r\n") == 0) break ;
		std::string	name;
		{ // field-name
			size_t count = 0;
			const std::string& substr = msg.substr(begin);
			while (count < substr.size()) {
				if (!is_char(substr[count])) break ;
				if (is_tspecial(substr[count])) break ;
				if (is_ctl(substr[count])) break ;
				count++;
			}
			if (count < 1) return (PARSE_ERROR_HEADER_MISSING_KEY);
			name = msg.substr(begin, count);
			begin += count;
		}
		{ // ':'
			const std::string& substr = msg.substr(begin);
			if (substr.size() < 1 || substr[0] != ':') return (PARSE_ERROR_HEADER_ENTRY_INCOMPLETE);
			begin++;
		}
		std::string	value;
		{ // [ field-value ]
			size_t	count  = msg.substr(begin).find("\r\n");
			if (count == std::string::npos) return (PARSE_ERROR_HEADER_ENTRY_DELIMITER);
			value = msg.substr(begin, count);
			{ // internal validation
				size_t	i = 0;
				while (i < value.size()) {
					// field-content  = <the OCTETs making up the field-value and consisting of either *<any OCTET except CTLs, but including LWS> or combinations of 1*<any CHAR except CTLs>, tspecials>
					if (value[i] == '"') { // quoted-string  = ( '"' *<any CHAR except '"' and CTLs, but including LWS> '"' )
						size_t j = i++;
						while (i < value.size()) {
							if (!is_char(value[i])) break ;
							if (is_ctl(value[i]) && value[i] != ' ' && value[i] != '\t') break ;
							if (value[i] == '"') break ;
							i++;
						}

						size_t count = i - j;
						if (count <= 1 || value[i] != '"') i = j;
						else {
							i++;
							continue ;
						}
					}
					if (is_ctl(value[i])) return (PARSE_ERROR_HEADER_ENTRY_INVALID_VALUE);
					i++;
				}
			}
			begin += count;
		}
		{ // CRLF
			if (msg.substr(begin).find("\r\n") != 0) return (PARSE_ERROR_HEADER_ENTRY_DELIMITER);
			begin += 2;
		}
		request.headers[name] = value;
	}
	if (rest == "\r\n")
		return (PARSE_ERROR_NONE);
	return (PARSE_ERROR_HEADER_SECTION_DELIMITER);
}

Parse_Error	parse_request(const std::string& msg, Request& request) {
	size_t		count = 0, offset = 0;
	Parse_Error error;
	std::string	substr;

	count = msg.find("\r\n");
	if (count == std::string::npos) return (PARSE_ERROR_NOT_DELIMITED);
	else count = (count + 2) - offset;
	substr = msg.substr(offset, count);
	error = parse_request_line(request, substr); if (error != PARSE_ERROR_NONE) return (error);
	offset += count;

	count = msg.find("\r\n\r\n", offset);
	if (count == std::string::npos) return (PARSE_ERROR_NOT_DELIMITED);
	else count = (count + 4) - offset;
	substr = msg.substr(offset, count); // std::cout << substr << std::endl;
	error = parse_request_headers(request, substr); if (error != PARSE_ERROR_NONE) return (error);
	offset += count;

	// TODO(XENOBAS): Understand application/x-www-form-urlencoded
	// TODO(XENOBAS): Content-Length
	std::cout << "Offset: " << offset << ", Length: " << msg.size() << std::endl;
	request.body = msg.substr(offset);
	return (PARSE_ERROR_NONE);
}

#ifdef TEST_HTTP_MAIN
int	main(void) {
	return (0);
}
#endif
