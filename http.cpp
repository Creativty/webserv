/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   http.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 17:21:41 by aindjare          #+#    #+#             */
/*   Updated: 2025/07/17 19:36:15 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* REFERENCES:
* - https://www.rfc-editor.org/rfc/rfc1945
* - https://github.com/laytan/odin-http
*/

#include "webserv.hpp"
#include <map>
#include <string>
#include <ostream>
#include <iostream>

namespace http {
	enum HTTP_Method {
		HTTP_METHOD_INVALID,
		HTTP_METHOD_GET,
		HTTP_METHOD_POST,
		HTTP_METHOD_HEAD,
		HTTP_METHOD_DELETE,
	};

	HTTP_Method	parse_method(const std::string& text) {
		if (text == "GET") return (HTTP_METHOD_GET);
		if (text == "POST") return (HTTP_METHOD_POST);
		if (text == "HEAD") return (HTTP_METHOD_HEAD);
		if (text == "DELETE") return (HTTP_METHOD_DELETE);
		return (HTTP_METHOD_INVALID);
	}
	
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

	struct Request {
		HTTP_Method							method;
		std::string							uri;
		std::string							version;

		std::map<std::string, std::string>	headers;
		std::string							body;
	};

	struct Token {
		int			line, column;
		std::string	text;
	};

	enum Parse_Error {
		PARSE_ERROR_NONE,
		PARSE_ERROR_NOT_DELIMITED,
		PARSE_ERROR_SUCCESSIVE_SPACE,
		PARSE_ERROR_NOT_ENOUGH_FIELDS,
		PARSE_ERROR_UNSUPPORTED_METHOD,
		PARSE_ERROR_UNSUPPORTED_VERSION,
	};

	std::ostream&	operator<<(std::ostream& stream, const Parse_Error& error) {
		switch (error) {
		case PARSE_ERROR_NONE:
			return (stream << "Parse_Error{ Success }");
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
		if (request.version != "HTTP/1.0") return (PARSE_ERROR_UNSUPPORTED_VERSION);

		return (PARSE_ERROR_NONE);
	}
};

#ifdef HTTP_MAIN
int	main(void) {
	http::Request		request;
	http::Parse_Error	err = http::parse_request_line(request, "GET /path?version HTTP/1.0\r\n");

	std::cout << "ERROR: " << err << std::endl;
	return (0);
}
#endif
