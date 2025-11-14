/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv_config.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xenobas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 13:42:31 by xenobas           #+#    #+#             */
/*   Updated: 2025/11/14 16:24:43 by xenobas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

typedef WEBSERV_Route				Route;
typedef WEBSERV_Route_Kind			Route_Kind;

typedef WEBSERV_Route_Basic			Route_Basic;
typedef WEBSERV_Route_Redirect		Route_Redirect;
typedef WEBSERV_Route_Upload		Route_Upload;
typedef WEBSERV_Route_CGI			Route_CGI;

typedef WEBSERV_Config				Config;
typedef WEBSERV_Instance			Instance;

typedef WEBSERV_Config_Error		Error;
typedef WEBSERV_Config_Error_Kind	Error_Kind;

struct					Parser_Context {
	b32			did_parse_variant;
	Instance&	instance;
	Config&		config;

	b32			ok;
};

#define					config_error(VARIANT, ...) \
					WEBSERV_config_error(config, WEBSERV_CONFIG_ERROR_##VARIANT, __VA_ARGS__)

#define					context_error(VARIANT, ...) \
	do { \
		ctx.ok = 0; \
		WEBSERV_config_error(ctx.config, WEBSERV_CONFIG_ERROR_##VARIANT, __VA_ARGS__); \
	} while (0)

static const char*		toml_value_kind_strings[] = {
#define TOML_VALUE_KIND(NAME, STRING, ...) STRING,
	TOML_VALUE_KINDS
#undef TOML_VALUE_KIND
};

static void				WEBSERV_config_error(Config& config, Error_Kind kind) {
	Error	error;

	error.kind = kind;
	error.pos = (Position){ 0, 1, 1, config.document.file };

	error.str = string_view();
	error.value = 0;
	error.token = (TOML_Token){ TOML_TOKEN_INVALID, string_view(), error.pos };

	config.ok = 0;
	config.errors.push(error);
}
static void				WEBSERV_config_error(Config& config, Error_Kind kind, const Position& pos) {
	Error	error;

	error.kind = kind;
	error.pos = pos;

	error.str = string_view();
	error.value = 0;
	error.token = (TOML_Token){ TOML_TOKEN_INVALID, string_view(), error.pos };

	config.ok = 0;
	config.errors.push(error);
}
static void				WEBSERV_config_error(Config& config, Error_Kind kind, const Position& pos, const string_view& str) {
	Error	error;

	error.kind = kind;
	error.pos = pos;

	error.str = str;
	error.value = 0;
	error.token = (TOML_Token){ TOML_TOKEN_INVALID, string_view(), error.pos };

	config.ok = 0;
	config.errors.push(error);
}
static void				WEBSERV_config_error(Config& config, Error_Kind kind, const TOML_Value& value) {
	Error	error;

	error.kind = kind;
	error.pos = value.pos;

	error.str = string_view();
	error.value = (TOML_Value*)&value;
	error.token = (TOML_Token){ TOML_TOKEN_INVALID, string_view(), error.pos };

	config.ok = 0;
	config.errors.push(error);
}
static void				WEBSERV_config_error(Config& config, Error_Kind kind, const TOML_Value& value, const string_view& str) {
	Error	error;

	error.kind = kind;
	error.pos = value.pos;

	error.str = str;
	error.value = (TOML_Value*)&value;
	error.token = (TOML_Token){ TOML_TOKEN_INVALID, "", error.pos };

	config.ok = 0;
	config.errors.push(error);
}

static Route			WEBSERV_route_make(void) {
	Route	route;
	MEM_zero(route);

	route.cascade = 1;
	return (route);
}
static void				WEBSERV_route_delete(Route& route) {
	switch (route.kind) {
		case WEBSERV_ROUTE_BASIC: {
			Route_Basic&	basic = route.Basic;

			basic.directory.free();
		} break ;
		case WEBSERV_ROUTE_REDIRECT: {
			Route_Redirect&	redirect = route.Redirect;

			redirect.location.free();
		} break ;
		case WEBSERV_ROUTE_UPLOAD: {
			Route_Upload&	upload = route.Upload;

			upload.directory.free();
		} break ;
		case WEBSERV_ROUTE_CGI: {
			Route_CGI&	cgi = route.CGI;

			/* CGI Binaries */
			for_table_begin(cgi.interpreters, hash_table<string_view>, entry) {
				entry.value.free();
			} for_table_end ;
			cgi.interpreters.free();

			/* CGI Environment */
			for_table_begin(cgi.env, hash_table<string_view>, entry) {
				entry.value.free();
			} for_table_end ;
			cgi.env.free();
		} break ;
		case WEBSERV_ROUTE_COUNT:
		case WEBSERV_ROUTE_INVALID:
		default: {
		} break ;
	}
	route.path.free();
}

static Route_Basic		WEBSERV_route_basic_make(void) {
	Route_Basic	basic;
	MEM_zero(basic);

	basic.directory = string_view();
	basic.directory_list = 0;
	return (basic);
}
static Route_Redirect	WEBSERV_route_redirect_make(void) {
	Route_Redirect	redirect;
	MEM_zero(redirect);

	redirect.location = string_view();
	return (redirect);
}
static Route_Upload		WEBSERV_route_upload_make(void) {
	Route_Upload	upload;
	MEM_zero(upload);

	upload.directory = string_view();
	upload.max_file_size = I32_MAX;
	return (upload);
}
static Route_CGI		WEBSERV_route_cgi_make(void) {
	Route_CGI	cgi;
	MEM_zero(cgi);

	cgi.interpreters = hash_table<string_view>();
	cgi.env = hash_table<string_view>();
	return (cgi);
}

static Instance			WEBSERV_instance_make(void) {
	Instance	instance;

	instance.interfaces = dynamic_array<WEBSERV_Interface>();

	instance.port      = 0;
	instance.addr.blob = 0;

	instance.request_body_max = U32_MAX;

	instance.error_4xx = string_view();
	instance.error_5xx = string_view();

	instance.routes = hash_table<WEBSERV_Route>();
	return (instance);
}
static void				WEBSERV_instance_delete(Instance& instance) {
	instance.error_4xx.free();
	instance.error_5xx.free();

	instance.interfaces.free();

	for_table_begin(instance.routes, hash_table<Route>, item) {
		WEBSERV_route_delete(item.value);
	} for_table_end ;
	instance.routes.free();
}

static Config			WEBSERV_config_make(const TOML_Document& document) {
	Config	config;

	config.document = document;

	config.ok = 1;
	config.errors = dynamic_array<Error>();
	config.instances = dynamic_array<WEBSERV_Instance>();
	return (config);
}
void					WEBSERV_config_delete(Config& config) {
	for (i32 i = 0; i < config.instances.len; ++i) {
		WEBSERV_instance_delete(config.instances[i]);
	}
	config.instances.free();
	config.errors.free();
	config.ok = 1;

	TOML_delete(config.document);
}

static b32				CONTEXT_parse_expect(Parser_Context& ctx, const TOML_Value& value, TOML_Value_Kind expected, string_view msg = "") {
	if (value.kind == expected) {
		return (1);
	}

	if (!(bool)msg) {
		msg = toml_value_kind_strings[expected];
	}

	context_error(TYPE_MISMATCH, value, msg);
	return (0);
}

static void				WEBSERV_config_parse_port(Parser_Context& ctx, const TOML_Value& value) {
	Instance&	instance = ctx.instance;

	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_NUMBER)) {
		return ;
	}

	i64	port_signed = value.Number;
	if (port_signed < 0 || port_signed > 65535) {
		context_error(PORT_RANGE, value.pos);
		return ;
	}

	instance.port = cast(u16)port_signed;
}
static void				WEBSERV_config_parse_host(Parser_Context& ctx, const TOML_Value& value) {
	Config&		config = ctx.config;
	Instance&	instance = ctx.instance;

	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_STRING)) {
		return ;
	}
	if (value.String == 0 || !(bool)(*(value.String))) {
		config_error(STRING_EMPTY, value.pos, "ipv4 address");
		return ;
	}

	string_view	host_string = *value.String;
	if (host_string.count('.') != 3) {
		config_error(VALUE_INVALID, value.pos, "ipv4 address");
		return ;
	}

	string_view		byte_string;
	i32				byte_index = 0;
	WEBSERV_Address	addr = { .blob = 0 };
	while (host_string.split_iter(".", byte_string) && byte_index < 4) {
		i32	byte_value = 0;
		b32	is_leading_zero = 1;
		b32	has_leading_zero = 0;
		b32	byte_ok = byte_string.len > 0;
		for (i32 i = 0; i < byte_string.len; ++i, is_leading_zero = 0) {
			char	digit = byte_string[i];
			if (digit < '0' || digit > '9') {
				byte_ok = 0;
				break ;
			}
			if (digit == '0' && is_leading_zero) {
				has_leading_zero = 1;
			}
			byte_value = (byte_value * 10) + (digit - '0');
		}
		if (!byte_ok || byte_value > 255 || (has_leading_zero && byte_string.len > 1)) {
			break ;
		}

		addr.bytes[byte_index++] = cast(u8)byte_value;
	}
	if ((bool)host_string || byte_index != 4) {
		config_error(VALUE_INVALID, value.pos, "ipv4 address");
		return ;
	}

	instance.addr = addr;
}
static void				WEBSERV_config_parse_error_4xx(Parser_Context& ctx, const TOML_Value& value) {
	Config&		config = ctx.config;
	Instance&	instance = ctx.instance;

	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_STRING)) {
		return ;
	}
	if (value.String == 0 || !(bool)(*(value.String))) {
		config_error(STRING_EMPTY, value.pos, "html file path");
		return ;
	}

	string_view	path = *value.String;
	struct stat	stat;
	if (!path.has_suffix(".html") || !OS_stat_file(path, &stat) || !OS_access_file(path) || !(S_ISREG(stat.st_mode))) {
		config_error(ERROR_FILE, value.pos, path);
		return ;
	}

	instance.error_4xx = string_view::alloc(path);
}
static void				WEBSERV_config_parse_error_5xx(Parser_Context& ctx, const TOML_Value& value) {
	Config&		config = ctx.config;
	Instance&	instance = ctx.instance;

	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_STRING)) {
		return ;
	}
	if (value.String == 0 || !(bool)(*(value.String))) {
		context_error(STRING_EMPTY, value.pos, "html file path");
		return ;
	}

	string_view	path = *value.String;
	struct stat	stat;
	if (!path.has_suffix(".html") || !OS_stat_file(path, &stat) || !OS_access_file(path) || !(S_ISREG(stat.st_mode))) {
		config_error(ERROR_FILE, value.pos, path);
		return ;
	}

	instance.error_5xx = string_view::alloc(path);
}
static void				WEBSERV_config_parse_request_body_max(Parser_Context& ctx, const TOML_Value& value) {
	Instance&	instance = ctx.instance;

	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_NUMBER)) {
		return ;
	}

	i64	request_body_max = value.Number;
	if (request_body_max < 0 || request_body_max > I32_MAX) {
		context_error(REQUEST_BODY_MAX_RANGE, value.pos);
		return ;
	}

	instance.request_body_max = cast(u32)request_body_max;
}

static Route_Kind		WEBSERV_route_kind_match(const string_view& str) {
	if (str.eq_insensitive("server")) {
		return (WEBSERV_ROUTE_BASIC);
	} else if (str.eq_insensitive("redirect")) {
		return (WEBSERV_ROUTE_REDIRECT);
	} else if (str.eq_insensitive("upload")) {
		return (WEBSERV_ROUTE_UPLOAD);
	} else if (str.eq_insensitive("cgi")) {
		return (WEBSERV_ROUTE_CGI);
	} else {
		return (WEBSERV_ROUTE_INVALID);
	}
}
static b32				WEBSERV_config_parse_route_path(Parser_Context& ctx, Route& route, const TOML_Value& value) {

	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_STRING)) {
		return (0);
	}
	if (value.String == 0 || !(bool)(*value.String)) {
		context_error(STRING_EMPTY, value.pos, "uri relative path");
		return (0);
	}

	const string_view&	str = *(value.String);
	if (!str.has_prefix("/")) {
		context_error(VALUE_INVALID, value.pos, "uri relative path");
		return (0);
	}

	route.path = string_view::alloc(str);
	return (1);
}
static b32				WEBSERV_config_parse_route_cascade(Parser_Context& ctx, Route& route, const TOML_Value& value) {
	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_BOOLEAN)) {
		return (0);
	}

	route.cascade = !value.Boolean;
	return (1);
}
static b32				WEBSERV_config_parse_route_kind(Parser_Context& ctx, Route& route, const TOML_Value& value) {
	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_STRING)) {
		return (0);
	}
	if (value.String == 0 || !(bool)(*value.String)) {
		context_error(ROUTE_TYPE, value);
		return (0);
	}

	Route_Kind	kind = WEBSERV_route_kind_match(*(value.String));
	if (kind == WEBSERV_ROUTE_INVALID) {
		context_error(ROUTE_TYPE, value);
		return (0);
	}

	route.kind = kind;

	return (1);
}

/* route basic parsers */
static void				WEBSERV_route_parse_basic_directory(Parser_Context& ctx, WEBSERV_Route_Basic& basic, const TOML_Value& value) {
	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_STRING)) {
		return ;
	}
	if (value.String == 0 || !(bool)(*value.String)) {
		context_error(VALUE_INVALID, value.pos, "directory path");
		return ;
	}

	/* TODO(xenobas): directory checks */
	const string_view&	directory = *value.String;
	basic.directory = string_view::alloc(directory);
}
static void				WEBSERV_route_parse_basic_directory_list(Parser_Context& ctx, WEBSERV_Route_Basic& basic, const TOML_Value& value) {
	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_BOOLEAN)) {
		return ;
	}

	basic.directory_list = value.Boolean;
}
/* route redirect parsers */
static void				WEBSERV_route_parse_redirect_location(Parser_Context& ctx, WEBSERV_Route_Redirect& redirect, const TOML_Value& value) {
	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_STRING)) {
		return ;
	}
	if (value.String == 0 || !(bool)(*value.String)) {
		context_error(VALUE_INVALID, value.pos, "redirection location uri");
		return ;
	}

	const string_view&	location = *value.String;
	redirect.location = string_view::alloc(location); /* NOTE(xenobas): This probably should be a uri?!? */
}
/* route upload parsers */
static void				WEBSERV_route_parse_upload_directory(Parser_Context& ctx, WEBSERV_Route_Upload& upload, const TOML_Value& value) {
	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_STRING)) {
		return ;
	}
	if (value.String == 0 || !(bool)(*value.String)) {
		context_error(VALUE_INVALID, value.pos, "directory path");
		return ;
	}

	/* TODO(xenobas): directory checks */
	const string_view&	directory = *value.String;
	upload.directory = string_view::alloc(directory);
}
static void				WEBSERV_route_parse_upload_max_file_size(Parser_Context& ctx, WEBSERV_Route_Upload& upload, const TOML_Value& value) {
	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_NUMBER)) {
		return ;
	}
	i64	max_file_size = value.Number;
	if (max_file_size < 0 || max_file_size > cast(i64)I32_MAX) {
		context_error(VALUE_INVALID, value.pos, "size in bytes");
		return ;
	}

	upload.max_file_size = cast(i32)max_file_size;
}
/* route cgi parsers */
static void				WEBSERV_route_parse_cgi_interpreters(Parser_Context& ctx, Route_CGI& cgi, const TOML_Value& value) {
	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_TABLE)) {
		return ;
	}
	if (value.Table == 0) {
		context_error(VALUE_INVALID, value.pos, "interpreters table { ext = path, ... }");
		return ;
	}

	/* TODO(xenobas): Check for all interpreters executability and existence */
	const TOML_Table&	table = *value.Table;

	cgi.interpreters.resize(table.cap);
	for_table_begin(table, const TOML_Table, kv) {
		const TOML_Value&	value = kv.value;
		if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_STRING, "interpreter path")) {
			continue ;
		}
		if (value.String == 0 || !(bool)(*value.String)) {
			context_error(VALUE_INVALID, value.pos, "interpreter path");
			continue ;
		}

		string_view	path = string_view::alloc(*value.String);
		cgi.interpreters.set(kv.key, path);
	} for_table_end ;
}
static void				WEBSERV_route_parse_cgi_env(Parser_Context& ctx, Route_CGI& cgi, const TOML_Value& value) {
	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_TABLE)) {
		return ;
	}
	if (value.Table == 0) {
		context_error(VALUE_INVALID, value.pos, "environment table { KEY = VALUE, ... }");
		return ;
	}

	const TOML_Table&	table = *value.Table;

	cgi.env.resize(table.cap);
	for_table_begin(table, const TOML_Table, kv) {
		const TOML_Value&	value = kv.value;
		if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_STRING, "environment value")) {
			continue ;
		}

		string_view	ref = string_view();
		if (value.String != 0) {
			ref = *value.String;
		}

		string_view	val = string_view::alloc(ref);
		cgi.env.set(kv.key, val);
	} for_table_end ;
}
/* route variants parsers */
static b32				WEBSERV_route_parse_basic(Parser_Context& ctx, Route& route, const TOML_Value& value) {
	typedef void	(*Entry_Proc)(Parser_Context&, WEBSERV_Route_Basic&, const TOML_Value&);
	typedef struct {
		string_view	key;
		Entry_Proc	proc;

		b32			is_required;
		b32			is_seen;
	}	Entry;

	Entry	entries[] = {
		{ "directory", WEBSERV_route_parse_basic_directory, /* required = */ 1, 0 },
		{ "directory_list", WEBSERV_route_parse_basic_directory_list, /* required = */ 0, 0 },
	};

	b32	ok_old = ctx.ok;
	ctx.ok = 1;

	route.Basic = WEBSERV_route_basic_make();
	const TOML_Table&	table = *value.Table;
	for_table_begin(table, const TOML_Table, kv) {
		const string_view&	key = kv.key;
		const TOML_Value&	value = kv.value;

		i32	idx = -1;
		for (i32 i = 0; i < cast(i32)count_of(entries); ++i) {
			if (key == entries[i].key) {
				idx = i;
				break ;
			}
		}
		if (idx == -1) {
			context_error(KEY_UNKNOWN, value.pos, key);
			continue ;
		}

		entries[idx].proc(ctx, route.Basic, value);
		entries[idx].is_seen = 1;
	} for_table_end ;

	for (i32 i = 0; i < cast(i32)count_of(entries); ++i) {
		if (entries[i].is_seen || !entries[i].is_required) {
			continue ;
		}

		context_error(KEY_REQUIRED, value.pos, entries[i].key);
	}

	b32	ok_new = ctx.ok;
	ctx.ok = ok_old && ok_new;

	return (ctx.ok);
}
static b32				WEBSERV_route_parse_redirect(Parser_Context& ctx, Route& route, const TOML_Value& value) {
	typedef void	(*Entry_Proc)(Parser_Context&, WEBSERV_Route_Redirect&, const TOML_Value&);
	typedef struct {
		string_view	key;
		Entry_Proc	proc;

		b32			is_required;
		b32			is_seen;
	}	Entry;

	Entry	entries[] = {
		{ "location", WEBSERV_route_parse_redirect_location, /* required = */ 1, 0 },
	};

	b32	ok_old = ctx.ok;
	ctx.ok = 1;

	route.Redirect = WEBSERV_route_redirect_make();
	const TOML_Table&	table = *value.Table;
	for_table_begin(table, const TOML_Table, kv) {
		const string_view&	key = kv.key;
		const TOML_Value&	value = kv.value;

		i32	idx = -1;
		for (i32 i = 0; i < cast(i32)count_of(entries); ++i) {
			if (key == entries[i].key) {
				idx = i;
				break ;
			}
		}
		if (idx == -1) {
			context_error(KEY_UNKNOWN, value.pos, key);
			continue ;
		}

		entries[idx].proc(ctx, route.Redirect, value);
		entries[idx].is_seen = 1;
	} for_table_end ;

	for (i32 i = 0; i < cast(i32)count_of(entries); ++i) {
		if (entries[i].is_seen || !entries[i].is_required) {
			continue ;
		}

		context_error(KEY_REQUIRED, value.pos, entries[i].key);
	}

	b32	ok_new = ctx.ok;
	ctx.ok = ok_old && ok_new;

	return (ctx.ok);
}
static b32				WEBSERV_route_parse_upload(Parser_Context& ctx, Route& route, const TOML_Value& value) {
	typedef void	(*Entry_Proc)(Parser_Context&, WEBSERV_Route_Upload&, const TOML_Value&);
	typedef struct {
		string_view	key;
		Entry_Proc	proc;

		b32			is_required;
		b32			is_seen;
	}	Entry;

	Entry	entries[] = {
		{ "directory", WEBSERV_route_parse_upload_directory, /* required = */ 1, 0 },
		{ "max_file_size", WEBSERV_route_parse_upload_max_file_size, /* required = */ 0, 0 },
	};

	b32	ok_old = ctx.ok;
	ctx.ok = 1;

	route.Upload = WEBSERV_route_upload_make();
	const TOML_Table&	table = *value.Table;
	for_table_begin(table, const TOML_Table, kv) {
		const string_view&	key = kv.key;
		const TOML_Value&	value = kv.value;

		i32	idx = -1;
		for (i32 i = 0; i < cast(i32)count_of(entries); ++i) {
			if (key == entries[i].key) {
				idx = i;
				break ;
			}
		}
		if (idx == -1) {
			context_error(KEY_UNKNOWN, value.pos, key);
			continue ;
		}

		entries[idx].proc(ctx, route.Upload, value);
		entries[idx].is_seen = 1;
	} for_table_end ;

	for (i32 i = 0; i < cast(i32)count_of(entries); ++i) {
		if (entries[i].is_seen || !entries[i].is_required) {
			continue ;
		}

		context_error(KEY_REQUIRED, value.pos, entries[i].key);
	}

	b32	ok_new = ctx.ok;
	ctx.ok = ok_old && ok_new;

	return (ctx.ok);
}
static b32				WEBSERV_route_parse_cgi(Parser_Context& ctx, Route& route, const TOML_Value& value) {
	typedef void	(*Entry_Proc)(Parser_Context&, WEBSERV_Route_CGI&, const TOML_Value&);
	typedef struct {
		string_view	key;
		Entry_Proc	proc;

		b32			is_required;
		b32			is_seen;
	}	Entry;

	Entry	entries[] = {
		{ "interpreters", WEBSERV_route_parse_cgi_interpreters, /* required = */ 1, 0 },
		{ "env", WEBSERV_route_parse_cgi_env, /* required = */ 0, 0 },
	};

	b32	ok_old = ctx.ok;
	ctx.ok = 1;

	route.CGI = WEBSERV_route_cgi_make();
	const TOML_Table&	table = *value.Table;
	for_table_begin(table, const TOML_Table, kv) {
		const string_view&	key = kv.key;
		const TOML_Value&	value = kv.value;

		i32	idx = -1;
		for (i32 i = 0; i < cast(i32)count_of(entries); ++i) {
			if (key == entries[i].key) {
				idx = i;
				break ;
			}
		}
		if (idx == -1) {
			context_error(KEY_UNKNOWN, value.pos, key);
			continue ;
		}

		entries[idx].proc(ctx, route.CGI, value);
		entries[idx].is_seen = 1;
	} for_table_end ;

	for (i32 i = 0; i < cast(i32)count_of(entries); ++i) {
		if (entries[i].is_seen || !entries[i].is_required) {
			continue ;
		}

		context_error(KEY_REQUIRED, value.pos, entries[i].key);
	}

	b32	ok_new = ctx.ok;
	ctx.ok = ok_old && ok_new;

	return (ctx.ok);
}

static b32				WEBSERV_config_parse_route_props(Parser_Context& ctx, Route& route, const TOML_Value& value) {
	typedef b32			(*Entry_Proc)(Parser_Context&, Route&, const TOML_Value&);
	const Entry_Proc	procs[WEBSERV_ROUTE_COUNT] = {
		0, /* INVALID */

		WEBSERV_route_parse_basic,    /* BASIC   : server */
		WEBSERV_route_parse_redirect, /* REDIRECT: redirect */
		WEBSERV_route_parse_upload,   /* UPLOAD  : upload */
		WEBSERV_route_parse_cgi,      /* CGI     : cgi */
	};

	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_TABLE)) {
		return (0);
	}
	if (value.Table == 0) {
		context_error(VALUE_INVALID, value.pos, "properties table");
		return (0);
	}

	const Entry_Proc	proc = procs[route.kind];
	if (proc == 0) {
		ctx.ok = 0;
		return (0);
	}
	return (proc(ctx, route, value));
}

static b32				WEBSERV_config_parse_route(Parser_Context& ctx, Route& route, const TOML_Value& value) {
	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_TABLE)) {
		return (0);
	}
	if (value.Table == 0) {
		context_error(VALUE_INVALID, value.pos, "route");
		return (0);
	}

	b32	ok_old = ctx.ok;
	ctx.ok = 1;

	typedef b32	(*Entry_Proc)(Parser_Context&, Route&, const TOML_Value&);
	typedef struct {
		string_view	key;
		Entry_Proc	proc;

		b32			is_required;
		b32			is_seen;
	} Entry;

	Entry	entries[] = {
		{ "path", WEBSERV_config_parse_route_path, /* required = */ 1, 0 }, /* Path of the route */
		{ "cascade_disable", WEBSERV_config_parse_route_cascade, /* required = */ 0, 0 }, /* Whether the path is cascaded to descendants */

		{ "type", WEBSERV_config_parse_route_kind, /* required = */ 1, 0 }, /* Variant of the route */
		{ "props", WEBSERV_config_parse_route_props, /* required = */ 1, 0 }, /* Props of the route variant */
	};

	i32	idx_props = -1;
	for_table_begin(*value.Table, const hash_table<TOML_Value>, kv) {
		const string_view&	key = kv.key;
		const TOML_Value&	value = kv.value;

		i32	idx = -1;
		for (i32 i = 0; i < cast(i32)count_of(entries); ++i) {
			if (key == entries[i].key) {
				idx = i;
				break ;
			}
		}

		if (idx == -1) {
			context_error(KEY_UNKNOWN, value.pos, key);
			continue ;
		}

		if (key == "props") { /* NOTE(xenobas): defer "props" parsing for later */
			idx_props = idx;
			continue ;
		}

		entries[idx].proc(ctx, route, value);
		entries[idx].is_seen = 1;
	} for_table_end ;

	if (idx_props >= 0 && route.kind != WEBSERV_ROUTE_INVALID) { /* NOTE(xenobas): Must always be run the last */
		const TOML_Value&	props = value.Table->get("props");

		entries[idx_props].proc(ctx, route, props);
		entries[idx_props].is_seen = 1;
	}

	for (i32 i = 0; i < cast(i32)count_of(entries); ++i) {
		if (entries[i].is_seen || !entries[i].is_required) {
			continue ;
		}

		context_error(KEY_REQUIRED, value.pos, entries[i].key);
	}

	b32	ok_new = ctx.ok;
	ctx.ok = ok_old && ctx.ok;

	return (ok_new);
}
static void				WEBSERV_config_parse_routes(Parser_Context& ctx, const TOML_Value& value) {
	Instance&	instance = ctx.instance;

	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_ARRAY)) {
		return ;
	}
	if (value.Array == 0) {
		context_error(VALUE_INVALID, value.pos, "routes array");
		return ;
	}

	const TOML_Array&	routes_array = *value.Array;
	for (i32 i = 0; i < routes_array.len; ++i) {
		const TOML_Value&	value = routes_array[i];

		Route	route = WEBSERV_route_make();

		b32		route_ok = WEBSERV_config_parse_route(ctx, route, value);
		if (!route_ok) {
			WEBSERV_route_delete(route);
			continue ;
		}

		/* TODO(xenobas): move this to inside of respective parser to get better position for error reporting */
		b32		path_ok = !instance.routes.has(route.path);
		if (!path_ok) {
			/* NOTE(xenobas): Due to the deletion below, we have to clone it */
			string_view	path = string_view::alloc(route.path);
			context_error(ROUTE_PATH_DUP, value.pos, path);

			WEBSERV_route_delete(route);
			continue ;
		}

		const string_view& key = route.path;
		instance.routes.set(key, route);
	}
}
static void				WEBSERV_config_check_routes(Parser_Context& ctx) {
	Instance&	instance = ctx.instance;

	typedef hash_table<WEBSERV_Route>	Routes;
	for_table_begin(instance.routes, const Routes, kv) {
		const string_view&	path = kv.key;
		const WEBSERV_Route	route = kv.value;

		unused(path);
		unused(route);
	} for_table_end ;
}
static void				WEBSERV_config_parse_stage_key(Parser_Context& ctx, const string_view& key, const TOML_Value& value) {
	if (key == "port") {
		WEBSERV_config_parse_port(ctx, value);
	} else if (key == "host") {
		WEBSERV_config_parse_host(ctx, value);
	} else if (key == "error_4xx") {
		WEBSERV_config_parse_error_4xx(ctx, value);
	} else if (key == "error_5xx") {
		WEBSERV_config_parse_error_5xx(ctx, value);
	} else if (key == "request_body_max") {
		WEBSERV_config_parse_request_body_max(ctx, value);
	} else if (key == "routes") {
		WEBSERV_config_parse_routes(ctx, value);
		WEBSERV_config_check_routes(ctx);
	} else {
		context_error(KEY_UNKNOWN, value.pos, key);
	}
}

/* DESCRIPTION: check instance keys and exclusions */
static void				WEBSERV_config_parse_stage_check_instance(Config& config, const TOML_Value& instance_value) {
	if (instance_value.Table == 0 || instance_value.Table->count == 0) {
		config_error(INSTANCE_EMPTY, instance_value);
		return ;
	}

	const TOML_Table&	table = *instance_value.Table;
	
	Instance			instance = WEBSERV_instance_make();
	Parser_Context		ctx = { 0, instance, config, 1 };
	/* TODO(xenobas): Use the entry system to distinguish defaults and necessities */
	for_table_begin(table, const TOML_Table, pair) {
		const string_view&	key = pair.key;
		const TOML_Value&	value = pair.value;

		WEBSERV_config_parse_stage_key(ctx, key, value);
	} for_table_end ;

	if (ctx.ok) {
		config.instances.push(instance);
	} else {
		WEBSERV_instance_delete(instance);
	}
}
/* DESCRIPTION: check all top level keys and verifies their types */
static void				WEBSERV_config_parse_stage_check(Config& config) {
	if (!config.ok) {
		return ;
	}

	const TOML_Value&	root = config.document.root;
	if (root.kind != TOML_VALUE_TABLE) {
		WEBSERV_config_error(config, WEBSERV_CONFIG_ERROR_ROOT_TYPE);
		return ;
	}
	if (root.Table == 0 || root.Table->count == 0) {
		config_error(ROOT_DATA, root);
		return ;
	}

	for_table_begin((*root.Table), TOML_Table, pair_root) {
		const string_view&	key = pair_root.key;
		const TOML_Value&	instances = pair_root.value;

		if (key == "instance" && instances.kind != TOML_VALUE_ARRAY_TABLES) {
			config_error(TYPE_MISMATCH, instances, "array of tables");
			return ;
		}
		if (key != "instance") {
			config_error(KEY_DISALLOWED, instances, "global scope can only contain \"[[instance]]\" tables");
			return ;
		}

		const TOML_Array&	array = *instances.Array;
		for (i32 instance_index = 0; instance_index < array.len; ++instance_index) {
			const TOML_Value&	instance = array[instance_index];
			WEBSERV_config_parse_stage_check_instance(config, instance);
		}
	} for_table_end ;
}

Config					WEBSERV_config_parse(const TOML_Document& document) {
	Config	config = WEBSERV_config_make(document);
	
	WEBSERV_config_parse_stage_check(config);
	return (config);
}
