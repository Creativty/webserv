/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv_http.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xenobas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 14:43:46 by xenobas           #+#    #+#             */
/*   Updated: 2025/12/15 16:53:51 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

WEBSERV_Method			WEBSERV_method_make(const string_view& str) {
	if (str == "GET") {
		return (WEBSERV_METHOD_GET);
	}
	if (str == "HEAD") {
		return (WEBSERV_METHOD_HEAD);
	}
	if (str == "POST") {
		return (WEBSERV_METHOD_POST);
	}
	if (str == "PUT") {
		return (WEBSERV_METHOD_PUT);
	}
	if (str == "DELETE") {
		return (WEBSERV_METHOD_DELETE);
	}
	if (str == "CONNECT") {
		return (WEBSERV_METHOD_CONNECT);
	}
	if (str == "OPTIONS") {
		return (WEBSERV_METHOD_OPTIONS);
	}
	if (str == "TRACE") {
		return (WEBSERV_METHOD_TRACE);
	}
	if (str == "PATCH") {
		return (WEBSERV_METHOD_PATCH);
	}
	return (WEBSERV_METHOD_INVALID);
}
const char*				WEBSERV_method_cstr(const WEBSERV_Method& method) {
	static const char*	strings[] = {
		"GET",
		"HEAD",
		"POST",
		"PUT",
		"DELETE",
		"CONNECT",
		"OPTIONS",
		"TRACE",
		"PATCH",

		"INVALID",
	};
	const i32			strings_count = cast(i32)count_of(strings);


	if (method == WEBSERV_METHOD_INVALID) {
		return (strings[strings_count - 1]);
	}
	for (i32 i = 0; i < strings_count - 1; ++i) {
		WEBSERV_Method	mask = cast(WEBSERV_Method)(1 << i);

		if ((method & mask) == method) {
			return (strings[i]);
		}
	}
	return (strings[strings_count - 1]);
}
b32						WEBSERV_http_route_method_test(const WEBSERV_Route& route, WEBSERV_Method method) {
	return (cast(b32)(method & route.methods_whitelist));
}

b32						WEBSERV_http_version_supported(const string_view& str) {
	return (
		str == "HTTP/1.1"
		|| str == "HTTP/1.0"
		|| str == "HTTP/0.9"
	);
}

string_view				WEBSERV_http_route_pick(const WEBSERV_Instance& instance, const string_view& path) {
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
string_view				WEBSERV_http_route_pick(const WEBSERV_Instance& instance, const HTTP_Request& req) {
	const dynamic_array<string_view>&	path_req = req.uri.path;
	unused(path_req);

	for_table_begin(instance.routes, const hash_table<WEBSERV_Route>, kv) {
		const string_view&		path_route = kv.key;
		const WEBSERV_Route&	route = kv.value;

		if (path_req.len == 0 || path_req[0] == "index.html") {
			if (path_route == "/" || path_route == "/index.html") {
				if (route.kind == WEBSERV_ROUTE_BASIC || route.kind == WEBSERV_ROUTE_REDIRECT) {
					return (path_route);
				}
				if (route.kind == WEBSERV_ROUTE_UPLOAD && path_req.len == 0 && path_route == "/") {
					return (path_route);
				}
			}
		}
		unused(path_route);
		unused(route);

		string_view	path_temp = kv.key;
		string_view	path_iter = string_view();
		i32			path_index = 0;
		while (path_temp.split_iter("/", path_iter) && path_index < path_req.len) {
			if (!path_iter) { /* NOTE(xenobas): Ignore initial split */
				continue ;
			}

			CLI_debug("[%2d] PATH_ITER = \"%.*s\"\tPATH_REQ  = \"%.*s\"", path_index, path_iter.len, path_iter.text, path_req[path_index].len, path_req[path_index].text);

			++path_index;
		}

	} for_table_end ;
	return (string_view(""));
}
