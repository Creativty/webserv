/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/26 15:46:57 by aindjare          #+#    #+#             */
/*   Updated: 2025/11/07 10:19:20 by xenobas          ###   ########.fr       */
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
// TOML_VALUE_NIL,
// 
// TOML_VALUE_BOOLEAN,
// TOML_VALUE_NUMBER,
// TOML_VALUE_STRING,
// 
// TOML_VALUE_ARRAY,
// TOML_VALUE_ARRAY_TABLES,
// TOML_VALUE_TABLE,
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

	TOML_ERROR_LOAD_BYTES,

	/* Tokenizer */
	TOML_ERROR_TOKEN_INVALID,
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
	dynamic_array<byte>			bytes;

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
	WEBSERV_METHOD_INVALID,

	WEBSERV_METHOD_GET,
	WEBSERV_METHOD_POST,
	WEBSERV_METHOD_PUT,

	WEBSERV_METHOD_COUNT,
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
};
struct WEBSERV_Route_Basic {
	b32	directory_list;
};
struct WEBSERV_Route_Redirect {
	string_view	location;
};
struct WEBSERV_Route_Upload {
};
struct WEBSERV_Route_CGI {
	hash_table<string_view>		env;
};
struct WEBSERV_Route {
	string_view			pattern;
	WEBSERV_Method		methods_whitelist;

	WEBSERV_Route_Kind	kind;

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
	dynamic_array<WEBSERV_Interface>	interfaces;

	u16									port;
	WEBSERV_Address						addr;

	u32									request_body_max;

	string_view							error_4xx;
	string_view							error_5xx;

	hash_table<WEBSERV_Route>			routes;
};

#define CONFIG_ERROR_KINDS \
	CONFIG_ERROR_KIND(INVALID, "invalid") \
	CONFIG_ERROR_KIND(ROOT_TYPE, "root type") \
	CONFIG_ERROR_KIND(ROOT_DATA, "root data") \
	CONFIG_ERROR_KIND(KEY_UNKNOWN, "key unknown") \
	CONFIG_ERROR_KIND(KEY_DISALLOWED, "key disallowed") \
	CONFIG_ERROR_KIND(TYPE_MISMATCH, "type mismatch") \
	CONFIG_ERROR_KIND(PORT_RANGE, "port range") \
	CONFIG_ERROR_KIND(STRING_EMPTY, "string empty") \
	CONFIG_ERROR_KIND(VALUE_INVALID, "value invalid") \
	CONFIG_ERROR_KIND(INSTANCE_EMPTY, "instance empty")

enum WEBSERV_Config_Error_Kind {
#define CONFIG_ERROR_KIND(NAME, ...) WEBSERV_CONFIG_ERROR_##NAME,
	CONFIG_ERROR_KINDS
#undef CONFIG_ERROR_KIND
// WEBSERV_CONFIG_ERROR_INVALID,
// 
// WEBSERV_CONFIG_ERROR_ROOT_TYPE,
// WEBSERV_CONFIG_ERROR_ROOT_DATA,
// 
// WEBSERV_CONFIG_ERROR_KEY_UNKNOWN,
// WEBSERV_CONFIG_ERROR_KEY_DISALLOWED,
// WEBSERV_CONFIG_ERROR_TYPE_MISMATCH,
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

b32					OS_read_file(const string_view path, dynamic_array<byte>& out_data);
b32					OS_stat_file(const string_view& path, struct stat* buf = 0);
b32					OS_access_file(const string_view& _path, i32 flags = F_OK);
        			
#define				CLI_debug(...) CLI_debug_internal(__FILE__, __LINE__, __VA_ARGS__)
void				CLI_debug_internal(const char* file, i32 line, const char *fmt, ...);
void				CLI_show_help(FILE* stream);
void				CLI_show_extra(const char* prefix, const char* fmt, ...);
void				CLI_show_error_file_ext(string_view file_path);
void				CLI_show_error_file_access(string_view file_path);
void				CLI_show_error_file_mode(string_view file_path, mode_t mode);
void				CLI_show_error_syntax(const Position pos, const char *fmt, ...);
void				CLI_show_error_config(const Position pos, const char *fmt, ...);
void				CLI_show_errors_toml(const TOML_Document& document);
void				CLI_show_errors_config(const WEBSERV_Config& config);

TOML_Document		TOML_make(const string_view& file);
void				TOML_delete(TOML_Document& document);
TOML_Document		TOML_parse_file(const string_view& file);

WEBSERV_URI			WEBSERV_uri_make(const string_view& str);
void				WEBSERV_uri_delete(WEBSERV_URI& uri);
WEBSERV_URI			WEBSERV_uri_decode(const string_view& str);
string_view			WEBSERV_uri_encode(const WEBSERV_URI& uri, b32 write_trailing_slash = 0);

WEBSERV_Method		WEBSERV_method_make(const string_view& str);
b32					WEBSERV_http_version_supported(const string_view& str);

void				WEBSERV_config_delete(WEBSERV_Config& config);
WEBSERV_Config		WEBSERV_config_parse(const TOML_Document& toml);

HTTP_Request		HTTP_request_make(void);
void				HTTP_request_delete(HTTP_Request& req);
b32					HTTP_request_is_error(const HTTP_Request& req);
b32					HTTP_request_is_closed(const HTTP_Request& req);
void				HTTP_request_close(HTTP_Request& req);
b32					HTTP_request_read(HTTP_Request& req, const byte* data, i32 size);

void				HTTP_request_debug(HTTP_Request& req);
#endif
