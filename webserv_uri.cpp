/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv_uri.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xenobas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 16:30:27 by xenobas           #+#    #+#             */
/*   Updated: 2025/11/01 15:32:00 by xenobas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "string_builder.hpp"

static WEBSERV_URI	WEBSERV_uri_make(const string_view& str) {
	WEBSERV_URI	uri;

	uri.path = string_view();
	uri.query = hash_table<string_view>();

	uri.str = str;
	uri.ok = 0;

	return (uri);
}
void				WEBSERV_uri_delete(WEBSERV_URI& uri) {
	uri.path.free();

	for (i32 i = 0; i < uri.query.cap; ++i) {
		hash_table<string_view>::hash_table_item&	item = uri.query.items[i];
		if (!item.used()) {
			continue ;
		}

		item.value.free();
	}
	uri.query.destroy();
}

static b32			WEBSERV_hex_match(byte b) {
	if (b >= 'A' && b <= 'F') {
		return (1);
	}
	if (b >= 'a' && b <= 'f') {
		return (1);
	}
	return (b >= '0' && b <= '9');
}

static i32			WEBSERV_hex_value(byte b) {
	if (b >= 'A' && b <= 'F') {
		b ^= (1 << 5);
	}

	if (b >= 'a' && b <= 'f') {
		return (10 + cast(i32)(b - 'a'));
	}
	else if (b >= '0' && b <= '9') {
		return (cast(i32)(b - '0'));
	}
	else {
		return (0);
	}
}

static b32			WEBSERV_uri_write_byte(string_builder& builder, const string_view& view) {
	if (view.len == 3) {
		u8 lb = cast(u8)WEBSERV_hex_value(view[1]) << 4;
		u8 rb = cast(u8)WEBSERV_hex_value(view[2]);
		byte ascii = cast(byte)(lb | rb);
		builder.write(cast(char)ascii);
		return (1);
	}
	builder.write(view);
	return (0);
}
static string_view	WEBSERV_uri_decode_comp(const string_view& str) {
	string_builder	builder;

	for (i32 i = 0; i < str.len; ) {
		i32	len = 1;
		if (str[i] == '%') {
			while (len < 3 && i + len < str.len) {
				if (!WEBSERV_hex_match(cast(byte)str[i + len])) {
					break ;
				}
				++len;
			}

			string_view	view = str.slice(i, i + len);
			WEBSERV_uri_write_byte(builder, view);
		} else {
			builder.write(str[i]);
		}
		i += len;
	}
	return (builder.to_string());
}

static void			WEBSERV_uri_debug(const WEBSERV_URI uri) {
	printf("String: \"%.*s\"\n", uri.str.len, uri.str.text);
	printf("Path: %.*s\n", uri.path.len, uri.path.text);
	printf("Query Table:\n");
	for (i32 i = 0; i < uri.query.cap; ++i) {
		hash_table<string_view>::item	iter = uri.query.items[i];
		if (!iter.used()) {
			continue ;
		}

		printf("  %.*s = %.*s\n", iter.key.len, iter.key.text, iter.value.len, iter.value.text);
	}
}

WEBSERV_URI			WEBSERV_uri_decode(const string_view& str) {
	WEBSERV_URI	uri = WEBSERV_uri_make(str);
	unused(WEBSERV_uri_debug); /* NOTE(xenobas): To avoid unused static function error */

	if (str.has_prefix("/")) {
		/* DESCRIPTION(xenobas): Discard URI fragment section */
		i32	index_fragment = uri.str.index("#");
		if (index_fragment > -1) {
			uri.str = uri.str.slice(0, index_fragment);
		}

		string_view	view_path = uri.str;
		string_view	view_query = "";

		i32			index_query = view_path.index("?");
		if (index_query > -1) {
			view_path = uri.str.slice(0, index_query);
			view_query = uri.str.slice(index_query + 1);
		}

		string_view	pair;
		while (view_query.split_iter("&", pair)) {
			string_view	view_key, view_val;

			pair.split_iter("=", view_key);
			pair.split_iter("=", view_val);

			if (view_key.len == 0) {
				continue ;
			}
			string_view	key = WEBSERV_uri_decode_comp(view_key);
			string_view	val = WEBSERV_uri_decode_comp(view_val);

			uri.query.set(key, val);
			key.free();
		}
		uri.path = WEBSERV_uri_decode_comp(view_path);
		uri.ok = 1;
	}
	return (uri);
}
string_view			WEBSERV_uri_encode(const WEBSERV_URI& uri) {
	string_view	str;

	if (!uri.ok)
		return ("");
	return (str);
}
