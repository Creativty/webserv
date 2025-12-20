/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv_context.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xenobas <rahimos.123@gmail.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/17 21:32:19 by xenobas           #+#    #+#             */
/*   Updated: 2025/12/20 15:14:52 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

#ifndef CONTEXT_BUFFER_CAPACITY
#define CONTEXT_BUFFER_CAPACITY (8192)
#endif

#ifndef CONTEXT_STREAM_BASELINE
#define CONTEXT_STREAM_BASELINE (1024 * 1024 * 4)
#endif

template <typename T>
string_view		ENV_string(const string_view& name, T value, b32 is_header = 0) {
	string_builder	b;

	if (is_header) {
		b.write("HTTP");

		string_view	name_iter;
		string_view	name_temp = name;
		while (name_temp.split_iter("-", name_iter)) {
			b.write('_');
			b.write_uppercase(name_iter);
		}
	} else {
		b.write(name);
	}
	b.write('=');
	b.write(value);
	return (b.to_string());
}

enum HTTP_Status {
	HTTP_STATUS_PROCESSING			=  102,
    HTTP_STATUS_OK					=  200,
    HTTP_STATUS_CREATED				=  201,
	HTTP_STATUS_NO_CONTENT			=  204,
	HTTP_STATUS_MULTIPLE_CHOICES	=  300,
	HTTP_STATUS_MOVED_PERMANENTLY	=  301,
	HTTP_STATUS_FOUND				=  302,
	HTTP_STATUS_SEE_OTHER			=  303,
	HTTP_STATUS_TEMPORARY_REDIRECT	=  307,
	HTTP_STATUS_PERMANENT_REDIRECT	=  308,
    HTTP_STATUS_BAD_REQUEST			=  400,
    HTTP_STATUS_UNAUTHORIZED		=  401,
    HTTP_STATUS_FORBIDDEN			=  403,
    HTTP_STATUS_NOT_FOUND			=  404,
	HTTP_STATUS_METHOD_NOT_ALLOWED	=  405,
	HTTP_STATUS_CONTENT_TOO_LARGE	=  413,
    HTTP_STATUS_SERVER_ERROR		=  500,
	HTTP_STATUS_NOT_IMPLEMENTED		=  501,
	HTTP_STATUS_BAD_GATEWAY			=  502,
	HTTP_STATUS_GATEWAY_TIMEOUT		=  504,
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
		case HTTP_STATUS_MULTIPLE_CHOICES: {
			return ("300");
		} break ;
		case HTTP_STATUS_MOVED_PERMANENTLY: {
			return ("301");
		} break ;
		case HTTP_STATUS_FOUND: {
			return ("302");
		} break ;
		case HTTP_STATUS_SEE_OTHER: {
			return ("303");
		} break ;
		case HTTP_STATUS_TEMPORARY_REDIRECT: {
			return ("307");
		} break ;
		case HTTP_STATUS_PERMANENT_REDIRECT: {
			return ("308");
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
		case HTTP_STATUS_GATEWAY_TIMEOUT: {
			return ("504");
		} break ;
	}
	return ("500");
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
		case HTTP_STATUS_MULTIPLE_CHOICES: {
			return ("Multiple Choices");
		} break ;
		case HTTP_STATUS_MOVED_PERMANENTLY: {
			return ("Moved Permanently");
		} break ;
		case HTTP_STATUS_FOUND: {
			return ("Found");
		} break ;
		case HTTP_STATUS_SEE_OTHER: {
			return ("See Other");
		} break ;
		case HTTP_STATUS_TEMPORARY_REDIRECT: {
			return ("Temporary Redirect");
		} break ;
		case HTTP_STATUS_PERMANENT_REDIRECT: {
			return ("Permanent Redirect");
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
		case HTTP_STATUS_GATEWAY_TIMEOUT: {
			return ("Gateway Timeout");
		} break ;
	}
	return ("Internal Server Error");
}
HTTP_Status		HTTP_status_from_string(string_view	code) {
	if (code == "102") {
		return (HTTP_STATUS_PROCESSING);
	}
    if (code == "200") {
		return (HTTP_STATUS_OK);
	}
    if (code == "201") {
		return (HTTP_STATUS_CREATED);
	}
	if (code == "204") {
		return (HTTP_STATUS_NO_CONTENT);
	}
	if (code == "300") {
		return (HTTP_STATUS_MULTIPLE_CHOICES);
	}
	if (code == "301") {
		return (HTTP_STATUS_MOVED_PERMANENTLY);
	}
	if (code == "302") {
		return (HTTP_STATUS_FOUND);
	}
	if (code == "303") {
		return (HTTP_STATUS_SEE_OTHER);
	}
	if (code == "307") {
		return (HTTP_STATUS_TEMPORARY_REDIRECT);
	}
	if (code == "308") {
		return (HTTP_STATUS_PERMANENT_REDIRECT);
	}
    if (code == "400") {
		return (HTTP_STATUS_BAD_REQUEST);
	}
    if (code == "401") {
		return (HTTP_STATUS_UNAUTHORIZED);
	}
    if (code == "403") {
		return (HTTP_STATUS_FORBIDDEN);
	}
    if (code == "404") {
		return (HTTP_STATUS_NOT_FOUND);
	}
	if (code == "405") {
		return (HTTP_STATUS_METHOD_NOT_ALLOWED);
	}
	if (code == "413") {
		return (HTTP_STATUS_CONTENT_TOO_LARGE);
	}
    if (code == "500") {
		return (HTTP_STATUS_SERVER_ERROR);
	}
	if (code == "501") {
		return (HTTP_STATUS_NOT_IMPLEMENTED);
	}
	if (code == "502") {
		return (HTTP_STATUS_BAD_GATEWAY);
	}
	if (code == "504") {
		return (HTTP_STATUS_GATEWAY_TIMEOUT);
	}
	return (HTTP_STATUS_NOT_IMPLEMENTED);
}

string_view		HTTP_mime_from_name(const string_view& filename) {
    if (filename.has_suffix(".html")) return ("text/html");
    if (filename.has_suffix(".htm") ) return ("text/html");
    if (filename.has_suffix(".css") ) return ("text/css");
    if (filename.has_suffix(".js")  ) return ("application/javascript");
    if (filename.has_suffix(".json")) return ("application/json");
    if (filename.has_suffix(".xml") ) return ("application/xml");
    if (filename.has_suffix(".pdf") ) return ("application/pdf");

    // Images
    if (filename.has_suffix(".png") ) return ("image/png");
    if (filename.has_suffix(".jpg") ) return ("image/jpeg");
    if (filename.has_suffix(".jpeg")) return ("image/jpeg");
    if (filename.has_suffix(".gif") ) return ("image/gif");
    if (filename.has_suffix(".bmp") ) return ("image/bmp");
    if (filename.has_suffix(".ico") ) return ("image/x-icon");
    if (filename.has_suffix(".svg") ) return ("image/svg+xml");
    if (filename.has_suffix(".webp")) return ("image/webp");

    // Audio
    if (filename.has_suffix(".mp3") ) return ("audio/mpeg");
    if (filename.has_suffix(".wav") ) return ("audio/wav");
    if (filename.has_suffix(".ogg") ) return ("audio/ogg");

    // Video
    if (filename.has_suffix(".mp4") ) return ("video/mp4");
    if (filename.has_suffix(".webm")) return ("video/webm");
    if (filename.has_suffix(".avi") ) return ("video/x-msvideo");
    if (filename.has_suffix(".mov") ) return ("video/quicktime");
    if (filename.has_suffix(".mkv") ) return ("video/x-matroska");

    // Fonts
    if (filename.has_suffix(".ttf") ) return ("font/ttf");
    if (filename.has_suffix(".otf") ) return ("font/otf");
    if (filename.has_suffix(".woff")) return ("font/woff");
    if (filename.has_suffix(".woff2")) return ("font/woff2");

    // Archives
    if (filename.has_suffix(".zip") ) return ("application/zip");
    if (filename.has_suffix(".rar") ) return ("application/vnd.rar");
    if (filename.has_suffix(".7z")  ) return ("application/x-7z-compressed");
    if (filename.has_suffix(".tar") ) return ("application/x-tar");
    if (filename.has_suffix(".gz")  ) return ("application/gzip");

    // Scripts / Code
    if (filename.has_suffix(".php") ) return ("application/x-httpd-php");
    if (filename.has_suffix(".py")  ) return ("text/x-python");
    if (filename.has_suffix(".cpp") ) return ("text/x-c++src");
    if (filename.has_suffix(".c")   ) return ("text/x-csrc");
    if (filename.has_suffix(".sh")  ) return ("application/x-sh");

    return ("text/plain");
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

enum	WEBSERV_Interest_Type {
	WEBSERV_INTEREST_SERVER,
	WEBSERV_INTEREST_CLIENT,
	WEBSERV_INTEREST_PROCESS,
};
struct	WEBSERV_Interest {
	i32						fd;
	WEBSERV_Interest_Type	type;

	i64						timestamp; /* NOTE(xenobas): Last event timestamp */
	i32						server_idx; /* NOTE(xenobas): context.config.instances[server_idx] */

	HTTP_Request			client_req;
	HTTP_Response			client_res;
	i32						client_process_fd;
	b32						client_process_await;
	struct sockaddr_in		client_sockaddr;

	i32						process_id;
	struct {
		b32					done;

		i32					read;
		i32					write;
		i32					parent;
		i32					remote[2];
	}						process_fds;
	char**					process_envp;
	char**					process_argv;

	dynamic_array<byte>		process_read_stream;

	dynamic_array<byte>		process_write_stream;
	i32						process_write_offset;
};
struct	WEBSERV_Context {
	WEBSERV_Config			config;
	b32						ok;

	i32							fd_events;
	i64_table<WEBSERV_Interest>	interests;
};

string_view		strconv_i64(i64 n) {
	string_builder	b;

	b.write(n);
	return (b.to_string());
}

dynamic_array<byte>	HTTP_request_body(const HTTP_Request& req) {
	dynamic_array<byte>	body;

	for (i32 chunk_index = 0; chunk_index < req.chunks.len; ++chunk_index) {
		const HTTP_Chunk&	chunk = req.chunks[chunk_index];

		body.push(chunk.size, &req.buff[chunk.index]);
	}
	return (body);
}

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
	switch (interest.type) {
		case WEBSERV_INTEREST_SERVER: {
			::close(interest.fd);
		} break ;
		case WEBSERV_INTEREST_CLIENT: {
			::close(interest.fd);
			HTTP_request_delete(interest.client_req);
			HTTP_response_delete(interest.client_res);
		} break ;
		case WEBSERV_INTEREST_PROCESS: {
			interest.process_read_stream.free();
			interest.process_write_stream.free();

			char**	process_argv = interest.process_argv;
			for (i32 i = 0; process_argv[i]; ++i) {
				delete[]	process_argv[i];
			}
			delete[]		process_argv;

			char**	process_envp = interest.process_envp;
			for (i32 i = 0; process_envp[i]; ++i) {
				delete[]	process_envp[i];
			}
			delete[]		process_envp;

			i32	fd_read = interest.process_fds.read;
			if (fd_read >= 0) {
				::close(fd_read);
			}

			i32	fd_write = interest.process_fds.write;
			if (fd_write >= 0) {
				::close(fd_write);
			}

			HTTP_request_delete(interest.client_req);
			HTTP_response_delete(interest.client_res);
		} break ;
	}
}

b32				WEBSERV_filedesc_cloexec(i32 fd) {
    if (fd < 0) {
        return (0);
    }

    i32 ret_flags = ::fcntl(fd, F_GETFD);
    if (ret_flags == -1) {
        return (0);
    }

    i32 ret_set = ::fcntl(fd, F_SETFD, ret_flags | FD_CLOEXEC);
    if (ret_set == -1) {
        return (0);
    }

    i32 ret_flags_new = ::fcntl(fd, F_GETFD);
    if (ret_flags_new == -1 || !(ret_flags_new & FD_CLOEXEC)) {
        return (0);
    }

    return (1);
}

b32				WEBSERV_context_interest_unregister(WEBSERV_Context& context, WEBSERV_Interest& interest) {
	switch (interest.type) {
	case WEBSERV_INTEREST_SERVER:
	case WEBSERV_INTEREST_CLIENT: {
		i32	ret_ctl = ::epoll_ctl(context.fd_events, EPOLL_CTL_DEL, interest.fd, NULL);
		if (ret_ctl == -1) {
			CLI_show_error_runtime("Could not remove interest from epoll's internal data structure");
			CLI_show_extra("Reason", "%m");
			return (0);
		}
	} break ;
	case WEBSERV_INTEREST_PROCESS: {
		if (context.interests.has(interest.process_fds.parent)) {
			WEBSERV_Interest&	parent = context.interests.get(interest.process_fds.parent);

			parent.timestamp = OS_timestamp_now();
			parent.client_process_await = 0;
			parent.client_process_fd = -1;
		}

		if (!interest.process_fds.done && interest.process_fds.read >= 0) { /* NOTE(xenobas): fd_read */
			i32	ret_ctl = ::epoll_ctl(context.fd_events, EPOLL_CTL_DEL, interest.process_fds.read, NULL);
			if (ret_ctl == -1) {
				CLI_show_error_runtime("Could not remove process read interest from epoll's internal data structure");
				CLI_show_extra("Reason", "%m");
				return (0);
			}
		}
		if (!interest.process_fds.done && interest.process_fds.write >= 0) { /* NOTE(xenobas): fd_write */
			i32	ret_ctl = ::epoll_ctl(context.fd_events, EPOLL_CTL_DEL, interest.process_fds.write, NULL);
			if (ret_ctl == -1) {
				CLI_show_error_runtime("Could not remove process write interest from epoll's internal data structure");
				CLI_show_extra("Reason", "%m");
				return (0);
			}
		}
	} break ;
	}
	return (1);
}
void			WEBSERV_context_interest_delete(WEBSERV_Context& context, WEBSERV_Interest& interest) {
	switch (interest.type) {
		case WEBSERV_INTEREST_SERVER: {
			CLI_debug("Server %d has been deleted from interests", interest.server_idx);
		} break ;
		case WEBSERV_INTEREST_CLIENT: {
			CLI_debug("Client %u.%u.%u.%u:%u has been deleted from interests",
						(cast(u8*)(&interest.client_sockaddr.sin_addr.s_addr))[0],
						(cast(u8*)(&interest.client_sockaddr.sin_addr.s_addr))[1],
						(cast(u8*)(&interest.client_sockaddr.sin_addr.s_addr))[2],
						(cast(u8*)(&interest.client_sockaddr.sin_addr.s_addr))[3],
						::ntohs(interest.client_sockaddr.sin_port));
		} break ;
		case WEBSERV_INTEREST_PROCESS: {
			CLI_debug("Process %d has been deleted from interests", interest.process_id);
		} break ;
	}
	WEBSERV_context_interest_unregister(context, interest);

	WEBSERV_interest_close(interest);
	context.interests.unset(interest.fd);
}
b32				WEBSERV_context_interest_make_client(WEBSERV_Context& context, i32 srv, i32 fd, struct sockaddr_in& sockaddr) {
	WEBSERV_Interest	interest; MEM_zero(interest);

	interest.fd = fd;
	interest.type = WEBSERV_INTEREST_CLIENT;

	interest.timestamp = OS_timestamp_now();
	interest.server_idx = srv;

	interest.client_req = HTTP_request_make();
	interest.client_sockaddr = sockaddr;
	interest.client_process_fd = -1;
	interest.client_process_await = 0;

	struct epoll_event	event_register; MEM_zero(event_register);
	event_register.events  = EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR;
	event_register.data.fd = fd;

	i32					ret_ctl = ::epoll_ctl(context.fd_events, EPOLL_CTL_ADD, fd, &event_register);
	if (ret_ctl == -1) {
		WEBSERV_interest_close(interest);

		CLI_show_error_runtime("Could not add client to epoll's internal data structure");
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

	interest.timestamp = OS_timestamp_now();
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
b32				WEBSERV_context_interest_make_process(WEBSERV_Context& context,
										i32 id, dynamic_array<byte> stream, char** envp, char** argv, i32 fd_read, i32 fd_write, i32 fd_parent) {
	i32					fd = fd_read;
	WEBSERV_Interest	interest; MEM_zero(interest);

	interest.fd = fd;
	interest.type = WEBSERV_INTEREST_PROCESS;

	if (context.interests.has(fd_parent)) {
		WEBSERV_Interest&	parent = context.interests.get(fd_parent);

		parent.client_process_await = 1;
		parent.client_process_fd = fd;
		interest.server_idx = parent.server_idx;
	}

	interest.timestamp = OS_timestamp_now();

	interest.process_id = id;
	interest.process_envp = envp;
	interest.process_argv = argv;
	interest.process_fds.done = 0;
	interest.process_fds.read = fd_read;
	interest.process_fds.write = fd_write;
	interest.process_fds.parent = fd_parent;
	interest.process_read_stream = dynamic_array<byte>();
	interest.process_write_stream = stream;
	interest.process_write_offset = 0;

	{
		struct epoll_event	event_register; MEM_zero(event_register);
		event_register.events  = EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR;
		event_register.data.fd = fd_read;

		i32					ret_ctl = ::epoll_ctl(context.fd_events, EPOLL_CTL_ADD, fd_read, &event_register);
		if (ret_ctl == -1) {
			WEBSERV_interest_close(interest);

			CLI_show_error_runtime("Could not add process to epoll's internal data structure");
			CLI_show_extra("Reason", "%m");

			::close(fd);
			return (0);
		}
	}
	{
		struct epoll_event	event_register; MEM_zero(event_register);
		event_register.events  = EPOLLOUT | EPOLLRDHUP | EPOLLHUP | EPOLLERR;
		event_register.data.fd = fd_read;

		i32					ret_ctl = ::epoll_ctl(context.fd_events, EPOLL_CTL_ADD, fd_write, &event_register);
		if (ret_ctl == -1) {
			WEBSERV_interest_close(interest);

			CLI_show_error_runtime("Could not add process to epoll's internal data structure");
			CLI_show_extra("Reason", "%m");

			::close(fd);
			return (0);
		}
	}
	
	context.interests.set(interest.fd, interest);
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
		CLI_show_error_runtime("Could not set server socket to reusable mode");
		CLI_show_extra("Reason", "%m");

		::close(fd);
		context.ok = 0;

		return ;
	}

	if (!WEBSERV_filedesc_cloexec(fd)) {
		::close(fd);

		CLI_show_error_runtime("Could not set server file descriptor to close-on-exec");
		CLI_show_extra("Reason", "%m");

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

HTTP_Response&	WEBSERV_context_response(WEBSERV_Context& context, WEBSERV_Interest& interest, HTTP_Status status_code) {
	HTTP_Response		response;

	response.status_code = status_code;

	response.headers = HTTP_Headers();
	response.headers.case_insensitive = 1;

	response.is_file = 0;
	response.content_body.len = 0;
	response.content_body.bytes = NULL;

	response.write_idx = 0;
	response.write_str = string_view();

	unused(context);
	interest.client_res = response;
	return (interest.client_res);
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
void			WEBSERV_context_response_from_content(WEBSERV_Context& context, WEBSERV_Interest& interest, HTTP_Status status_code, string_view content, string_view mime = "text/html", b32 content_allocated = 0) { /* Can be either chunked or regular stream */
	HTTP_Response	response;

	response.status_code = status_code;

	response.headers = HTTP_Headers();
	response.headers.case_insensitive = 1;

	string_view		content_owned = content;
	if (!content_allocated) {
		content_owned = string_view::alloc(content);
	}

	response.is_file = 0;
	response.content_body.len = cast(i32)content_owned.len;
	response.content_body.bytes = cast(byte*)content_owned.text;

	response.headers.set("Content-Type", string_view::alloc(mime));
	response.headers.set("Content-Length", strconv_i64(response.content_body.len));

	string_builder	message;
	WEBSERV_response_message_write_prelude(response, message);
	message.write(content_owned);

	response.write_idx = 0;
	response.write_str = message.to_string();

	unused(context);
	interest.client_res = response;
}
void			WEBSERV_context_response_from_redirect(WEBSERV_Context& context, WEBSERV_Interest& interest, HTTP_Status status_code, string_view location) {
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

	response.headers.set("Location", string_view::alloc(location));
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

char**			WEBSERV_context_response_cgi_env(WEBSERV_Context& context, WEBSERV_Interest& interest, HTTP_Request& req, const string_view& script_name, const string_view& path_info) {
	dynamic_array<char*>		env;
	string_view					entry;

	const WEBSERV_Instance&		instance = context.config.instances[interest.server_idx];

	entry = ENV_string("GATEWAY_INTERFACE", "CGI/1.1");
	env.push(entry.text);

	entry = ENV_string("PATH_INFO", path_info);
	env.push(entry.text);

	string_view	params_string;
	i32			params_start = req.uri.str.index("?");
	if (params_start >= 0)
		params_string = req.uri.str.slice(params_start + 1);
	entry = ENV_string("QUERY_STRING", params_string);
	env.push(entry.text);

	entry = ENV_string("REQUEST_METHOD", WEBSERV_method_cstr(req.method));
	env.push(entry.text);

	entry = ENV_string("SCRIPT_FILENAME", script_name);
	env.push(entry.text);

	entry = ENV_string("SERVER_PORT", cast(i64)instance.port);
	env.push(entry.text);

	entry = ENV_string("SERVER_PROTOCOL", req.protocol);
	env.push(entry.text);

	if (req.headers.has("Authorization")) {
		entry = ENV_string("AUTH_TYPE", req.headers.get("Authorization"));
		env.push(entry.text);
	}
	if (req.headers.has("Content-Length")) {
		entry = ENV_string("CONTENT_LENGTH", req.headers.get("Content-Length"));
		env.push(entry.text);
	}
	if (req.headers.has("Content-Type")) {
		entry = ENV_string("CONTENT_TYPE", req.headers.get("Content-Type"));
		env.push(entry.text);
	}

	/* NOTE(xenobas): Required to support php-cgi */
	entry = ENV_string("REDIRECT_STATUS", "200");
	env.push(entry.text);

	for_table_begin(req.headers, const hash_table<string_view>, header) {
		entry = ENV_string(header.key, header.value, /* is_header = */ 1);
		env.push(entry.text);
	} for_table_end;

	/* NOTE(xenobas): NULL terminated array of strings */
	env.push(NULL);

	return (cast(char**)env.data);
}
char**			WEBSERV_context_response_cgi_args(WEBSERV_Context& context, WEBSERV_Interest& interest, HTTP_Request& req, const string_view& interpreter_path, const string_view& script_path) {
	dynamic_array<char*>		args;
	string_view					arg;

	unused(context);
	unused(interest);
	unused(req);

	arg = string_view::alloc(interpreter_path);
	args.push(arg.text);

	arg = string_view::alloc(script_path);
	args.push(arg.text);

	args.push(NULL);
	return ((char**)args.data);
}

void			WEBSERV_context_response_make(WEBSERV_Context& context, WEBSERV_Interest& interest) {
	if (interest.type != WEBSERV_INTEREST_CLIENT) {
		return ;
	}

	HTTP_Request&		req = interest.client_req;
	WEBSERV_Instance&	instance = context.config.instances[interest.server_idx];

	if (HTTP_request_is_error(req)) {
		WEBSERV_context_response_from_status(context, interest, HTTP_STATUS_BAD_REQUEST);
		return ;
	}
	/* TODO(xenobas): Can check for host later */

	string_view			_route_id = WEBSERV_http_route_pick(instance, req);
	if (!instance.routes.has(_route_id)) {
		WEBSERV_context_response_from_status(context, interest, HTTP_STATUS_NOT_FOUND);
		return ;
	}
	WEBSERV_Route&		route = instance.routes.get(_route_id);
	if ((req.method & route.methods_whitelist) == 0x0) {
		WEBSERV_context_response_from_status(context, interest, HTTP_STATUS_METHOD_NOT_ALLOWED);
		return ;
	}

	i32					builder_index = 0;

	string_builder		builder_path_match;
	builder_path_match.write('/');
	for (i32 i = 0; builder_index < req.uri.path.len && builder_index < route.uri.path.len; ++builder_index, ++i) {
		if (req.uri.path[builder_index] != route.uri.path[builder_index]) {
			break ;
		}

		if (i > 0) {
			builder_path_match.write('/');
		}
		builder_path_match.write(req.uri.path[builder_index]);
	}

	string_view			cgi_script_file;
	if (route.kind == WEBSERV_ROUTE_CGI && builder_index < req.uri.path.len) { /* NOTE(xenoas): Add script location */
		builder_path_match.write('/');
		builder_path_match.write(req.uri.path[builder_index]);

		cgi_script_file = req.uri.path[builder_index];

		++builder_index;
	}

	string_builder		builder_path_extra;
	if (!req.uri.is_file || builder_index < req.uri.path.len) {
		builder_path_extra.write('/');
	}
	for (i32 i = 0; builder_index < req.uri.path.len; ++builder_index, ++i) {

		if (i > 0) {
			builder_path_extra.write('/');
		}
		builder_path_extra.write(req.uri.path[builder_index]);
	}

	i32			fd_client = interest.fd;
	string_view	path_match = builder_path_match.to_view();
	string_view	path_extra = builder_path_extra.to_view();

	switch (route.kind) {
		case WEBSERV_ROUTE_SERVER: {
			b32					files_list = route.Server.directory_list;
			const string_view&	files_directory = route.Server.directory;

			string_builder		builder_resource_path;
			builder_resource_path.write(files_directory);
			builder_resource_path.write(path_extra);

			string_view	resource_path = builder_resource_path.to_view();
			builder_resource_path.write('\0'); /* NOTE(xenobas): NULL-terminated for passing directly to syscalls */

			if (OS_test_file_read(resource_path)) { /* NOTE(xenobas): File check */
				CLI_debug("Client is being served file at \"%.*s\"",
							resource_path.len, resource_path.text);

				i64			resource_size = OS_file_size(resource_path.text);
				if (resource_size == -1) {
					CLI_show_error_runtime("Could not query file \"%s\" size", resource_path.text);
					if (errno != 0) {
						CLI_show_extra("Reason", "%m");
					}

					WEBSERV_context_response_from_status(context, interest, HTTP_STATUS_SERVER_ERROR);
					return ;
				}

				string_view	resource_mime = HTTP_mime_from_name(resource_path);
				if (resource_size > CONTEXT_STREAM_BASELINE) { /* Transfer-Encoding: chunked */
					WEBSERV_context_response_from_status(context, interest, HTTP_STATUS_NOT_IMPLEMENTED);
					return ;
				}

				string_view	resource_data;
				if (!OS_read_file(resource_path, resource_data)) {
					CLI_show_error_runtime("Could not query file \"%s\" size", resource_path.text);
					CLI_show_extra("Reason", "%m");

					WEBSERV_context_response_from_status(context, interest, HTTP_STATUS_SERVER_ERROR);
					return ;
				}

				WEBSERV_context_response_from_content(context, interest, HTTP_STATUS_OK, resource_data, /* mime = */ resource_mime, /* content_allocated = */ 1);
				return ;
			}

			if (!OS_test_dir_read(resource_path)) { /* NOTE(xenobas): Directory check */
				WEBSERV_context_response_from_status(context, interest, HTTP_STATUS_NOT_FOUND);
				return ;
			}
			if (files_list) {
				CLI_debug("Client is being served directory listing at \"%.*s\"",
							resource_path.len, resource_path.text);

				string_builder	builder_html;

				DIR*			dir_handle = ::opendir(resource_path.text);
				if (dir_handle == NULL) {
					CLI_show_error_runtime("Client could not be served directory listing");
					CLI_show_extra("Reason", "%m");

					return ;
				}

				builder_html.write(
					"<!DOCTYPE html>\n"
					"<html>\n"
					"  <head>\n"
					"    <title>Directory Listing for ");
				if (!path_extra.has_prefix("/")) {
					builder_html.write("/");
				}
				builder_html.write(path_extra);
				builder_html.write("</title>\n"
					"  </head>\n"
					"  <body>\n"
					"    <h1>Directory Listing for "
				);
				if (!path_extra.has_prefix("/")) {
					builder_html.write("/");
				}
				builder_html.write(path_extra);
				builder_html.write("</h1>\n"
					"    <ul>\n");

				errno = 0;
				struct dirent*	dir_entry = NULL;
				while ((dir_entry = ::readdir(dir_handle))) {
					const char*	name_cstr = dir_entry->d_name;

					string_view	name(name_cstr);
					if (name == "." || name == "..") continue ;

					string_builder	builder_href;
					if (req.uri.path.len == 0) {
						builder_href.write("/");
					}
					for (i32 i = 0; i < req.uri.path.len; ++i) {
						builder_href.write("/");
						builder_href.write(req.uri.path[i]);
					}
					{
						builder_href.write("/");
						builder_href.write(name);
					}
					string_view		href = builder_href.to_view();
					
					string_builder	builder_attr;
					if (dir_entry->d_type == DT_DIR) {
						builder_attr.write("data-folder ");
					}
					string_view		attr = builder_attr.to_view();

					builder_html.write("    <li>" "<a href=\"");
					builder_html.write(href);
					builder_html.write("\" ");
					builder_html.write(attr);
					builder_html.write(">");
					builder_html.write(name);
					builder_html.write("</a>" "</li>" "\n");
				}
				::closedir(dir_handle);

				builder_html.write("    </ul>\n");
				builder_html.write("  </body>\n");
				builder_html.write("</html>");

				if (errno != 0) {
					CLI_show_error_runtime("Client could not be served directory listing");
					CLI_show_extra("Reason", "%m");

					return ;
				}
				
				string_view		html = builder_html.to_view();
				WEBSERV_context_response_from_content(context, interest, HTTP_STATUS_OK, html, /* mime = */ "text/html");
				return ;
			}

			if (HTTP_mime_from_name(resource_path) != "text/plain") {
				WEBSERV_context_response_from_status(context, interest, HTTP_STATUS_NOT_FOUND);
				return ;
			}
			builder_resource_path.pop();

			if (!resource_path.has_suffix("/")) {
				builder_resource_path.write("/");
			}
			builder_resource_path.write("index.html");

			string_view	fallback_data;
			string_view	fallback_path = builder_resource_path.to_view();
			if (!OS_read_file(fallback_path, fallback_data)) {
				WEBSERV_context_response_from_status(context, interest, HTTP_STATUS_NOT_FOUND);
				return ;
			}

			CLI_debug("Client is being served fallback at \"%.*s\"",
						fallback_path.len, fallback_path.text);
			WEBSERV_context_response_from_content(context, interest, HTTP_STATUS_OK, fallback_data, /* mime = */ HTTP_mime_from_name(fallback_path), /* content_allocated = */ 1);
			return ;
		} break ;
		case WEBSERV_ROUTE_UPLOAD: {
		} break ;
		case WEBSERV_ROUTE_REDIRECT: {
			const string_view&	status = route.Redirect.status;
			const string_view&	location = route.Redirect.location;
			HTTP_Status			status_code = HTTP_status_from_string(status);

			CLI_debug("Client redirected from \"%.*s\" to \"%.*s\"",
						path_match.len, path_match.text,
						location.len, location.text);

			WEBSERV_context_response_from_redirect(context, interest, status_code, location);
			return ;
		} break ;
		case WEBSERV_ROUTE_CGI: {
			/* TODO(xenobas): Use env from CGI config */

			/* SECTION(xenobas): Process path builder */
			string_builder	process_path_builder;
			
			string_view		directory = route.CGI.directory;
			process_path_builder.write(directory);
			if (!directory.has_suffix("/")) process_path_builder.write("/");

			process_path_builder.write(cgi_script_file);

			string_view			process_path = process_path_builder.to_view();

			/* SECTION(xenobas): Interpreter picker */

			string_view			interpreter_path;
			for_table_begin(route.CGI.interpreters, const hash_table<string_view>, kv) {
				const string_view&	ext = kv.key;
				const string_view&	path = kv.value;

				if (process_path.has_suffix(ext) && process_path.len > ext.len + 1 && process_path[process_path.len - ext.len - 1] == '.') {
					interpreter_path = path;
					continue ;
				}
			} for_table_end;

			if (!interpreter_path) {
				CLI_debug("CGI Script at \"%.*s\" has no interpreters configured", process_path.len, process_path.text);

				WEBSERV_context_response_from_status(context, interest, HTTP_STATUS_NOT_FOUND);

				return ;
			}
			if (!OS_access_file(process_path, F_OK)) {
				CLI_debug("CGI Script at \"%.*s\" does not exist", process_path.len, process_path.text);

				WEBSERV_context_response_from_status(context, interest, HTTP_STATUS_NOT_FOUND);

				return ;
			}

			CLI_debug("CGI Script at \"%.*s\" is to be run via \"%.*s\"", process_path.len, process_path.text, interpreter_path.len, interpreter_path.text);

			i32					process_pipe_in[2];
			i32					ret_pipe_in  = ::pipe(process_pipe_in);
			if (ret_pipe_in == -1) {
				CLI_show_error_runtime("Could not create pipe for process input");
				CLI_show_extra("Reason", "%m");

				WEBSERV_context_response_from_status(context, interest, HTTP_STATUS_SERVER_ERROR);

				return ;
			}

			i32					process_pipe_out[2];
			i32					ret_pipe_out = ::pipe(process_pipe_out);
			if (ret_pipe_out == -1) {
				::close(process_pipe_in[0]);
				::close(process_pipe_in[1]);

				CLI_show_error_runtime("Could not create pipe for process output");
				CLI_show_extra("Reason", "%m");

				WEBSERV_context_response_from_status(context, interest, HTTP_STATUS_SERVER_ERROR);

				return ;
			}

			char**				process_envp = WEBSERV_context_response_cgi_env(context, interest, req, process_path, path_extra);
			char**				process_argv = WEBSERV_context_response_cgi_args(context, interest, req, interpreter_path, process_path);

			#if 0
			{
				CLI_debug("Process \"%.*s\" environment", process_path.len, process_path.text);
				for (i32 i = 0; process_envp[i]; ++i) {
					CLI_debug("\t%s", process_envp[i]);
				}
			}
			#endif

			i32					process_id = ::fork();
			b32					process_ok = 1;
			b32					process_fatal = 0;
			if (process_id > 0) { /* SECTION(xenobas): Parent process */
				i32					fd_read = process_pipe_in[0];
				i32					fd_write = process_pipe_out[1];
				i32					fd_parent = fd_client;

				i32					fd_remote[2] = { process_pipe_in[1], process_pipe_out[0] };
				for (i32 i = 0; i < 2; ++i) ::close(fd_remote[i]);

				dynamic_array<byte>	process_stream_write = HTTP_request_body(req);
				process_ok = WEBSERV_context_interest_make_process(context, process_id, process_stream_write, process_envp, process_argv, fd_read, fd_write, fd_parent);
				if (process_ok) {
					CLI_debug("Created Process with ID %d", process_id);
				}

				return ;
			} else if (process_id == 0) { /* SECTION(xenobas): Child process */
				i32					fd_read = process_pipe_out[0];
				if (::dup2(fd_read, FD_STDIN) == -1) {
					CLI_show_error_runtime("Could not duplicate file descriptor for process input");
					CLI_show_extra("Reason", "%m");

					process_ok = 0;
					context.ok = 0;
					goto CGI_label_cleanup;
				}
                ::close(fd_read); process_pipe_out[0] = -1;

				i32					fd_write = process_pipe_in[1];
				if (::dup2(fd_write, FD_STDOUT) == -1) {
					CLI_show_error_runtime("Could not duplicate filed descriptor for process output");
					CLI_show_extra("Reason", "%m");

					process_ok = 0;
					context.ok = 0;
					goto CGI_label_cleanup;
				}
                ::close(fd_write); process_pipe_in[1] = -1;

				i32					fd_remote[2] = { process_pipe_in[0], process_pipe_out[1] };
				for (i32 i = 0; i < 2; ++i) ::close(fd_remote[i]);

				i32					ret_exec = ::execve(process_argv[0], process_argv, process_envp);
				if (ret_exec == -1) {
					CLI_show_error_runtime("CGI script execution threw an error");
					CLI_show_extra("Reason", "%m");

					context.ok = 0;
					process_ok = 0;
					process_fatal = 1;

					goto CGI_label_cleanup;
				}
			} else if (process_id == -1) {
				CLI_show_error_runtime("Could not create fork process for CGI");
				CLI_show_extra("Reason", "%m");

				WEBSERV_context_response_from_status(context, interest, HTTP_STATUS_SERVER_ERROR);

				process_ok = 0;
			}
CGI_label_cleanup:
			if (!process_ok) {
				for (i32 i = 0; i < 2; ++i) {
                    if (process_pipe_in[i] >= 0) {
                        ::close(process_pipe_in[i]);
                    }
                    if (process_pipe_out[i] >= 0) {
                        ::close(process_pipe_out[i]);
                    }
				}
				for (i32 i = 0; process_argv[i]; ++i) {
					delete[]	process_argv[i];
				}
				delete[]		process_argv;
				for (i32 i = 0; process_envp[i]; ++i) {
					delete[]	process_envp[i];
				}
				delete[]		process_envp;
			}
			if (!context.ok) {
				if (process_fatal) {
					::exit(32);
				}
				return ;
			}
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

			/* TODO(xenobas): Close connection */
		}
		else if (write_ret >= 0) {
			res.write_idx += cast(i32)write_ret;
		}

		interest.timestamp = OS_timestamp_now();
		return (0);
	}

	if (interest.client_process_fd == -1) {
		WEBSERV_context_interest_delete(context, interest);
		return (1);
	}
	return (0);
}
void			WEBSERV_context_process_write(WEBSERV_Context& context, WEBSERV_Interest& interest) {
	if (interest.type != WEBSERV_INTEREST_PROCESS) {
		return ;
	}
	unused(context);

	i32							fd = interest.process_fds.write;
	const dynamic_array<byte>&	stream = interest.process_write_stream;
	i32&						chunk_idx = interest.process_write_offset;
	const i32					chunk_cap = 1024 * 8;
	if (chunk_idx < stream.len) {
		byte*		write_arr = cast(byte*)(&stream[chunk_idx]);
		i32			write_cap = (stream.len - chunk_idx >= chunk_cap) ? (chunk_cap) : (stream.len - chunk_idx);

		i64			write_ret = ::write(fd, write_arr, cast(u64)write_cap);
		if (write_ret == -1) {
			CLI_show_error_runtime("Could not write response to client");
			CLI_show_extra("Reason", "%m");

		}
		else if (write_ret >= 0) {
			chunk_idx += cast(i32)write_ret;
		}

		interest.timestamp = OS_timestamp_now();
		if (context.interests.has(interest.process_fds.parent)) {
			WEBSERV_Interest&	parent = context.interests.get(interest.process_fds.parent);
			parent.timestamp = interest.timestamp;
		}
		return ;
	}
	if (chunk_idx >= stream.len) {
		i32	ret_ctl = ::epoll_ctl(context.fd_events, EPOLL_CTL_DEL, interest.process_fds.write, NULL);
		if (ret_ctl == -1) {
			CLI_show_error_runtime("Could not remove process write interest from epoll's internal data structure");
			CLI_show_extra("Reason", "%m");

			return ;
		}

		::close(interest.process_fds.write);
		interest.process_fds.write = -1;
	}
}
b32				WEBSERV_context_process_watch(WEBSERV_Context& context, WEBSERV_Interest& interest) {
	if (interest.type != WEBSERV_INTEREST_PROCESS) return (0);
	if (!interest.process_fds.done) return (0);

	i32	fd_parent = interest.process_fds.parent;
	i32	process_id = interest.process_id;
	if (!context.interests.has(fd_parent)) {
		CLI_show_error_runtime("Process %d parent has disappeared", process_id);

		WEBSERV_context_interest_delete(context, interest);
		return (1);
	}

	WEBSERV_Interest&		parent = context.interests.get(fd_parent);
	dynamic_array<byte>&	stream = interest.process_read_stream;
	string_view				content = string_view(cast(char*)stream.data, stream.len);

	i32	process_status = 0;
	i32	process_waited = ::waitpid(process_id, &process_status, WNOHANG);
	if (process_waited == -1) {
		CLI_show_error_runtime("Process %d could not query status", process_id);
		CLI_show_extra("Reason", "%m");

		WEBSERV_context_response_from_status(context, parent, HTTP_STATUS_SERVER_ERROR);
		WEBSERV_context_interest_delete(context, interest);
		return (1);
	}
	if (process_waited ==  0) { /* NOTE(xenobas): State has not changed */
		return (0);
	}

	process_status = WEXITSTATUS(process_status);
	if (process_status != 0) {
		CLI_show_error_runtime("Process %d exited abnormally with exit status %d", process_id, process_status);

		// parent.process_await = 0;
		WEBSERV_context_response_from_status(context, parent, HTTP_STATUS_BAD_GATEWAY);
		WEBSERV_context_interest_delete(context, interest);
		return (1);
	}

	CLI_debug("Process %d exited normally", process_id);

	HTTP_Response&		response = WEBSERV_context_response(context, parent, HTTP_STATUS_OK);

	string_view			content_header = string_view();
	b32					content_length_is_set = 0;
	HTTP_Status			content_status = HTTP_STATUS_OK;
	/* CLI_debug("Process Headers"); */
	while (content.split_iter("\n", content_header)) {
		if (content_header.len == 0) break ;
		
		string_view	name;
		content_header.split_iter(":", name);

		string_view	value = content_header.trim();
		if (!name || !value) break ;

		/* CLI_debug("\t%.*s: %.*s", name.len, name.text, value.len, value.text); */

		response.headers.set(name, string_view::alloc(value));
		if (name.eq_insensitive("Content-Length")) {
			content_length_is_set = 1;
		}
		if (name.eq_insensitive("Status") && value.len > 3) {
			string_view	status_code = value.slice(0, 3);

			content_status = HTTP_status_from_string(status_code);
		}
	}

	response.status_code = content_status;

	string_view			content_body = string_view::alloc(content);
	response.content_body.len = cast(i32)content_body.len;
	response.content_body.bytes = cast(byte*)content_body.text;

	if (!content_length_is_set) {
		string_builder	content_length;

		content_length.write(cast(i64)response.content_body.len);
		response.headers.set("Content-Length", content_length.to_string());
	}

	{
		string_builder	content_builder;
		WEBSERV_response_message_write_prelude(response, content_builder);
		content_builder.write(content_body);

		response.write_idx = 0;
		response.write_str = content_builder.to_string();
	}
	WEBSERV_context_interest_delete(context, interest);

	CLI_debug("Process %d response has been queued for writing to client", process_id);
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
    if (!WEBSERV_filedesc_cloexec(context.fd_events)) {
        ::close(context.fd_events);

		CLI_show_error_runtime("Could not set events file descriptor as close-on-exec");
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
	for (i32 req_count = 0; req_count < 32;) {
		if (!context.ok) break;

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
			if (!context.ok) break;

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
					if (!WEBSERV_filedesc_cloexec(fd_client)) {
						::close(fd_client);

						CLI_show_error_runtime("Could not set client file descriptor to close-on-exec");
						CLI_show_extra("Reason", "%m");

						continue ;
					}

					if (WEBSERV_context_interest_make_client(context, server_idx, fd_client, sockaddr)) {
						CLI_debug("Accepted connection from %u.%u.%u.%u:%u described by %d",
							(cast(u8*)(&sockaddr.sin_addr.s_addr))[0],
							(cast(u8*)(&sockaddr.sin_addr.s_addr))[1],
							(cast(u8*)(&sockaddr.sin_addr.s_addr))[2],
							(cast(u8*)(&sockaddr.sin_addr.s_addr))[3],
							::ntohs(sockaddr.sin_port),
							fd_client
						);
					}
				} break ;
				case WEBSERV_INTEREST_CLIENT: {
					i32	fd_client = interest.fd;
					b32	event_type_stop = event.events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR);
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

						if (interest.client_process_fd >= 0 && context.interests.has(interest.client_process_fd)) {
							WEBSERV_Interest&	child = context.interests.get(interest.client_process_fd);

							::kill(child.process_id, SIGKILL);
							WEBSERV_context_interest_delete(context, child);
						}
						WEBSERV_context_interest_delete(context, interest);
						CLI_debug("Client has disconnected", fd_client);
						continue ;
					}
					if (event_type_read) {
						const u64	buff_cap = CONTEXT_BUFFER_CAPACITY;
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

						CLI_debug("Client has sent %d bytes", buff_len);

						if (!HTTP_request_read(interest.client_req, buff_arr, cast(i32)buff_len)) {
							CLI_show_error_runtime("Client sent unsupported HTTP request");
						}
						if (HTTP_request_is_done(interest.client_req)) {
							struct epoll_event	event_mod; MEM_zero(event_mod);
							event_mod.events = EPOLLOUT | EPOLLRDHUP | EPOLLHUP | EPOLLERR;
							event_mod.data.fd = interest.fd;

							i32					ret_ctl = ::epoll_ctl(context.fd_events, EPOLL_CTL_MOD, fd_client, &event_mod);
							if (ret_ctl == -1) {
								CLI_show_error_runtime("Could not modify interest inside epoll's internal data structure");

								WEBSERV_context_interest_delete(context, interest);
								continue ;
							}

							WEBSERV_context_response_make(context, interest);
						}

						interest.timestamp = OS_timestamp_now();
						continue ;
					}
					if (event_type_write) {
						b32	res_done = WEBSERV_context_response_write(context, interest);
						if (res_done) {
							req_count++;
						}
						continue ;
					}
				} break ;
				case WEBSERV_INTEREST_PROCESS: {
					i32	fd_read = interest.process_fds.read;
					i32	fd_parent = interest.process_fds.parent;
					i32	process_id = interest.process_id;

					b32	event_type_stop = event.events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR);
					if (event_type_stop) {
						if (event.events & EPOLLERR) {
							CLI_show_error_runtime("Process %d had an error occur", process_id);
						}

						i32	ret_ctl = ::epoll_ctl(context.fd_events, EPOLL_CTL_DEL, interest.process_fds.read, NULL);
						if (ret_ctl == -1) {
							CLI_show_error_runtime("Could not remove process read interest from epoll's internal data structure");
							CLI_show_extra("Reason", "%m");

							continue ;
						}

						interest.process_fds.done = 1;
						CLI_debug("Process %d marked for cleanup", process_id);
						continue ;
					}

					b32	event_type_read = event.events & EPOLLIN;
					if (event_type_read) {
						dynamic_array<byte>&	process_stream = interest.process_read_stream;

						const u64	buff_cap = CONTEXT_BUFFER_CAPACITY;
						byte		buff_arr[buff_cap];
						i64			buff_len = ::read(fd_read, buff_arr, buff_cap);
						if (buff_len <= 0) {
							if (buff_len == -1) {
								CLI_show_error_runtime("Failed during read data from client");
								CLI_show_extra("Reason", "%m");
							}

							if (context.interests.has(fd_parent)) {
								WEBSERV_Interest&	parent = context.interests.get(fd_parent);

								WEBSERV_context_response_from_status(context, parent, HTTP_STATUS_BAD_GATEWAY);
							}

							WEBSERV_context_interest_delete(context, interest);
							continue ;
						}

						CLI_debug("Process %d has sent %d bytes", process_id, buff_len);
						process_stream.push(cast(i32)buff_len, buff_arr);

						interest.timestamp = OS_timestamp_now();
						if (context.interests.has(fd_parent)) {
							WEBSERV_Interest&	interest_parent = context.interests.get(fd_parent);

							interest_parent.timestamp = interest.timestamp;
						}
						continue ;
					}

					b32	event_type_write = event.events & EPOLLOUT;
					if (event_type_write) {
						WEBSERV_context_process_write(context, interest);
						continue ;
					}
				} break ;
			}
		}
		if (!context.ok) break ;

		i64	ts_curr = OS_timestamp_now();
		for (i32 interest_idx = 0; interest_idx < context.interests.cap; ++interest_idx) {
			if (!context.interests.items[interest_idx].used) {
				continue ;
			}

			WEBSERV_Interest&	interest = context.interests.items[interest_idx].value;
			if (interest.type == WEBSERV_INTEREST_SERVER) {
				continue ;
			}

			WEBSERV_Instance&	instance = context.config.instances[interest.server_idx];

			if (interest.type == WEBSERV_INTEREST_PROCESS) {
				if (WEBSERV_context_process_watch(context, interest)) {
					continue ;
				}
			}

			i64	ts_last  = interest.timestamp;
			i64	duration = ts_curr - ts_last;
			if (duration < instance.timeout) {
				continue ;
			}

			if (interest.type == WEBSERV_INTEREST_PROCESS) {
				::kill(interest.process_id, SIGKILL);

				i32	fd_parent = interest.process_fds.parent;
				if (context.interests.has(fd_parent)) {
					WEBSERV_Interest&	parent = context.interests.get(fd_parent);

					WEBSERV_context_response_from_status(context, parent, HTTP_STATUS_GATEWAY_TIMEOUT);

					parent.client_process_fd = -1;
					parent.client_process_await = 0;
				}
			}

			if (interest.type == WEBSERV_INTEREST_CLIENT) {
				CLI_debug("Client %u.%u.%u.%u:%u has been kicked due to timeout", 
					(cast(u8 *)&interest.client_sockaddr.sin_addr.s_addr)[0],
					(cast(u8 *)&interest.client_sockaddr.sin_addr.s_addr)[1],
					(cast(u8 *)&interest.client_sockaddr.sin_addr.s_addr)[2],
					(cast(u8 *)&interest.client_sockaddr.sin_addr.s_addr)[3],
					::ntohs(interest.client_sockaddr.sin_port));
			}
			else if (interest.type == WEBSERV_INTEREST_PROCESS) {
				CLI_debug("Process %d has been kicked due to timeout", interest.process_id);
			}
			else {
				CLI_debug("Interest file descriptor %d has been kicked due to timeout", interest.fd);
			}
			WEBSERV_context_interest_delete(context, interest);
		}
		CLI_flush();
	}

	WEBSERV_context_delete(context);
	return (context.ok);
}
