/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv_uri.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xenobas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 16:30:27 by xenobas           #+#    #+#             */
/*   Updated: 2025/12/18 16:23:17 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "string_builder.hpp"

WEBSERV_URI			WEBSERV_uri_make(const string_view& str) {
	WEBSERV_URI	uri;

	uri.path = dynamic_array<string_view>();
	uri.params = hash_table<string_view>();
	uri.is_file = !str.has_suffix("/");

	uri.str = string_view::alloc(str);
	uri.ok = 0;

	return (uri);
}
void				WEBSERV_uri_delete(WEBSERV_URI& uri) {
	for (i32 i = 0; i < uri.path.len; ++i) {
		uri.path[i].free();
	}
	uri.path.free();

	for_table_begin(uri.params, hash_table<string_view>, param) {
		param.value.free();
	} for_table_end ;
	uri.params.free();

	uri.str.free();
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
static char			WEBSERV_hex_repr(u8 n) {
	if (n > 0xf) {
		return '_';
	}

	if (n >= 0xa) {
		return (cast(char)(n - 10) + 'A');
	}

	return (cast(char)n + '0');
}

static b32			WEBSERV_uri_write_byte(string_builder& builder, const string_view& view) {
	if (view.len == 3) {
		u8 lb = cast(u8)((WEBSERV_hex_value(cast(byte)view[1])) << (4 * 1));
		u8 rb = cast(u8)((WEBSERV_hex_value(cast(byte)view[2])) << (4 * 0));
		byte ascii = cast(byte)(lb | rb);
		builder.write(cast(char)ascii);
		return (1);
	}
	builder.write(view);
	return (0);
}
static string_view	WEBSERV_uri_decode_comp(const string_view& str, b32 is_query_param = 0) {
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
		}
		else if (str[i] == '+' && is_query_param) {
			builder.write(' ');
		}
		else {
			builder.write(str[i]);
		}
		i += len;
	}
	return (builder.to_string());
}
static void			WEBSERV_uri_encode_comp(string_builder& builder, char c) {
	u8		lb = ((cast(u8)c) & 0xf0) >> (4 * 1);
	u8		rb = ((cast(u8)c) & 0x0f) >> (4 * 0);
	
	char	lc = WEBSERV_hex_repr(lb);
	char	rc = WEBSERV_hex_repr(rb);

	builder.write('%');
	builder.write(lc);
	builder.write(rc);
}
static b32			WEBSERV_uri_encode_required(char c) {
	switch (c) {
	/* gen-delims */
	case ':':
	case '/':
	case '?':
	case '#':
	case '[':
	case ']':
	case '@':
	/* sub-delims */
	case '!':
	case '$':
	case '&':
	case '\'':
	case '(':
	case ')':
	case '*':
	case '+':
	case ',':
	case ';':
	case '=':
		return (1);
	default:
		/* NOTE(XENOBAS): Anything that does not match [0-9a-zA-Z_-.] will be percent encoding required, as is per convention */
		return (!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'z') && !(c >= 'A' && c <= 'Z') && c != '_' && c != '-' && c != '.');
	}
}
static void			WEBSERV_uri_encoded_write(string_builder& builder, const string_view& str) {
	for (i32 i = 0; i < str.len; ++i) {
		if (WEBSERV_uri_encode_required(str[i])) {
			WEBSERV_uri_encode_comp(builder, str[i]);
		} else {
			builder.write(str[i]);
		}
	}
}

static void			WEBSERV_uri_debug(const WEBSERV_URI& uri) {
	std::printf("String: \"%.*s\"\n", uri.str.len, uri.str.text);
	std::printf("Path: [ ");
	for	(i32 i = 0; i < uri.path.len; ++i) {
		std::printf("\"%.*s\"", uri.path[i].len, uri.path[i].text);
		if (i + 1 < uri.path.len) {
			std::printf(", ");
		}
	}
	std::printf(" ]\n");
	std::printf("params Table:\n");
	for_table_begin(uri.params, hash_table<string_view>, param) {
		std::printf("  %.*s = %.*s\n", param.key.len, param.key.text, param.value.len, param.value.text);
	} for_table_end ;
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
		string_view	view_params = "";

		i32			index_params = view_path.index("?");
		if (index_params > -1) {
			view_path = uri.str.slice(0, index_params);
			view_params = uri.str.slice(index_params + 1);
		}

		string_view	comp;
		while (view_path.split_iter("/", comp)) {
			if (comp.len == 0) {
				continue ;
			}
			string_view	node = WEBSERV_uri_decode_comp(comp);
			uri.path.push(node);
		}

		string_view	pair;
		while (view_params.split_iter("&", pair)) {
			string_view	view_key, view_val;

			pair.split_iter("=", view_key);
			pair.split_iter("=", view_val);

			if (view_key.len == 0) {
				continue ;
			}
			string_view	key = WEBSERV_uri_decode_comp(view_key, /* is_query_param = */ 1);
			string_view	val = WEBSERV_uri_decode_comp(view_val, /* is_query_param = */ 1);

			uri.params.set(key, val);
			key.free();
		}

		uri.ok = 1;
	}
	return (uri);
}

/* DANGER(xenobas): Does not encode query parameters properly using form style encoding */
string_view			WEBSERV_uri_encode(const WEBSERV_URI& uri, b32 write_trailing_slash) {
	string_builder	builder;

	if (uri.ok) { /* NOTE(xenobas): This is a naive implementation */
		/* SECTION: Path */
		builder.write('/');
		for (i32 path_index = 0; path_index < uri.path.len; ++path_index) {
			if (path_index > 0) {
				builder.write('/');
			}
			const string_view&	comp = uri.path[path_index];
			WEBSERV_uri_encoded_write(builder, comp);
		}
		if (uri.path.len > 0 && write_trailing_slash) {
			builder.write('/');
		}

		/* SECTION: params */
		u32	count = 0;
		if (uri.params.count > 0) {
			builder.write('?');
		}
		for_table_begin(uri.params, const hash_table<string_view>, param) {
			if (count > 0) {
				builder.write('&');
			}
			++count;

			WEBSERV_uri_encoded_write(builder, param.key);
			if (param.value) {
				builder.write('=');
				WEBSERV_uri_encoded_write(builder, param.value);
			}
		} for_table_end ;
	}
	return (builder.to_string());
}
