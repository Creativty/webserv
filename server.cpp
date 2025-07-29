#include <cassert>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <cstdlib>
#include <stdio.h>
#include "webserv.hpp"
#include <fstream>
#include <sys/epoll.h>
#include <fcntl.h>

#define PORT 8080

/* TODO(XENOBAS):
 * - "HTTP/1.0 501 Not Implemented"
 * - "HTTP/1.0 505 HTTP Version Not Supported"
 */

void	http_respond_html(int fd, std::string status, std::string type, const std::string &data) {
	std::string	response_line = "HTTP/1.0 " + status + "\r\n";
	std::string	headers = "Content-Type: " + type + "\r\n"
		"\r\n";
	::write(fd, response_line.c_str(), response_line.size());
	::write(fd, headers.c_str(), headers.size());
	if (data.size())
		::write(fd, data.c_str(), data.size());
}

std::string getContentType(const std::string& filename) {
    if (filename.find(".html") != std::string::npos) return "text/html";
    if (filename.find(".htm")  != std::string::npos) return "text/html";
    if (filename.find(".css")  != std::string::npos) return "text/css";
    if (filename.find(".js")   != std::string::npos) return "application/javascript";
    if (filename.find(".json") != std::string::npos) return "application/json";
    if (filename.find(".xml")  != std::string::npos) return "application/xml";
    if (filename.find(".pdf")  != std::string::npos) return "application/pdf";

    // Images
    if (filename.find(".png")  != std::string::npos) return "image/png";
    if (filename.find(".jpg")  != std::string::npos) return "image/jpeg";
    if (filename.find(".jpeg") != std::string::npos) return "image/jpeg";
    if (filename.find(".gif")  != std::string::npos) return "image/gif";
    if (filename.find(".bmp")  != std::string::npos) return "image/bmp";
    if (filename.find(".ico")  != std::string::npos) return "image/x-icon";
    if (filename.find(".svg")  != std::string::npos) return "image/svg+xml";
    if (filename.find(".webp") != std::string::npos) return "image/webp";

    // Audio
    if (filename.find(".mp3")  != std::string::npos) return "audio/mpeg";
    if (filename.find(".wav")  != std::string::npos) return "audio/wav";
    if (filename.find(".ogg")  != std::string::npos) return "audio/ogg";

    // Video
    if (filename.find(".mp4")  != std::string::npos) return "video/mp4";
    if (filename.find(".webm") != std::string::npos) return "video/webm";
    if (filename.find(".avi")  != std::string::npos) return "video/x-msvideo";
    if (filename.find(".mov")  != std::string::npos) return "video/quicktime";
    if (filename.find(".mkv")  != std::string::npos) return "video/x-matroska";

    // Fonts
    if (filename.find(".ttf")  != std::string::npos) return "font/ttf";
    if (filename.find(".otf")  != std::string::npos) return "font/otf";
    if (filename.find(".woff") != std::string::npos) return "font/woff";
    if (filename.find(".woff2")!= std::string::npos) return "font/woff2";

    // Archives
    if (filename.find(".zip")  != std::string::npos) return "application/zip";
    if (filename.find(".rar")  != std::string::npos) return "application/vnd.rar";
    if (filename.find(".7z")   != std::string::npos) return "application/x-7z-compressed";
    if (filename.find(".tar")  != std::string::npos) return "application/x-tar";
    if (filename.find(".gz")   != std::string::npos) return "application/gzip";

    // Scripts / Code
    if (filename.find(".php")  != std::string::npos) return "application/x-httpd-php";
    if (filename.find(".py")   != std::string::npos) return "text/x-python";
    if (filename.find(".cpp")  != std::string::npos) return "text/x-c++src";
    if (filename.find(".c")    != std::string::npos) return "text/x-csrc";
    if (filename.find(".sh")   != std::string::npos) return "application/x-sh";

    return "text/pain";
}

std::string url_decode(const std::string &str) {
	std::string decoded;
	char ch;
	int ii;
	for (size_t i = 0; i < str.length(); i++) {
		if (str[i] == '%') {

			sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
			ch = static_cast<char>(ii);
			decoded += ch;
			i = i + 2;
		} else if (str[i] == '+') {
			decoded += ' ';
		} else {
			decoded += str[i];
		}
	}
	return decoded;
}

int methods(http::Request request, int new_socket){
    std::string path;
    bool file_ok;

    if (request.uri.find("/favicon.ico") == 0) return(1);
    if (request.method == http::HTTP_METHOD_POST){
        path = url_decode(request.body.substr(5));
        // std::cout << "=================post data: '" << path << "'" << std::endl;
        // std::cout << "=================header: '" << request.headers["name"] << "'" << std::endl;

    }
    if (request.uri.find("/DELETE?data=") == 0){
        path = url_decode(request.uri.substr(13));
        if (access(path.c_str(), F_OK)){
            http_respond_html(new_socket, "404 Not Found", "text/pain", "");
            close(new_socket);
            std::cout << "ERROR: cannot open " << path << std::endl;
            return(1);
        }else{
            if (remove(path.c_str())){
                http_respond_html(new_socket, "403 Forbidden", "text/pain", "");
                close(new_socket);
                return(1);
            }
            return(0);
        }
    }
    if (request.uri.find("/GET?data=") == 0){
        path = url_decode(request.uri.substr(10));
        std::string res_file = read_entire_file(path, &file_ok);
        if (res_file.length()){
            assert(file_ok && "could not load template html file");            
            http_respond_html(new_socket, "200 OK", getContentType(path), res_file);
        }
        close(new_socket);
        return(1);
    }
    return(0);
}

/* TODO(XENOBAS): 
 * - We need a better way of error handling, Either we bow down to exceptions (hate it) or we just hand roll our own.
 * - int server(), can be inlined into the main function directly...
 * - server initialisation needs to be its own function since we will be having multiple servers setup...
 * - Improve the current logging, we would like to have it more robust like what's been done with HARL, in the CPP Modules.
 * - HTTP_Writer interface for safely constructing responses.
 * - CGI is still not even considered in the current architecture.
 * - Event loop of the server is currently in chaos.
 */
int server() {
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	int epfd = epoll_create1(0);

	if (server_fd < 0) {
		std::cerr << "Socket creation failed" << std::endl;
		return 2;
	}
	{
		const int reuseaddr = 1;
		setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int));
	}
	fcntl(server_fd, F_SETFL, O_NONBLOCK);
	std::memset((char*)&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(PORT);

	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
		perror("bind failed");
		return 2;
	}

	if (listen(server_fd, SOMAXCONN) < 0) {
		perror("In listen");
		return EXIT_FAILURE;
	}

	bool				file_html_ok;
	const std::string	file_html = read_entire_file("pages/index.html", &file_html_ok);
	assert(file_html_ok && "could not load template html file");

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = server_fd;
	epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev);

	while (true) {
		struct epoll_event events[100];
		int nfds = epoll_wait(epfd, events, 100, -1);
		if (nfds == -1) {
			perror("epoll_wait");
			continue;
		}


		for (int i = 0; i < nfds; i++){
			int fd = events[i].data.fd;

			if (fd == server_fd){
				int new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
				if (new_socket < 0) {
					perror("In accept");
					return EXIT_FAILURE;
				}
				fcntl(new_socket, F_SETFL, O_NONBLOCK);

				struct epoll_event client_ev;
				client_ev.events = EPOLLIN;
				client_ev.data.fd = new_socket;
				if (epoll_ctl(epfd, EPOLL_CTL_ADD, new_socket, &client_ev) == -1) {
					perror("epoll_ctl failed");
					close(new_socket);
					continue;
				}

			}
			else{
				char buffer[1024 * 1024];
				std::memset(buffer, 0, sizeof(buffer));
				ssize_t valread = ::read(fd, buffer, sizeof(buffer));
				

				if (valread > 0) {
					std::string	message(buffer, cast(size_t)valread);
					http::Request request;
					http::Parse_Error error = http::parse_request(message, request);
					if (error != http::PARSE_ERROR_NONE) {
						std::cout << "Could not parse request because of " << error << std::endl;
						//TODO
					}

					std::cout << "->uri: " << request.uri << std::endl;
					if (request.uri == "/5") sleep(5);
					if (methods(request, fd)){
							epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
							close(fd);
							continue;
					}
					http_respond_html(fd,"200 OK", "text/html", file_html);
					epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
					::close(fd);
					if (request.uri == "/shutdown") {
						std::cout << "\x1b\x5b\x48\x1b\x5b\x32\x4a\x1b\x5b\x33\x4a" << std::endl;
						break ;
					}
				} else {
					// std::cout << "No bytes are there to read" << std::endl;
				}		
			}
		}
	}
	epoll_ctl(epfd, EPOLL_CTL_DEL, server_fd, nullptr);
	close(server_fd);

	return 0;
}
