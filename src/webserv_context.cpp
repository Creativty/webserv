/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv_context.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xenobas <rahimos.123@gmail.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/17 21:32:19 by xenobas           #+#    #+#             */
/*   Updated: 2025/12/18 00:03:54 by xenobas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "i64_table.hpp"

/* TODO(xenobas): Continue with response work */
struct HTTP_Response {
	i32						write_idx;

	HTTP_Status				status_code;
	hash_table<string_view>	headers;

	union {
		struct {
			byte*			bytes;
			i32				len;
		}	body;
		i32	fd;
	} content;
};

enum WEBSERV_Interest_Type {
	WEBSERV_INTEREST_SERVER,
	WEBSERV_INTEREST_CLIENT,
	WEBSERV_INTEREST_PROCESS,
};
struct WEBSERV_Interest {
	i32						fd;
	WEBSERV_Interest_Type	type;

	i64						client_ts; /* TODO(xenobas): Timestamps */
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

void			WEBSERV_interest_close(WEBSERV_Interest& interest) {
	::close(interest.fd);
	switch (interest.type) {
		case WEBSERV_INTEREST_SERVER: {
		} break ;
		case WEBSERV_INTEREST_CLIENT: {
			HTTP_request_delete(interest.client_req);
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
b32				WEBSERV_context_interest_make_client(WEBSERV_Context& context, i32 fd, struct sockaddr_in& sockaddr) {
	WEBSERV_Interest	interest; MEM_zero(interest);

	interest.fd = fd;
	interest.type = WEBSERV_INTEREST_CLIENT;

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
b32				WEBSERV_context_interest_make_server(WEBSERV_Context& context, i32 fd) {
	WEBSERV_Interest	interest; MEM_zero(interest);

	interest.fd = fd;
	interest.type = WEBSERV_INTEREST_SERVER;

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

void			WEBSERV_context_server_make(WEBSERV_Context& context, const WEBSERV_Instance& instance) {
	i32					fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (fd == -1) {
		CLI_show_error_runtime("Could not create server socket");
		CLI_show_extra("Reason", "%m");

		context.ok = 0;
		return ;
	}
	
	const i32			opts[] = { 1 };
	if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opts, size_of(opts)) == -1) {
		CLI_show_error_runtime("Could not set server socket to non-blocking mode");
		CLI_show_extra("Reason", "%m");

		::close(fd);
		context.ok = 0;

		return ;
	}

	struct sockaddr_in	sockaddr; MEM_zero(sockaddr);
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

	if (WEBSERV_context_interest_make_server(context, fd)) {
		CLI_debug("Listening on %u.%u.%u.%u:%u",
			(cast(u8*)(&sockaddr.sin_addr.s_addr))[0],
			(cast(u8*)(&sockaddr.sin_addr.s_addr))[1],
			(cast(u8*)(&sockaddr.sin_addr.s_addr))[2],
			(cast(u8*)(&sockaddr.sin_addr.s_addr))[3],
			::ntohs(sockaddr.sin_port));
	}

}
b32				WEBSERV_context_run(const WEBSERV_Config& config) {
	WEBSERV_Context	context = WEBSERV_context_make(config);
	if (!context.ok) {
		WEBSERV_context_delete(context);
		return (0);
	}

	for (i32 i = 0; i < context.config.instances.len; ++i) {
		const WEBSERV_Instance&	instance = context.config.instances[i];

		WEBSERV_context_server_make(context, instance);
	}
	if (!context.ok) {
		WEBSERV_context_delete(context);
		return (0);
	}

	// for (;;) {
	for (i32 frame = 0; frame < (1000 / 41) * 5; ++frame) {
		const u64				events_cap = 128;
		struct epoll_event		events_arr[events_cap];
		i32						events_len = ::epoll_wait(context.fd_events, events_arr, events_cap, 41);
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
					struct sockaddr_in	sockaddr; MEM_zero(sockaddr);
					socklen_t			socklen = size_of(sockaddr);

					i32	fd_server = interest.fd;
					i32	fd_client = ::accept(fd_server, cast(struct sockaddr*)&sockaddr, &socklen);
					if (fd_client == -1) {
						CLI_show_error_runtime("Could not accept incoming connection");
						CLI_show_extra("Reason", "%m");

						continue ;
					}

					if (WEBSERV_context_interest_make_client(context, fd_client, sockaddr)) {
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
						}

						CLI_debug("Received %d bytes of data from client", buff_len);
						continue ;
					}
					if (event_type_write) {
						const string_view	message =
							"HTTP/1.1 200 OK\r\n"
							"Content-Length: 0\r\n"
							"\r\n"
							"";
						i64			ret_write = ::write(fd_client, message.text, cast(u64)message.len);
						if (ret_write == -1) {
							CLI_show_error_runtime("Could not write response to client");
							CLI_show_extra("Reason", "%m");

							WEBSERV_context_interest_delete(context, interest);
							continue ;
						}

						WEBSERV_context_interest_delete(context, interest);
						continue ;
					}
				} break ;
				case WEBSERV_INTEREST_PROCESS: {
					CLI_todo("WEBSERV_INTEREST_PROCESS");
				} break ;
			}
		}
	}

	WEBSERV_context_delete(context);
	return (context.ok);
}
