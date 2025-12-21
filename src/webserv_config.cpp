/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv_config.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xenobas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 13:42:31 by xenobas           #+#    #+#             */
/*   Updated: 2025/12/21 05:35:29 by xenobas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "string_builder.hpp"

typedef WEBSERV_Route				Route;
typedef WEBSERV_Route_Kind			Route_Kind;

typedef WEBSERV_Route_Server		Route_Server;
typedef WEBSERV_Route_Redirect		Route_Redirect;
typedef WEBSERV_Route_Upload		Route_Upload;
typedef WEBSERV_Route_CGI			Route_CGI;

typedef WEBSERV_Config				Config;
typedef WEBSERV_Instance			Instance;

typedef WEBSERV_Config_Error		Error;
typedef WEBSERV_Config_Error_Kind	Error_Kind;

struct					Parser_Context {
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
	route.methods_whitelist = WEBSERV_METHOD_GET;
	return (route);
}
static void				WEBSERV_route_delete(Route& route) {
	switch (route.kind) {
		case WEBSERV_ROUTE_SERVER: {
			Route_Server&	server = route.Server;

			server.fallback.free();
			server.directory.free();
		} break ;
		case WEBSERV_ROUTE_REDIRECT: {
			Route_Redirect&	redirect = route.Redirect;

			redirect.location.free();
			redirect.status.free();
		} break ;
		case WEBSERV_ROUTE_UPLOAD: {
			Route_Upload&	upload = route.Upload;

			upload.directory.free();
		} break ;
		case WEBSERV_ROUTE_CGI: {
			Route_CGI&	cgi = route.CGI;

			cgi.directory.free();

			/* CGI Binaries */
			for_table_begin(cgi.interpreters, hash_table<string_view>, kv) {
				kv.value.free();
			} for_table_end ;
			cgi.interpreters.free();

			/* CGI Environment */
			for_table_begin(cgi.env, hash_table<string_view>, kv) {
				kv.value.free();
			} for_table_end ;
			cgi.env.free();
		} break ;
		case WEBSERV_ROUTE_COUNT:
		case WEBSERV_ROUTE_INVALID:
		default: {
		} break ;
	}
	route.path.free();
	WEBSERV_uri_delete(route.uri);
}

static Route_Server		WEBSERV_route_server_make(void) {
	Route_Server	server;
	MEM_zero(server);

	server.fallback = string_view::alloc("index.html");
	server.directory = string_view();
	server.directory_list = 0;
	return (server);
}
static Route_Redirect	WEBSERV_route_redirect_make(void) {
	Route_Redirect	redirect;
	MEM_zero(redirect);

	redirect.location = string_view();
	redirect.status = string_view::alloc("302");
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

	cgi.directory = string_view();
	cgi.interpreters = hash_table<string_view>();
	cgi.env = hash_table<string_view>();
	return (cgi);
}

static Instance			WEBSERV_instance_make(void) {
	Instance	instance;

	instance.port		= 0;
	instance.addr.blob	= 0;

	instance.timeout = 5000;
	instance.request_body_limit = U32_MAX;

	instance.routes = hash_table<WEBSERV_Route>();
	instance.status = hash_table<string_view>();

	instance.fd			= -1;
	return (instance);
}
static void				WEBSERV_instance_delete(Instance& instance) {

	for_table_begin(instance.status, hash_table<string_view>, kv) {
		string_view&	html = kv.value;

		html.free();
	} for_table_end;
	instance.status.free();

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

static Route_Kind		WEBSERV_route_kind_match(const string_view& str) {
	if (str.eq_insensitive("server")) {
		return (WEBSERV_ROUTE_SERVER);
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
	if ((!str.has_prefix("/") && !str.has_prefix(".")) || str.has("?") || str.has("#") || str.has("&")) {
		context_error(VALUE_INVALID, value.pos, "uri path component or extension");
		return (0);
	}

	route.path = string_view::alloc(str);
	route.uri = WEBSERV_uri_decode(str);
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

/* SECTION: Route basic parsers */
static void				WEBSERV_route_parse_server_fallback(Parser_Context& ctx, WEBSERV_Route_Server& server, const TOML_Value& value) {
	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_STRING)) {
		return ;
	}
	if (value.String == 0 || !(bool)(*value.String)) {
		context_error(VALUE_INVALID, value.pos, "directory path");
		return ;
	}

	const string_view&	fallback = *value.String;

	server.fallback.free(); /* NOTE(xenobas): Free old value */
	server.fallback = string_view::alloc(fallback);
}
static void				WEBSERV_route_parse_server_directory(Parser_Context& ctx, WEBSERV_Route_Server& server, const TOML_Value& value) {
	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_STRING)) {
		return ;
	}
	if (value.String == 0 || !(bool)(*value.String)) {
		context_error(VALUE_INVALID, value.pos, "directory path");
		return ;
	}

	const string_view&	directory = *value.String;
	if (!OS_test_dir_read(directory)) {
		context_error(DIR_RO, value.pos, directory);
		return ;
	}
	server.directory = string_view::alloc(directory);
}
static void				WEBSERV_route_parse_server_directory_list(Parser_Context& ctx, WEBSERV_Route_Server& server, const TOML_Value& value) {
	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_BOOLEAN)) {
		return ;
	}

	server.directory_list = value.Boolean;
}
/* SECTION: Route redirect parsers */
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
static void				WEBSERV_route_parse_redirect_status(Parser_Context& ctx, WEBSERV_Route_Redirect& redirect, const TOML_Value& value) {
	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_NUMBER)) {
		return ;
	}

	i64	status_code = value.Number;
	if ((status_code < 300 || status_code > 304) && (status_code != 307 && status_code != 308)) {
		context_error(VALUE_INVALID, value.pos, "redirection status code");
		return ;
	}

	string_view	status;
	{
		string_builder	builder;
		
		builder.write(status_code);
		status = builder.to_string();
	}

	redirect.status.free();
	redirect.status = status;
}
/* SECTION: Route upload parsers */
static void				WEBSERV_route_parse_upload_directory(Parser_Context& ctx, WEBSERV_Route_Upload& upload, const TOML_Value& value) {
	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_STRING)) {
		return ;
	}
	if (value.String == 0 || !(bool)(*value.String)) {
		context_error(VALUE_INVALID, value.pos, "directory path");
		return ;
	}

	const string_view&	directory = *value.String;
	if (!OS_test_dir_read_write(directory)) {
		context_error(DIR_RW, value.pos, *value.String);
		return ;
	}
	upload.directory = string_view::alloc(directory);
}
static void				WEBSERV_route_parse_upload_max_file_size(Parser_Context& ctx, WEBSERV_Route_Upload& upload, const TOML_Value& value) {
	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_NUMBER)) {
		return ;
	}
	i64	max_file_size = value.Number;
	if (max_file_size < 0 || max_file_size > cast(i64)U32_MAX) {
		context_error(VALUE_INVALID, value.pos, "size in bytes");
		return ;
	}

	upload.max_file_size = cast(u32)max_file_size;
}
/* SECTION: Route cgi parsers */
static void				WEBSERV_route_parse_cgi_directory(Parser_Context& ctx, WEBSERV_Route_CGI& cgi, const TOML_Value& value) {
	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_STRING)) {
		return ;
	}
	if (value.String == 0 || !(bool)(*value.String)) {
		context_error(VALUE_INVALID, value.pos, "cgi binaries directory path");
		return ;
	}

	const string_view&	directory = *value.String;
	if (!OS_test_dir_read(directory)) {
		context_error(DIR_RW, value.pos, *value.String);
		return ;
	}
	cgi.directory = string_view::alloc(directory);
}
static void				WEBSERV_route_parse_cgi_interpreters(Parser_Context& ctx, Route_CGI& cgi, const TOML_Value& value) {
	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_TABLE)) {
		return ;
	}
	if (value.Table == 0) {
		context_error(VALUE_INVALID, value.pos, "interpreters table { ext = path, ... }");
		return ;
	}

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
		if (!OS_access_file(*value.String, /* flags = */ F_OK | X_OK)) {
			context_error(INTERPRETER_INVALID, value.pos, *value.String);
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
/* SECTION: Route variants parsers */
static b32				WEBSERV_route_parse_server(Parser_Context& ctx, Route& route, const TOML_Value& value) {
	typedef void	(*Entry_Proc)(Parser_Context&, WEBSERV_Route_Server&, const TOML_Value&);
	typedef struct {
		string_view	key;
		Entry_Proc	proc;

		b32			is_required;
		b32			is_visited;
	}	Entry;

	Entry	entries[] = {
		{ "directory", WEBSERV_route_parse_server_directory, /* required = */ 1, 0 },

		{ "fallback", WEBSERV_route_parse_server_fallback, /* required = */ 0, 0 },
		{ "directory_list", WEBSERV_route_parse_server_directory_list, /* required = */ 0, 0 },
	};

	b32	ok_old = ctx.ok;
	ctx.ok = 1;

	route.Server = WEBSERV_route_server_make();
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

		entries[idx].proc(ctx, route.Server, value);
		entries[idx].is_visited = 1;
	} for_table_end ;

	for (i32 i = 0; i < cast(i32)count_of(entries); ++i) {
		if (entries[i].is_visited || !entries[i].is_required) {
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
		b32			is_visited;
	}	Entry;

	Entry	entries[] = {
		{ "location",	WEBSERV_route_parse_redirect_location, /* required = */ 1, 0 },

		{ "status",		WEBSERV_route_parse_redirect_status, /* required = */ 0, 0 },
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
		entries[idx].is_visited = 1;
	} for_table_end ;

	for (i32 i = 0; i < cast(i32)count_of(entries); ++i) {
		if (entries[i].is_visited || !entries[i].is_required) {
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
		b32			is_visited;
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
		entries[idx].is_visited = 1;
	} for_table_end ;

	for (i32 i = 0; i < cast(i32)count_of(entries); ++i) {
		if (entries[i].is_visited || !entries[i].is_required) {
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
		b32			is_visited;
	}	Entry;

	Entry	entries[] = {
		{ "directory",		WEBSERV_route_parse_cgi_directory,		/* required = */ 1, 0 },
		{ "interpreters",	WEBSERV_route_parse_cgi_interpreters,	/* required = */ 1, 0 },

		{ "env",			WEBSERV_route_parse_cgi_env,			/* required = */ 0, 0 },
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
		entries[idx].is_visited = 1;
	} for_table_end ;

	for (i32 i = 0; i < cast(i32)count_of(entries); ++i) {
		if (entries[i].is_visited || !entries[i].is_required) {
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

		WEBSERV_route_parse_server,   /* SERVER   : server */
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
		b32			is_visited;
	} Entry;

	Entry	entries[] = {
		{ "path", WEBSERV_config_parse_route_path, /* required = */ 1, 0 }, /* Path of the route */

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
		entries[idx].is_visited = 1;
	} for_table_end ;

	if (idx_props >= 0 && route.kind != WEBSERV_ROUTE_INVALID) { /* NOTE(xenobas): Must always be run the last */
		const TOML_Value&	props = value.Table->get("props");

		entries[idx_props].proc(ctx, route, props);
		entries[idx_props].is_visited = 1;
	}

	for (i32 i = 0; i < cast(i32)count_of(entries); ++i) {
		if (entries[i].is_visited || !entries[i].is_required) {
			continue ;
		}

		context_error(KEY_REQUIRED, value.pos, entries[i].key);
	}

	switch (route.kind) {
		case WEBSERV_ROUTE_SERVER: {
			route.cascade = 1;
			route.methods_whitelist = WEBSERV_METHOD_GET;
		} break ;
		case WEBSERV_ROUTE_UPLOAD: {
			route.cascade = 1;
			route.methods_whitelist = cast(WEBSERV_Method)(WEBSERV_METHOD_POST | WEBSERV_METHOD_PUT | WEBSERV_METHOD_DELETE);
		} break ;
		case WEBSERV_ROUTE_REDIRECT: {
			route.cascade = 0;
			route.methods_whitelist = cast(WEBSERV_Method)(0xFF);
		} break ;
		case WEBSERV_ROUTE_CGI: {
			route.cascade = 1;
			route.methods_whitelist = cast(WEBSERV_Method)(0xFF);
		} break ;
		case WEBSERV_ROUTE_INVALID:
		case WEBSERV_ROUTE_COUNT:
		default: break ;
	}

	b32	ok_new = ctx.ok;
	ctx.ok = ok_old && ctx.ok;

	return (ok_new);
}
static void				WEBSERV_config_parse_instance_routes(Parser_Context& ctx, const TOML_Value& value) {
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

static void				WEBSERV_config_parse_instance_addr(Parser_Context& ctx, const TOML_Value& value) {
	Config&		config = ctx.config;
	Instance&	instance = ctx.instance;

	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_STRING)) {
		return ;
	}
	if (value.String == 0 || !(bool)(*(value.String))) {
		config_error(STRING_EMPTY, value.pos, "ipv4 address");
		return ;
	}

	string_view	addr_string = *value.String;
	if (addr_string.count('.') != 3) {
		config_error(VALUE_INVALID, value.pos, "ipv4 address");
		return ;
	}

	string_view		byte_string;
	i32				byte_index = 0;
	WEBSERV_Address	addr = { .blob = 0 };
	while (addr_string.split_iter(".", byte_string) && byte_index < 4) {
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
	if (!!addr_string || byte_index != 4) {
		config_error(VALUE_INVALID, value.pos, "ipv4 address");
		return ;
	}

	instance.addr = addr;
}
static void				WEBSERV_config_parse_instance_port(Parser_Context& ctx, const TOML_Value& value) {
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
static void				WEBSERV_config_parse_instance_request_body_limit(Parser_Context& ctx, const string_view& key, const TOML_Value& value) {
	Instance&	instance = ctx.instance;
	unused(key);

	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_NUMBER)) {
		return ;
	}
	i64			request_body_limit = value.Number;
	if (request_body_limit < 0 || request_body_limit > cast(i64)(U32_MAX)) {
		context_error(REQUEST_BODY_LIMIT_RANGE, value.pos);
		return ;
	}

	instance.request_body_limit = cast(u32)request_body_limit;
}
static void				WEBSERV_config_parse_instance_timeout(Parser_Context& ctx, const string_view& key, const TOML_Value& value) {
	Instance&	instance = ctx.instance;
	unused(key);

	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_NUMBER)) {
		return ;
	}
	i64			timeout = value.Number;
	if (timeout <= 0) {
		context_error(TIMEOUT_RANGE, value.pos);
		return ;
	}

	instance.timeout = timeout;
}
static void				WEBSERV_config_parse_instance_status(Parser_Context& ctx, const string_view& key, const TOML_Value& value) {
	Config&				config = ctx.config;
	Instance&			instance = ctx.instance;

	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_STRING)) {
		return ;
	}
	if (value.String == 0 || !(bool)(*(value.String))) {
		config_error(STRING_EMPTY, value.pos, "html file path");

		return ;
	}

	string_view			html_content;
	string_view			html_path = *value.String;
	if (!html_path.has_suffix(".html") || !OS_read_file(html_path, html_content)) {
		config_error(FILE_INVALID, value.pos, html_path);

		return ;
	}

	string_view			html_status = key.slice(key.len - 3);
	instance.status.set(html_status, html_content);
}
static void				WEBSERV_config_parse_instance_http(Parser_Context& ctx, const TOML_Value& value) {
	if (!CONTEXT_parse_expect(ctx, value, TOML_VALUE_TABLE)) {
		return ;
	}
	if (value.Table == 0) {
		context_error(VALUE_INVALID, value.pos, "http settings table");
		return ;
	}

	typedef void	(*Entry_Proc)(Parser_Context&, const string_view&, const TOML_Value&);
	typedef struct {
		string_view	key;
		Entry_Proc	proc;

		b32			is_required;
		b32			is_visited;
	} Entry;

	Entry	entries[] = {
		{ "request_body_limit", WEBSERV_config_parse_instance_request_body_limit, /* required = */ 0, 0 },	/* Instance host			(e.g "website.com") */
		{ "timeout", WEBSERV_config_parse_instance_timeout, /* required = */ 0, 0 },	/* Instance timeout			(e.g 5000ms) */

		{ "status_102", WEBSERV_config_parse_instance_status, /* required = */ 0, 0 },
		{ "status_200", WEBSERV_config_parse_instance_status, /* required = */ 0, 0 },
		{ "status_201", WEBSERV_config_parse_instance_status, /* required = */ 0, 0 },
		{ "status_202", WEBSERV_config_parse_instance_status, /* required = */ 0, 0 },
		{ "status_300", WEBSERV_config_parse_instance_status, /* required = */ 0, 0 },
		{ "status_301", WEBSERV_config_parse_instance_status, /* required = */ 0, 0 },
		{ "status_302", WEBSERV_config_parse_instance_status, /* required = */ 0, 0 },
		{ "status_303", WEBSERV_config_parse_instance_status, /* required = */ 0, 0 },
		{ "status_307", WEBSERV_config_parse_instance_status, /* required = */ 0, 0 },
		{ "status_308", WEBSERV_config_parse_instance_status, /* required = */ 0, 0 },
		{ "status_400", WEBSERV_config_parse_instance_status, /* required = */ 0, 0 },
		{ "status_401", WEBSERV_config_parse_instance_status, /* required = */ 0, 0 },
		{ "status_403", WEBSERV_config_parse_instance_status, /* required = */ 0, 0 },
		{ "status_404", WEBSERV_config_parse_instance_status, /* required = */ 0, 0 },
		{ "status_405", WEBSERV_config_parse_instance_status, /* required = */ 0, 0 },
		{ "status_413", WEBSERV_config_parse_instance_status, /* required = */ 0, 0 },
		{ "status_500", WEBSERV_config_parse_instance_status, /* required = */ 0, 0 },
		{ "status_501", WEBSERV_config_parse_instance_status, /* required = */ 0, 0 },
		{ "status_502", WEBSERV_config_parse_instance_status, /* required = */ 0, 0 },
		{ "status_504", WEBSERV_config_parse_instance_status, /* required = */ 0, 0 },
	};

	for_table_begin(*value.Table, const TOML_Table, kv) {
		const string_view&	key = kv.key;
		const TOML_Value&	value = kv.value;

		i32					entry_index = -1;
		for (i32 i = 0; i < cast(i32)count_of(entries); ++i) {
			if (key == entries[i].key) {
				entry_index = i;

				break ;
			}
		}
		if (entry_index == -1) {
			context_error(KEY_UNKNOWN, value.pos, key);

			continue ;
		}

		entries[entry_index].proc(ctx, key, value);
		entries[entry_index].is_visited = 1;
	} for_table_end ;
}
static void				WEBSERV_config_parse_instance(Config& config, const TOML_Value& value) {
	if (value.Table == 0 || value.Table->count == 0) {
		config_error(INSTANCE_EMPTY, value);
		return ;
	}

	typedef void	(*Entry_Proc)(Parser_Context&, const TOML_Value&);
	typedef struct {
		string_view	key;
		Entry_Proc	proc;

		b32			is_required;
		b32			is_visited;
	} Entry;

	Entry	entries[] = {
		{ "addr", WEBSERV_config_parse_instance_addr, /* required = */ 1, 0 },		/* Instance address			(e.g "0.0.0.0") */
		{ "port", WEBSERV_config_parse_instance_port, /* required = */ 1, 0 }, 		/* Instance port    		(e.g 8080)*/
		{ "routes", WEBSERV_config_parse_instance_routes, /* required = */ 1, 0 },	/* Instance routes			(e.g [{ path = "shit" }, {...}])*/

		{ "http", WEBSERV_config_parse_instance_http, /* required = */ 0, 0 },		/* Instance http settings  	(e.g { request_body_max? = 1024, status_400 = "static/bad_request.html" }) */
	};

	Instance			instance = WEBSERV_instance_make();
	Parser_Context		ctx = { instance , config, 1 };
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

		entries[idx].proc(ctx, value);
		entries[idx].is_visited = 1;
	} for_table_end ;

	for (i32 i = 0; i < cast(i32)count_of(entries); ++i) {
		if (entries[i].is_visited || !entries[i].is_required) {
			continue ;
		}

		context_error(KEY_REQUIRED, value.pos, entries[i].key);
	}

	if (!ctx.ok) {
		WEBSERV_instance_delete(ctx.instance);
		return ;
	}

	config.instances.push(ctx.instance);
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

			WEBSERV_config_parse_instance(config, instance);
		}
	} for_table_end ;
}

Config					WEBSERV_config_parse(const TOML_Document& document) {
	Config	config = WEBSERV_config_make(document);
	
	WEBSERV_config_parse_stage_check(config);
	return (config);
}
