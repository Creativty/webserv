/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 15:46:57 by aindjare          #+#    #+#             */
/*   Updated: 2025/12/15 16:48:16 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP
#include "base.hpp"
#include "string_view.hpp"
#include "dynamic_array.hpp"
#include "hash_table.hpp"
#include "terminal.hpp"

#include <cstdio>
#include <cstring>
#include <cstdarg>

#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

struct Position {
	i32			index;
	i32			row;
	i32			col;
	string_view	file;
};

enum TOML_Token_Kind {
	TOML_TOKEN_INVALID,

	TOML_TOKEN_EOF, // \0
	TOML_TOKEN_EOL, // \n

	TOML_TOKEN_EQUALS, // =
	TOML_TOKEN_COMMA, // ,

	TOML_TOKEN_OSQUARE, // [
	TOML_TOKEN_CSQUARE, // ]

	TOML_TOKEN_OCURLY, // {
	TOML_TOKEN_CCURLY, // }

	TOML_TOKEN_IDENT, // [a-zA-Z_]+[a-zA-Z0-9_]*

	TOML_TOKEN_TRUE, // true
	TOML_TOKEN_FALSE, // false

	TOML_TOKEN_NUMBER, // [0-9]+
	TOML_TOKEN_STRING, // ".*"
};
struct TOML_Token {
	TOML_Token_Kind	kind;
	string_view		str;
	Position		pos;
};
struct TOML_Tokenizer {
	dynamic_array<TOML_Token>	tokens;

	Position					pos;
	string_view					source;
};

typedef struct TOML_Value			TOML_Value;

typedef b32							TOML_Boolean;
typedef i64							TOML_Number;
typedef string_view					TOML_String;
typedef dynamic_array<TOML_Value>	TOML_Array;
typedef hash_table<TOML_Value>		TOML_Table;

#define TOML_VALUE_KINDS \
	TOML_VALUE_KIND(NIL, "nil") \
	TOML_VALUE_KIND(BOOLEAN, "boolean") \
	TOML_VALUE_KIND(NUMBER, "number") \
	TOML_VALUE_KIND(STRING, "string") \
	TOML_VALUE_KIND(ARRAY, "array") \
	TOML_VALUE_KIND(ARRAY_TABLES, "array of tables") \
	TOML_VALUE_KIND(TABLE, "table")

enum TOML_Value_Kind {
#define TOML_VALUE_KIND(NAME, ...) TOML_VALUE_##NAME,
	TOML_VALUE_KINDS
#undef TOML_VALUE_KIND
};
struct TOML_Value {
	TOML_Value_Kind	kind;
	Position		pos;
	union {
		TOML_Boolean	Boolean;
		TOML_Number		Number;
		TOML_String*	String;

		TOML_Array*		Array;
		TOML_Table*		Table;
	};
};
struct TOML_Parser {
	i32				index;
	TOML_Token		token;
	TOML_Tokenizer*	lexer;
	TOML_Value*		scope;
};

enum TOML_Error_Kind {
	TOML_ERROR_INVALID,

	TOML_ERROR_LOAD_STRING,

	/* Tokenizer */
	TOML_ERROR_TOKEN_INVALID,
	TOML_ERROR_TOKEN_NUMBER_DIGITS,
	TOML_ERROR_TOKEN_NUMBER_UNDERSCORE,
	TOML_ERROR_TOKEN_UNTERMINATED,

	/* Parser */
	TOML_ERROR_PARSER_EXPECT,
	TOML_ERROR_PARSER_SCOPE_KEY_DUP,
	TOML_ERROR_PARSER_NUMBER_CHAR,
	TOML_ERROR_PARSER_NUMBER_RANGE,
	TOML_ERROR_PARSER_STRING_QUOTES,
	TOML_ERROR_PARSER_UNSUPPORTED,
};
struct TOML_Error {
	TOML_Error_Kind	kind;
	Position		pos;

	string_view		str;
	TOML_Token		token;
	TOML_Value*		value;
};
struct TOML_Document {
	TOML_Value					root;
	dynamic_array<TOML_Error>	errors;

	TOML_Tokenizer				lexer;
	TOML_Parser					parser;

	string_view					file;
	string_view					string;

	b32							ok;
};

struct WEBSERV_URI {
	/* Commented outs are unused, since WEBSERV does not have a proxy mode where the members are needed */
	dynamic_array<string_view>	path;
	hash_table<string_view>		query;

	/* string_view				domain; */
	/* u16						port; */
	/* string_view				protocol; */

	string_view					str;
	b32							ok;
};
enum WEBSERV_Method {
	WEBSERV_METHOD_INVALID	= 0,

	WEBSERV_METHOD_GET		= 1 << 0,
	WEBSERV_METHOD_HEAD		= 1 << 1,
	WEBSERV_METHOD_POST		= 1 << 2,
	WEBSERV_METHOD_PUT		= 1 << 3,
	WEBSERV_METHOD_DELETE	= 1 << 4,
	WEBSERV_METHOD_CONNECT	= 1 << 5,
	WEBSERV_METHOD_OPTIONS	= 1 << 6,
	WEBSERV_METHOD_TRACE	= 1 << 7,
	WEBSERV_METHOD_PATCH	= 1 << 8,
};
struct WEBSERV_Interface {
	u16	port;
	u32	address;
};
union WEBSERV_Address {
	u32	blob;
	u8	bytes[4];
};

enum WEBSERV_Route_Kind {
	WEBSERV_ROUTE_INVALID,

	WEBSERV_ROUTE_BASIC,
	WEBSERV_ROUTE_REDIRECT,
	WEBSERV_ROUTE_UPLOAD,
	WEBSERV_ROUTE_CGI,

	WEBSERV_ROUTE_COUNT,
};
struct WEBSERV_Route_Basic {
	string_view	directory;
	b32			directory_list;
};
struct WEBSERV_Route_Redirect {
	string_view	location;
};
struct WEBSERV_Route_Upload {
	string_view	directory;
	i32			max_file_size;
};
struct WEBSERV_Route_CGI {
	hash_table<string_view>		interpreters;
	hash_table<string_view>		env;
};
struct WEBSERV_Route {
	string_view				path;

	b32						cascade;
	WEBSERV_Route_Kind		kind;
	WEBSERV_Method			methods_whitelist;

	/* NOTE(xenobas):
	 * C++ doesn't allow non trivial constructor in union types
	 * So we do this abomination instead
	 */
	WEBSERV_Route_Basic		Basic;
	WEBSERV_Route_Redirect	Redirect;
	WEBSERV_Route_Upload	Upload;
	WEBSERV_Route_CGI		CGI;
};

struct WEBSERV_Instance {
	u16									port;
	WEBSERV_Address						addr;
	string_view							host;

	u32									request_body_max;

	string_view							error_4xx;
	string_view							error_5xx;

	hash_table<WEBSERV_Route>			routes;

	i32									fd;
};

#define CONFIG_ERROR_KINDS \
	CONFIG_ERROR_KIND(INVALID, "invalid") \
	CONFIG_ERROR_KIND(ROOT_TYPE, "root type") \
	CONFIG_ERROR_KIND(ROOT_DATA, "root data") \
	CONFIG_ERROR_KIND(KEY_UNKNOWN, "key unknown") \
	CONFIG_ERROR_KIND(KEY_DISALLOWED, "key disallowed") \
	CONFIG_ERROR_KIND(KEY_REQUIRED, "key required") \
	CONFIG_ERROR_KIND(TYPE_MISMATCH, "type mismatch") \
	CONFIG_ERROR_KIND(PORT_RANGE, "port range") \
	CONFIG_ERROR_KIND(REQUEST_BODY_MAX_RANGE, "request_body_max range") \
	CONFIG_ERROR_KIND(STRING_EMPTY, "string empty") \
	CONFIG_ERROR_KIND(VALUE_INVALID, "value invalid") \
	CONFIG_ERROR_KIND(ROUTE_TYPE, "route type") \
	CONFIG_ERROR_KIND(ROUTE_PATH_DUP, "route path duplicate") \
	CONFIG_ERROR_KIND(ERROR_FILE, "error html file") \
	CONFIG_ERROR_KIND(DIR_RO, "directory unreadable") \
	CONFIG_ERROR_KIND(DIR_RW, "directory unreadable/unwritable") \
	CONFIG_ERROR_KIND(INSTANCE_EMPTY, "instance empty")

enum WEBSERV_Config_Error_Kind {
#define CONFIG_ERROR_KIND(NAME, ...) WEBSERV_CONFIG_ERROR_##NAME,
	CONFIG_ERROR_KINDS
#undef CONFIG_ERROR_KIND
};

struct WEBSERV_Config_Error {
	WEBSERV_Config_Error_Kind	kind;
	Position					pos;

	string_view					str;
	TOML_Token					token;
	TOML_Value*					value;
};
typedef struct WEBSERV_Document	WEBSERV_Document;
struct WEBSERV_Config {
	dynamic_array<WEBSERV_Instance>		instances;

	TOML_Document						document;
	dynamic_array<WEBSERV_Config_Error>	errors;

	b32									ok;
};

typedef hash_table<string_view>	HTTP_Headers;

enum HTTP_Request_Stage {
	HTTP_REQUEST_STAGE_METHOD,
	HTTP_REQUEST_STAGE_URI,
	HTTP_REQUEST_STAGE_VERSION,
	HTTP_REQUEST_STAGE_HEADERS,
	HTTP_REQUEST_STAGE_BODY,
	HTTP_REQUEST_STAGE_CHUNK,
	HTTP_REQUEST_STAGE_CHUNK_DATA,

	HTTP_REQUEST_STAGE_ERROR,
	HTTP_REQUEST_STAGE_DONE,
};
struct HTTP_Chunk {
	i32	index;
	i32	size;
};
struct HTTP_Request {
	WEBSERV_Method				method;
	WEBSERV_URI					uri;
	string_view					protocol;

	HTTP_Headers				headers;
	b32							chunked;
	i64							content_length;

	dynamic_array<HTTP_Chunk>	chunks;
	HTTP_Chunk					chunk;

	dynamic_array<byte>			buff;
	i32							buff_index;
	HTTP_Request_Stage			buff_stage;
};

extern string_view	CLI_exec_path;
extern b32			CLI_is_tty;

b32					OS_read_file(const string_view path, string_view& text);
b32					OS_stat_file(const string_view& path, struct stat* buf = 0);
b32					OS_access_file(const string_view& _path, i32 flags = F_OK);

b32					OS_test_file_read(const string_view& path, b32 strict_regular = 0);
b32					OS_test_dir_read(const string_view& path, b32 strict_regular = 0);
b32					OS_test_dir_read_write(const string_view& path, b32 strict_regular = 0);
        			
#define				CLI_debug(...) CLI_debug_internal(__FILE__, __LINE__, "DEBUG", __VA_ARGS__)
#define				CLI_todo(...) CLI_debug_internal(__FILE__, __LINE__, "TODO", __VA_ARGS__)
void				CLI_debug_internal(const char* file, i32 line, const char *label, const char *fmt, ...);
void				CLI_show_help(FILE* stream);
void				CLI_show_extra(const char* prefix, const char* fmt, ...);
void				CLI_show_error_file_ext(string_view file_path);
void				CLI_show_error_file_access(string_view file_path);
void				CLI_show_error_file_mode(string_view file_path, mode_t mode);
void				CLI_show_error_syntax(const Position pos, const char *fmt, ...);
void				CLI_show_error_config(const Position pos, const char *fmt, ...);
void				CLI_show_errors_toml(const TOML_Document& document);
void				CLI_show_errors_config(const WEBSERV_Config& config);
void				CLI_show_error_runtime(const char* fmt, ...);

TOML_Document		TOML_make(const string_view& file);
void				TOML_delete(TOML_Document& document);
TOML_Document		TOML_parse_file(const string_view& file);

WEBSERV_URI			WEBSERV_uri_make(const string_view& str);
void				WEBSERV_uri_delete(WEBSERV_URI& uri);
WEBSERV_URI			WEBSERV_uri_decode(const string_view& str);
string_view			WEBSERV_uri_encode(const WEBSERV_URI& uri, b32 write_trailing_slash = 0);

WEBSERV_Method		WEBSERV_method_make(const string_view& str);
const char*			WEBSERV_method_cstr(const WEBSERV_Method& method);
b32					WEBSERV_http_version_supported(const string_view& str);

void				WEBSERV_config_delete(WEBSERV_Config& config);
WEBSERV_Config		WEBSERV_config_parse(const TOML_Document& toml);

b32					WEBSERV_http_route_method_test(const WEBSERV_Route& route, WEBSERV_Method method);
string_view			WEBSERV_http_route_pick(const WEBSERV_Instance& instance, const string_view& path);
string_view			WEBSERV_http_route_pick(const WEBSERV_Instance& instance, const HTTP_Request& req);

HTTP_Request		HTTP_request_make(void);
void				HTTP_request_delete(HTTP_Request& req);
b32					HTTP_request_is_error(const HTTP_Request& req);
b32					HTTP_request_is_closed(const HTTP_Request& req);
void				HTTP_request_close(HTTP_Request& req);
b32					HTTP_request_read(HTTP_Request& req, const byte* data, i32 size);

void				HTTP_request_debug(HTTP_Request& req);

int					server(WEBSERV_Config& config);
#endif
