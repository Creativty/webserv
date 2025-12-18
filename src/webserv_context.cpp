/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv_context.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xenobas <rahimos.123@gmail.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/17 21:32:19 by xenobas           #+#    #+#             */
/*   Updated: 2025/12/18 14:10:34 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

string_view		strconv_i64(i64 n) {
	string_builder	b;

	b.write(n);
	return (b.to_string());
}

enum HTTP_Status {
	HTTP_STATUS_PROCESSING			=  102,
    HTTP_STATUS_OK					=  200,
    HTTP_STATUS_CREATED				=  201,
	HTTP_STATUS_NO_CONTENT			=  204,
	HTTP_STATUS_MOVED_PERMANENTLY	=  301,
    HTTP_STATUS_BAD_REQUEST			=  400,
    HTTP_STATUS_UNAUTHORIZED		=  401,
    HTTP_STATUS_FORBIDDEN			=  403,
    HTTP_STATUS_NOT_FOUND			=  404,
	HTTP_STATUS_METHOD_NOT_ALLOWED	=  405,
	HTTP_STATUS_CONTENT_TOO_LARGE	=  413,
    HTTP_STATUS_SERVER_ERROR		=  500,
	HTTP_STATUS_NOT_IMPLEMENTED		=  501,
	HTTP_STATUS_BAD_GATEWAY			=  502,
};

string_view		HTTP_status_as_string(HTTP_Status status) {
	switch (status) {
		case HTTP_STATUS_PROCESSING: {
			return ("101");
		} break ;
		case HTTP_STATUS_METHOD_NOT_ALLOWED: {
			return ("405");
		} break ;
		case HTTP_STATUS_OK: {
			return ("200");
		} break ;
		case HTTP_STATUS_CREATED: {
			return ("201");
		} break ;
		case HTTP_STATUS_NO_CONTENT: {
			return ("204");
		} break ;
		case HTTP_STATUS_MOVED_PERMANENTLY: {
			return ("301");
		} break ;
		case HTTP_STATUS_BAD_REQUEST: {
			return ("400");
		} break ;
		case HTTP_STATUS_UNAUTHORIZED: {
			return ("401");
		} break ;
		case HTTP_STATUS_FORBIDDEN: {
			return ("403");
		} break ;
		case HTTP_STATUS_NOT_FOUND: {
			return ("404");
		} break ;
		case HTTP_STATUS_CONTENT_TOO_LARGE: {
			return ("413");
		} break ;
		case HTTP_STATUS_SERVER_ERROR: {
			return ("500");
		} break ;
		case HTTP_STATUS_NOT_IMPLEMENTED: {
			return ("501");
		} break ;
		case HTTP_STATUS_BAD_GATEWAY: {
			return ("502");
		} break ;
	}
}
string_view		HTTP_status_as_text(HTTP_Status status) {
	switch (status) {
		case HTTP_STATUS_PROCESSING: {
			return ("Processing");
		} break ;
		case HTTP_STATUS_METHOD_NOT_ALLOWED: {
			return ("Method Not Allowed");
		} break ;
		case HTTP_STATUS_OK: {
			return ("OK");
		} break ;
		case HTTP_STATUS_CREATED: {
			return ("Created");
		} break ;
		case HTTP_STATUS_NO_CONTENT: {
			return ("No Content");
		} break ;
		case HTTP_STATUS_MOVED_PERMANENTLY: {
			return ("Moved Permanently");
		} break ;
		case HTTP_STATUS_BAD_REQUEST: {
			return ("Bad Request");
		} break ;
		case HTTP_STATUS_UNAUTHORIZED: {
			return ("Unauthorized");
		} break ;
		case HTTP_STATUS_FORBIDDEN: {
			return ("Forbidden");
		} break ;
		case HTTP_STATUS_NOT_FOUND: {
			return ("Not Found");
		} break ;
		case HTTP_STATUS_CONTENT_TOO_LARGE: {
			return ("Content Too Large");
		} break ;
		case HTTP_STATUS_SERVER_ERROR: {
			return ("Internal Server Error");
		} break ;
		case HTTP_STATUS_NOT_IMPLEMENTED: {
			return ("Not Implemented");
		} break ;
		case HTTP_STATUS_BAD_GATEWAY: {
			return ("Bad Gateway");
		} break ;
	}
}

/* TODO(xenobas): Continue with response work */
struct HTTP_Response {
	i32				write_idx;
	string_view		write_str;

	HTTP_Status		status_code;
	HTTP_Headers	headers;

	b32				is_file;
	union {
		i32			content_fd;
		struct {
			byte*	bytes;
			i32		len;
		}			content_body;
	};
};

enum WEBSERV_Interest_Type {
	WEBSERV_INTEREST_SERVER,
	WEBSERV_INTEREST_CLIENT,
	WEBSERV_INTEREST_PROCESS,
};
struct WEBSERV_Interest {
	i32						fd;
	WEBSERV_Interest_Type	type;

	i32						server_idx; /* NOTE(xenobas): context.config.instances[server_idx] */

	i64						client_ts; /* TODO(xenobas): Timeout timestamp */
	HTTP_Request			client_req;
	HTTP_Response			client_res;
	struct sockaddr_in		client_sockaddr;
};
struct WEBSERV_Context {
	WEBSERV_Config			config;
	b32						ok;

	i32							fd_events;
	i64_table<WEBSERV_Interest>	interests;
};

void			HTTP_response_delete(HTTP_Response& response) {
	response.write_str.free();

	for_table_begin(response.headers, HTTP_Headers, header) {
		header.value.free();
	} for_table_end; 
	response.headers.free();

	if (response.is_file) {
		::close(response.content_fd);
	} else {
		string_view	content((char*)response.content_body.bytes, response.content_body.len);
		content.free();
	}
}
void			WEBSERV_interest_close(WEBSERV_Interest& interest) {
	::close(interest.fd);
	switch (interest.type) {
		case WEBSERV_INTEREST_SERVER: {
		} break ;
		case WEBSERV_INTEREST_CLIENT: {
			HTTP_request_delete(interest.client_req);
			HTTP_response_delete(interest.client_res);
		} break ;
		case WEBSERV_INTEREST_PROCESS: {
		} break ;
	}
}

b32				WEBSERV_context_interest_unregister(WEBSERV_Context& context, WEBSERV_Interest& interest) {
	i32	ret_ctl = ::epoll_ctl(context.fd_events, EPOLL_CTL_DEL, interest.fd, NULL);
	if (ret_ctl == -1) {
		CLI_show_error_runtime("Could not remove interest from epoll's internal data structure");
		CLI_show_extra("Reason", "%m");
		return (0);
	}
	return (1);
}
void			WEBSERV_context_interest_delete(WEBSERV_Context& context, WEBSERV_Interest& interest) {
	WEBSERV_context_interest_unregister(context, interest);

	WEBSERV_interest_close(interest);
	context.interests.unset(interest.fd);
}
b32				WEBSERV_context_interest_make_client(WEBSERV_Context& context, i32 srv, i32 fd, struct sockaddr_in& sockaddr) {
	WEBSERV_Interest	interest; MEM_zero(interest);

	interest.fd = fd;
	interest.type = WEBSERV_INTEREST_CLIENT;

	interest.server_idx = srv;

	interest.client_req = HTTP_request_make();
	interest.client_sockaddr = sockaddr;

	struct epoll_event	event_register; MEM_zero(event_register);
	event_register.events  = EPOLLIN | EPOLLHUP | EPOLLERR;
	event_register.data.fd = fd;

	i32					ret_ctl = ::epoll_ctl(context.fd_events, EPOLL_CTL_ADD, fd, &event_register);
	if (ret_ctl == -1) {
		WEBSERV_interest_close(interest);

		CLI_show_error_runtime("s internal data structure not add client to epoll's internal data structure");
		CLI_show_extra("Reason", "%m");

		::close(fd);
		return (0);
	}

	context.interests.set(fd, interest);

	return (1);
}
b32				WEBSERV_context_interest_make_server(WEBSERV_Context& context, i32 idx, i32 fd) {
	WEBSERV_Interest	interest; MEM_zero(interest);

	interest.fd = fd;
	interest.type = WEBSERV_INTEREST_SERVER;
	interest.server_idx = idx;

	struct epoll_event	event_register; MEM_zero(event_register);
	event_register.events  = EPOLLIN;
	event_register.data.fd = fd;

	i32					ret_ctl = ::epoll_ctl(context.fd_events, EPOLL_CTL_ADD, fd, &event_register);
	if (ret_ctl == -1) {
		WEBSERV_interest_close(interest);

		CLI_show_error_runtime("Could not add server to epoll's internal data structure");
		CLI_show_extra("Reason", "%m");

		::close(fd);
		return (0);
	}

	context.interests.set(fd, interest);

	return (1);
}

void			WEBSERV_context_server_make(WEBSERV_Context& context, i32 idx) {
	const WEBSERV_Instance&	instance = context.config.instances[idx];
	i32						fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (fd == -1) {
		CLI_show_error_runtime("Could not create server socket");
		CLI_show_extra("Reason", "%m");

		context.ok = 0;
		return ;
	}
	
	const i32				opts[] = { 1 };
	if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opts, size_of(opts)) == -1) {
		CLI_show_error_runtime("Could not set server socket to non-blocking mode");
		CLI_show_extra("Reason", "%m");

		::close(fd);
		context.ok = 0;

		return ;
	}

	struct sockaddr_in		sockaddr; MEM_zero(sockaddr);
	sockaddr.sin_port = ::htons(instance.port);
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = instance.addr.blob;

	i32					ret_bind = ::bind(fd, cast(struct sockaddr*)&sockaddr, size_of(sockaddr));
	if (ret_bind == -1) {
		CLI_show_error_runtime("Could not bind server socket");
		CLI_show_extra("Reason", "%m");

		::close(fd);
		context.ok = 0;

		return ;
	}

	i32					ret_listen = ::listen(fd, SOMAXCONN);
	if (ret_listen) {
		CLI_show_error_runtime("Could not start server listening on address %u.%u.%u.%u:%u",
			instance.addr.bytes[0],
			instance.addr.bytes[1],
			instance.addr.bytes[2],
			instance.addr.bytes[3],
			instance.port);
		CLI_show_extra("Reason", "%m");

		::close(fd);
		context.ok = 0;

		return ;
	}

	if (WEBSERV_context_interest_make_server(context, idx, fd)) {
		CLI_debug("Listening on %u.%u.%u.%u:%u",
			(cast(u8*)(&sockaddr.sin_addr.s_addr))[0],
			(cast(u8*)(&sockaddr.sin_addr.s_addr))[1],
			(cast(u8*)(&sockaddr.sin_addr.s_addr))[2],
			(cast(u8*)(&sockaddr.sin_addr.s_addr))[3],
			::ntohs(sockaddr.sin_port));
	}
}

void			WEBSERV_response_message_write_prelude(HTTP_Response& response, string_builder& content) {
	{
		string_view	status_code = HTTP_status_as_string(response.status_code);
		string_view	status_text = HTTP_status_as_text(response.status_code);

		content.write("HTTP/1.1 ");
		content.write(status_code);
		content.write(' ');
		content.write(status_text);
		content.write("\r\n");
	}
	{
		HTTP_Headers&	headers = response.headers;
		for_table_begin(headers, const hash_table<string_view>, header) {
			const string_view&	field_name = header.key;
			const string_view&	field_value = header.value;

			content.write(field_name);
			content.write(": ");
			content.write(field_value);
			content.write("\r\n");
		} for_table_end;

		content.write("\r\n");
	}
}
void			WEBSERV_response_message_write_content_status(HTTP_Response& response, string_builder& content, WEBSERV_Instance& instance) {
	HTTP_Status			status_code = response.status_code;
	string_view			status_code_string = HTTP_status_as_string(status_code);
	string_view			status_text = HTTP_status_as_text(status_code);

	if (instance.status.has(status_code_string)) {
		string_view		status_template = instance.status.get(status_code_string);

		content.write(status_template);
		return ;
	}
	{ /* NOTE(xenobas): Fallback <h1>{STATUS} {TEXT}</h1> */
		content.write("<h1>");
		content.write(cast(i64)status_code);
		content.write(' ');
		content.write(status_text);
		content.write("</h1>");
	}

}

void			WEBSERV_context_response_from_status(WEBSERV_Context& context, WEBSERV_Interest& interest, HTTP_Status status_code) {
	HTTP_Response		response;
	WEBSERV_Instance&	instance = context.config.instances[interest.server_idx];

	response.status_code = status_code;

	response.headers = HTTP_Headers();
	response.headers.case_insensitive = 1;

	string_builder		content_builder;
	WEBSERV_response_message_write_content_status(response, content_builder, instance);

	string_view			content = content_builder.to_string();
	response.is_file = 0;
	response.content_body.len = cast(i32)content.len;
	response.content_body.bytes = cast(byte*)content.text;

	response.headers.set("Content-Type", string_view::alloc("text/html"));
	response.headers.set("Content-Length", strconv_i64(response.content_body.len));

	string_builder		message;
	WEBSERV_response_message_write_prelude(response, message);
	message.write(content);

	response.write_idx = 0;
	response.write_str = message.to_string();

	interest.client_res = response;
}
void			WEBSERV_context_response_from_content(WEBSERV_Context& context, WEBSERV_Interest& interest, HTTP_Status status_code, string_view content) { /* Can be either chunked or regular stream */
	HTTP_Response		response;

	response.status_code = status_code;

	response.headers = HTTP_Headers();
	response.headers.case_insensitive = 1;

	response.is_file = 0;
	response.content_body.len = cast(i32)content.len;
	response.content_body.bytes = cast(byte*)content.text;

	response.headers.set("Content-Type", string_view::alloc("text/html"));
	response.headers.set("Content-Length", strconv_i64(response.content_body.len));

	string_builder		message;
	WEBSERV_response_message_write_prelude(response, message);
	message.write(content);

	response.write_idx = 0;
	response.write_str = message.to_string();

	unused(context);
	interest.client_res = response;
}
void			WEBSERV_context_response_from_file(WEBSERV_Context& context, WEBSERV_Interest& interest, HTTP_Status status_code, i32 fd) { /* Transfer-Encoding: chunked */
	unused(context);
	unused(interest);
	unused(status_code);
	unused(fd);
}

void			WEBSERV_context_response_make(WEBSERV_Context& context, WEBSERV_Interest& interest) {
	if (interest.type != WEBSERV_INTEREST_CLIENT) {
		return ;
	}

	HTTP_Request&		req = interest.client_req;
	WEBSERV_Instance&	instance = context.config.instances[interest.server_idx];
	/* TODO(xenobas): Can check for host later */

	string_view			_route_path = WEBSERV_http_route_pick(instance, req);
	if (!instance.routes.has(_route_path)) {
		WEBSERV_context_response_from_status(context, interest, HTTP_STATUS_NOT_FOUND);
		return ;
	}
	WEBSERV_Route&		route = instance.routes.get(_route_path);
	if ((req.method & route.methods_whitelist) == 0x0) {
		WEBSERV_context_response_from_status(context, interest, HTTP_STATUS_METHOD_NOT_ALLOWED);
		return ;
	}

	switch (route.kind) {
		case WEBSERV_ROUTE_SERVER: {
		} break ;
		case WEBSERV_ROUTE_UPLOAD: {
		} break ;
		case WEBSERV_ROUTE_REDIRECT: {
		} break ;
		case WEBSERV_ROUTE_CGI: {
		} break ;
		case WEBSERV_ROUTE_COUNT:
		case WEBSERV_ROUTE_INVALID:
		default: {
			CLI_show_error_runtime("Route kind of value %d is supposed to be unreachable", cast(i32)route.kind);
		} break ;
	}
	WEBSERV_context_response_from_status(context, interest, HTTP_STATUS_NOT_IMPLEMENTED);
}
b32				WEBSERV_context_response_write(WEBSERV_Context& context, WEBSERV_Interest& interest) {
	i32					fd = interest.fd;
	HTTP_Response&		res = interest.client_res;
	if (interest.type != WEBSERV_INTEREST_CLIENT) {
		return (0);
	}

	const i32	chunk_cap = 1024 * 8;
	i32			chunk_idx = res.write_idx;
	if (chunk_idx < res.write_str.len) {
		byte*		write_arr = cast(byte*)(&res.write_str[chunk_idx]);
		i32			write_cap = (res.write_str.len - res.write_idx >= chunk_cap) ? (chunk_cap) : (res.write_str.len - res.write_idx);
		i64			write_ret = ::write(fd, write_arr, cast(u64)write_cap);
		if (write_ret == -1) {
			CLI_show_error_runtime("Could not write response to client");
			CLI_show_extra("Reason", "%m");

		}
		else if (write_ret >= 0) {
			res.write_idx += write_ret;
		}
		return (0);
	}

	WEBSERV_context_interest_delete(context, interest);
	return (1);
}

WEBSERV_Context	WEBSERV_context_make(const WEBSERV_Config& config) {
	WEBSERV_Context	context;

	context.ok = 1;
	context.config = config;

	context.interests = i64_table<WEBSERV_Interest>();
	context.fd_events = ::epoll_create(SOMAXCONN);
	if (context.fd_events == -1) {
		CLI_show_error_runtime("Could not create epoll file descriptor");
		CLI_show_extra("Reason", "%m");

		context.ok = 0;
		return (context);
	}
	return (context);
}
void			WEBSERV_context_delete(WEBSERV_Context& context) {
	for (i32 i = 0; i < context.interests.cap; ++i) {
		if (!context.interests.items[i].used) {
			continue ;
		}

		WEBSERV_Interest&	interest = context.interests.items[i].value;
		WEBSERV_context_interest_delete(context, interest);
	}
	context.interests.free();

	::close(context.fd_events);
}
b32				WEBSERV_context_run(const WEBSERV_Config& config) {
	WEBSERV_Context	context = WEBSERV_context_make(config);
	if (!context.ok) {
		WEBSERV_context_delete(context);
		return (0);
	}

	for (i32 server_idx = 0; server_idx < context.config.instances.len; ++server_idx) {
		WEBSERV_context_server_make(context, server_idx);
	}
	if (!context.ok) {
		WEBSERV_context_delete(context);
		return (0);
	}

	::signal(SIGPIPE, SIG_IGN);
	// for (i32 frame = 0; frame < (1000 / 100) * 30; ++frame) {
	for (i32 req_count = 0; req_count < 3;) {
		const u64				events_cap = 128;
		struct epoll_event		events_arr[events_cap];
		i32						events_len = ::epoll_wait(context.fd_events, events_arr, events_cap, 100);
		if (events_len == -1) {
			if (errno == EINTR) {
				continue ;
			}

			CLI_show_error_runtime("Failed while polling for events");
			CLI_show_extra("Reason", "%m");

			context.ok = 0;
			break ;
		}

		for (i32 event_idx = 0; event_idx < events_len; ++event_idx) {
			struct epoll_event&	event = events_arr[event_idx];

			if (!context.interests.has(event.data.fd)) {
				CLI_show_error_runtime("Encountered unknown interest with fd %d", event.data.fd);
				continue ;
			}
			WEBSERV_Interest&	interest = context.interests.get(event.data.fd);
			switch (interest.type) {
				case WEBSERV_INTEREST_SERVER: {
					i32					server_idx = interest.server_idx;
					struct sockaddr_in	sockaddr; MEM_zero(sockaddr);
					socklen_t			socklen = size_of(sockaddr);

					i32	fd_server = interest.fd;
					i32	fd_client = ::accept(fd_server, cast(struct sockaddr*)&sockaddr, &socklen);
					if (fd_client == -1) {
						CLI_show_error_runtime("Could not accept incoming connection");
						CLI_show_extra("Reason", "%m");

						continue ;
					}

					if (WEBSERV_context_interest_make_client(context, server_idx, fd_client, sockaddr)) {
						CLI_debug("Accepted connection from %u.%u.%u.%u:%u",
							(cast(u8*)(&sockaddr.sin_addr.s_addr))[0],
							(cast(u8*)(&sockaddr.sin_addr.s_addr))[1],
							(cast(u8*)(&sockaddr.sin_addr.s_addr))[2],
							(cast(u8*)(&sockaddr.sin_addr.s_addr))[3],
							::ntohs(sockaddr.sin_port)
						);
					}
				} break ;
				case WEBSERV_INTEREST_CLIENT: {
					i32	fd_client = interest.fd;
					b32	event_type_stop = event.events & (EPOLLHUP | EPOLLERR);
					b32	event_type_read = event.events & EPOLLIN;
					b32	event_type_write = event.events & EPOLLOUT;

					if (event_type_stop) {
						if (event.events & EPOLLERR) {
							CLI_show_error_runtime("Client had an error occur at the socket level");

							i32	err_code = 0; u64	err_len = size_of(err_code);
							i32	ret_opt = ::getsockopt(fd_client, SOL_SOCKET, SO_ERROR, &err_code, cast(socklen_t*)&err_len);
							if (ret_opt == -1) {
								CLI_show_extra("Reason", "Could not get reason");
								continue ;
							}
							CLI_show_extra("Reason", "%s", ::strerror(err_code));
						}

						WEBSERV_context_interest_delete(context, interest);
						CLI_debug("Client at socket %d is terminated");
						continue ;
					}
					if (event_type_read) {
						const u64	buff_cap = 8192;
						byte		buff_arr[buff_cap];
						i64			buff_len = ::read(fd_client, buff_arr, buff_cap);
						if (buff_len <= 0) {
							if (buff_len == -1) {
								CLI_show_error_runtime("Failed during read data from client");
								CLI_show_extra("Reason", "%m");
							}

							WEBSERV_context_interest_delete(context, interest);
							continue ;
						}

						if (!HTTP_request_read(interest.client_req, buff_arr, cast(i32)buff_len)) {
							CLI_show_error_runtime("Parser has denied the request");

							WEBSERV_context_interest_delete(context, interest);
							continue ;
						}
						if (HTTP_request_is_done(interest.client_req)) {
							struct epoll_event	event_mod; MEM_zero(event_mod);
							event_mod.events = EPOLLOUT | EPOLLHUP | EPOLLERR;
							event_mod.data.fd = interest.fd;

							i32					ret_ctl = ::epoll_ctl(context.fd_events, EPOLL_CTL_MOD, fd_client, &event_mod);
							if (ret_ctl == -1) {
								CLI_show_error_runtime("Could not modify interest inside epoll's internal data structure");

								WEBSERV_context_interest_delete(context, interest);
								continue ;
							}

							WEBSERV_context_response_make(context, interest);
						}

						CLI_debug("Received %d bytes of data from client", buff_len);
						continue ;
					}
					if (event_type_write) {
						b32	req_terminated = WEBSERV_context_response_write(context, interest);
						if (req_terminated) {
							++req_count;
						}
						continue ;
					}
				} break ;
				case WEBSERV_INTEREST_PROCESS: {
					CLI_todo("WEBSERV_INTEREST_PROCESS");
				} break ;
			}
		}
		// TODO(xenobas): Implement Client Timeout
		// last_event_time - curr_time > TIMEOUT -> lbab al7bab
		// interest
		CLI_flush();
	}

	WEBSERV_context_delete(context);
	return (context.ok);
}
