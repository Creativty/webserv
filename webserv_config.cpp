/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv_config.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xenobas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 13:42:31 by xenobas           #+#    #+#             */
/*   Updated: 2025/10/30 16:28:47 by xenobas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

static WEBSERV_Config	WEBSERV_config_make(const TOML_Document& toml) {
	WEBSERV_Config	config;

	config.instances = dynamic_array<WEBSERV_Instance>();

	config.toml = toml;
	config.errors = dynamic_array<WEBSERV_Config_Error>();
	return (config);
}
static void				WEBSERV_route_delete(WEBSERV_Route& route) {
	unused(route);
}
static void				WEBSERV_instance_delete(WEBSERV_Instance& instance) {
	instance.error_4xx.free();
	instance.error_5xx.free();

	for (i32 i = 0; i < instance.routes.cap; ++i) {
		hash_table<WEBSERV_Route>::hash_table_item&	item = instance.routes.items[i];
		if (item.used()) {
			WEBSERV_route_delete(item.value);
		}
	}
	instance.routes.destroy();
}

void					WEBSERV_config_delete(WEBSERV_Config& config) {
	for (i32 i = 0; i < config.instances.len; ++i) {
		WEBSERV_instance_delete(config.instances[i]);
	}
	config.instances.free();

}
WEBSERV_Config			WEBSERV_config_parse(const TOML_Document& toml) {
	WEBSERV_Config	config = WEBSERV_config_make(toml);
	
	return (config);
}
