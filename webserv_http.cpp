/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv_http.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xenobas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 14:43:46 by xenobas           #+#    #+#             */
/*   Updated: 2025/11/14 18:36:24 by xenobas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

WEBSERV_Method			WEBSERV_method_make(const string_view& str) {
	if (str == "GET") {
		return (WEBSERV_METHOD_GET);
	}
	if (str == "DELETE") {
		return (WEBSERV_METHOD_DELETE);
	}
	if (str == "PUT") {
		return (WEBSERV_METHOD_PUT);
	}
	if (str == "POST") {
		return (WEBSERV_METHOD_POST);
	}
	return (WEBSERV_METHOD_INVALID);
}

b32						WEBSERV_http_version_supported(const string_view& str) {
	return (
		str == "HTTP/1.1"
		|| str == "HTTP/1.0"
		|| str == "HTTP/0.9"
	);
}

const string_view		WEBSERV_http_route_pick(const WEBSERV_Instance& instance, const string_view& path) {
	string_view	best_match;

	string_view	path_symbolic = path;
	if (path_symbolic.has_suffix("index.html")) { /* index.html = 10 bytes */
		path_symbolic = path_symbolic.slice(0, path_symbolic.len - 10);
	}

	i32			path_len = path_symbolic.len;
	while (path_len > 1) {
		if (path_symbolic[path_len - 1] != '/') {
			break ;
		}
		--path_len;
	}

	path_symbolic = path_symbolic.slice(0, path_len);
	/* TODO(xenobas): This needs to be tested with directory paths and the likes */
	for_table_begin(instance.routes, const hash_table<WEBSERV_Route>, kv) {
		const string_view&		key = kv.key;
		const WEBSERV_Route&	route = kv.value;

		if (!route.cascade) {
			if (key == path_symbolic) {
				return (path_symbolic);
			}
			continue ;
		}
		if (!path_symbolic.has_prefix(key)) {
			continue ;
		}
		if (key.len < best_match.len) {
			continue ;
		}

		best_match = key;
	} for_table_end ;
	return (best_match);
}
