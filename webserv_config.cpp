/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv_config.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xenobas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 13:42:31 by xenobas           #+#    #+#             */
/*   Updated: 2025/11/07 10:52:05 by xenobas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

typedef WEBSERV_Route				Route;
typedef WEBSERV_Config				Config;
typedef WEBSERV_Instance			Instance;

typedef WEBSERV_Config_Error		Error;
typedef WEBSERV_Config_Error_Kind	Error_Kind;

#define config_error(VARIANT, ...) WEBSERV_config_error(config, WEBSERV_CONFIG_ERROR_##VARIANT, __VA_ARGS__)

static const char*	toml_value_kind_strings[] = {
#define TOML_VALUE_KIND(NAME, STRING, ...) STRING,
	TOML_VALUE_KINDS
#undef TOML_VALUE_KIND
};

struct			Parser_Context {
	b32			did_parse_variant;
	Instance&	instance;
	Config&		config;

	b32			ok;
};

static void		WEBSERV_config_error(Config& config, Error_Kind kind) {
	Error	error;

	error.kind = kind;
	error.pos = (Position){ 0, 1, 1, config.document.file };

	error.str = string_view();
	error.value = 0;
	error.token = (TOML_Token){ TOML_TOKEN_INVALID, string_view(), error.pos };

	config.ok = 0;
	config.errors.push(error);
}
static void		WEBSERV_config_error(Config& config, Error_Kind kind, const Position& pos) {
	Error	error;

	error.kind = kind;
	error.pos = pos;

	error.str = string_view();
	error.value = 0;
	error.token = (TOML_Token){ TOML_TOKEN_INVALID, string_view(), error.pos };

	config.ok = 0;
	config.errors.push(error);
}
static void		WEBSERV_config_error(Config& config, Error_Kind kind, const Position& pos, const string_view& str) {
	Error	error;

	error.kind = kind;
	error.pos = pos;

	error.str = str;
	error.value = 0;
	error.token = (TOML_Token){ TOML_TOKEN_INVALID, string_view(), error.pos };

	config.ok = 0;
	config.errors.push(error);
}
static void		WEBSERV_config_error(Config& config, Error_Kind kind, const TOML_Value& value) {
	Error	error;

	error.kind = kind;
	error.pos = value.pos;

	error.str = string_view();
	error.value = (TOML_Value*)&value;
	error.token = (TOML_Token){ TOML_TOKEN_INVALID, string_view(), error.pos };

	config.ok = 0;
	config.errors.push(error);
}
static void		WEBSERV_config_error(Config& config, Error_Kind kind, const TOML_Value& value, const string_view& str) {
	Error	error;

	error.kind = kind;
	error.pos = value.pos;

	error.str = str;
	error.value = (TOML_Value*)&value;
	error.token = (TOML_Token){ TOML_TOKEN_INVALID, "", error.pos };

	config.ok = 0;
	config.errors.push(error);
}

static void		WEBSERV_route_delete(Route& route) {
	unused(route);
}

static Instance	WEBSERV_instance_make(void) {
	Instance	instance;

	instance.interfaces = dynamic_array<WEBSERV_Interface>();

	instance.port      = 0;
	instance.addr.blob = 0;

	instance.request_body_max = cast(u32)-1;

	instance.error_4xx = string_view();
	instance.error_5xx = string_view();

	instance.routes = hash_table<WEBSERV_Route>();
	return (instance);
}
static void		WEBSERV_instance_delete(Instance& instance) {
	instance.error_4xx.free();
	instance.error_5xx.free();

	instance.interfaces.free();

	for_table_begin(instance.routes, hash_table<Route>, item) {
		WEBSERV_route_delete(item.value);
	} for_table_end ;
	instance.routes.free();
}

static Config	WEBSERV_config_make(const TOML_Document& document) {
	Config	config;

	config.document = document;

	config.ok = 1;
	config.errors = dynamic_array<Error>();
	config.instances = dynamic_array<WEBSERV_Instance>();
	return (config);
}
void			WEBSERV_config_delete(Config& config) {
	for (i32 i = 0; i < config.instances.len; ++i) {
		WEBSERV_instance_delete(config.instances[i]);
	}
	config.instances.free();
	config.errors.free();
	config.ok = 1;

	TOML_delete(config.document);
}

static b32		WEBSERV_config_parse_expect(Parser_Context& ctx, const TOML_Value& value, TOML_Value_Kind expected, string_view msg = "") {
	Config&	config = ctx.config;

	if (value.kind == expected) {
		return (1);
	}

	ctx.ok = 0;
	if (!(bool)msg) {
		msg = toml_value_kind_strings[expected];
	}

	config_error(TYPE_MISMATCH, value, msg);
	return (0);
}
static void		WEBSERV_config_parse_stage_key(Parser_Context& ctx, const string_view& key, const TOML_Value& value) {
	Config&		config = ctx.config;
	Instance&	instance = ctx.instance;

	if (key == "port") {
		if (!WEBSERV_config_parse_expect(ctx, value, TOML_VALUE_NUMBER)) {
			return ;
		}
	
		i64	port_signed = value.Number;
		if (port_signed < 0 || port_signed > 65535) {
			ctx.ok = 0;
			config_error(PORT_RANGE, value.pos);
			return ;
		}

		instance.port = cast(u16)port_signed;
	} else if (key == "host") {
		if (!WEBSERV_config_parse_expect(ctx, value, TOML_VALUE_STRING)) {
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
			b32	byte_ok = 1;
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
	} else {
		config_error(KEY_UNKNOWN, value.pos, key);
	}
}

/* DESCRIPTION: check instance keys and exclusions */
static void		WEBSERV_config_parse_stage_check_instance(Config& config, const TOML_Value& instance_value) {
	if (instance_value.Table == 0 || instance_value.Table->count == 0) {
		config_error(INSTANCE_EMPTY, instance_value);
		return ;
	}

	const TOML_Table&	instance_table = *instance_value.Table;
	
	Instance			instance = WEBSERV_instance_make();
	Parser_Context		ctx = { 0, instance, config, 1 };
	for_table_begin(instance_table, const TOML_Table, pair) {
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
static void		WEBSERV_config_parse_stage_check(Config& config) {
	if (!config.ok) {
		return ;
	}

	const TOML_Value&	root = config.document.root;
	if (root.kind != TOML_VALUE_TABLE) {
		WEBSERV_config_error(config, WEBSERV_CONFIG_ERROR_ROOT_TYPE);
		return ;
	}
	if (root.Table == 0 || root.Table->count == 0) {
		WEBSERV_config_error(config, WEBSERV_CONFIG_ERROR_ROOT_DATA, root);
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

Config			WEBSERV_config_parse(const TOML_Document& document) {
	Config	config = WEBSERV_config_make(document);
	
	WEBSERV_config_parse_stage_check(config);
	return (config);
}
