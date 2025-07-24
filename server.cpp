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
    if (filename.find(".css")  != std::string::npos) return "text/css";
    if (filename.find(".js")   != std::string::npos) return "application/javascript";
    if (filename.find(".png")  != std::string::npos) return "image/png";
    if (filename.find(".jpg")  != std::string::npos) return "image/jpeg";
    if (filename.find(".gif")  != std::string::npos) return "image/gif";
    if (filename.find(".mp4")  != std::string::npos) return "video/mp4";
    if (filename.find(".mp3")  != std::string::npos) return "audio/mp3";
    return "text/plain";
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

int methods(std::string uri, int new_socket){
    std::string path;
    bool file_ok;

    if (uri.find("/favicon.ico") == 0) return(1);
    if (uri.find("/POST?data=") == 0) return(1);//TODO
    if (uri.find("/DELETE?data=") == 0){
        path = url_decode(uri.substr(13));
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
    if (uri.find("/GET?data=") == 0){
        path = url_decode(uri.substr(10));
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

int server() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if (server_fd < 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return 2;
    }
	{
		const int reuseaddr = 1;
		setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int));
	}

    std::memset((char*)&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return 2;
    }

    if (listen(server_fd, 3) < 0) {
        perror("In listen");
        return EXIT_FAILURE;
    }

	bool				file_html_ok;
	const std::string	file_html = read_entire_file("pages/index.html", &file_html_ok);
	assert(file_html_ok && "could not load template html file");

    while (true) {
        std::cout << "\n+++++++ Waiting for new connection ++++++++\n\n";
        int new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("In accept");
            return EXIT_FAILURE;
        }

        char buffer[1024 * 1024];
        std::memset(buffer, 0, sizeof(buffer));
        ssize_t valread = ::read(new_socket, buffer, sizeof(buffer));

        if (valread > 0) {
            // std::cout << buffer << std::endl;
        } else {
            std::cout << "No bytes are there to read" << std::endl;
        }

		std::string	message(buffer, cast(size_t)valread);
		http::Request request;
		http::Parse_Error error = http::parse_request(message, request);
        if (error != http::PARSE_ERROR_NONE) {
			std::cout << "Could not parse request because of " << error << std::endl;
		}

        if (methods(request.uri, new_socket)) continue;

		http_respond_html(new_socket,"200 OK", "text/html", file_html);
        ::close(new_socket);
		if (request.uri == "/shutdown") {
            std::cout << "\x1b\x5b\x48\x1b\x5b\x32\x4a\x1b\x5b\x33\x4a" << std::endl;
			break ;
        }
    }
    close(server_fd);

    return 0;
}
