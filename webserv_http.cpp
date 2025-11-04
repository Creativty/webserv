/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv_http.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xenobas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 14:43:46 by xenobas           #+#    #+#             */
/*   Updated: 2025/11/04 13:07:13 by xenobas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

WEBSERV_Method	WEBSERV_method_make(const string_view& str) {
	if (str == "GET") {
		return (WEBSERV_METHOD_GET);
	}
	if (str == "PUT") {
		return (WEBSERV_METHOD_PUT);
	}
	if (str == "POST") {
		return (WEBSERV_METHOD_POST);
	}
	return (WEBSERV_METHOD_INVALID);
}


b32				WEBSERV_http_version_supported(const string_view& str) {
	return (
		str == "HTTP/1.1"
		|| str == "HTTP/1.0"
		|| str == "HTTP/0.9"
	);
}
