/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv_http.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xenobas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 14:43:46 by xenobas           #+#    #+#             */
/*   Updated: 2025/12/20 23:25:39 by xenobas          ###   ########.fr       */
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

string_view				WEBSERV_http_route_pick(const WEBSERV_Instance& instance, const HTTP_Request& req) {
	const dynamic_array<string_view>&	req_path = req.uri.path;

	string_view							path = string_view();
	i32									path_score = 0;
	for_table_begin(instance.routes, const hash_table<WEBSERV_Route>, kv) {
		const WEBSERV_Route&				route = kv.value;
		i32									route_score = 0;

		i32									req_path_len = req_path.len;
		if (route.path.has_prefix(".")) { /* NOTE(xenobas): Extension match */
			if (req_path_len <= 0) {
				continue ;
			}
			const string_view&	last = req_path[req_path_len - 1];
			if (last.has_suffix(route.path)) {
				path = kv.key;
				path_score = req_path_len;
				break ;
			}
			continue ;
		}

		WEBSERV_URI							route_uri = WEBSERV_uri_decode(kv.key);
		const dynamic_array<string_view>&	route_path = route_uri.path;
		i32									route_path_len = route_path.len;
		if (route.kind == WEBSERV_ROUTE_SERVER
			&& req_path_len > 0
			&& route_path_len > 0
			&& !req.uri.is_file
			&& req_path[req_path_len - 1] == route.Server.fallback
			&& route_path[route_path_len - 1] == route.Server.fallback)
		{
			req_path_len--;
			route_path_len--;
		}
		for (; route_score < route_path_len && route_score < req_path_len; ++route_score) {
			if (req_path[route_score] != route_path[route_score]) {
				break ;
			}
		}

		WEBSERV_uri_delete(route_uri);

		if ((route_score > path_score) || (path_score == 0 && route_score == 0 && route_path_len == 0)) {
			if (route.cascade || route_score == route_path_len) {
				path = kv.key;
				path_score = route_score;
			}
		}
	} for_table_end;

	return (path);
}
