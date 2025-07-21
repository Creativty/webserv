#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <cstdlib>
#include <stdio.h>
#include "webserv.hpp"


#define PORT 8080

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
		if (error == http::PARSE_ERROR_NONE) {
			std::cout << "Version: " << request.version << '\n';
			std::cout << "Method:  " << request.method << '\n';
			std::cout << "URI:     " << request.uri << '\n';
			std::cout << "Body:    " << request.body << '\n';
			std::cout << "Headers: " << '\n';
			for (std::map<std::string, std::string>::iterator iter = request.headers.begin(); iter != request.headers.end(); iter++) {
				std::string	name = iter->first, value = iter->second;
				std::cout << '\t' << name << ":" << value << std::endl;
			}
		} else {
			std::cout << "Could not parse request because of " << error << std::endl;
			// std::cout << buffer << std::endl;
		}


        std::string http_response =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n\r\n"
    "<!DOCTYPE html>"
    "<html lang=\"en\">"
    "<head>"
    "<meta charset=\"UTF-8\">"
    "<style>"
    "  body {"
    "    background-color: #121212;"
    "    color: #ffffff;"
    "    font-family: Arial, sans-serif;"
    "  }"
    "  a, button {"
    "    color: #ffffff;"
    "    background-color: #333333;"
    "    padding: 8px 12px;"
    "    border: none;"
    "    border-radius: 5px;"
    "    text-decoration: none;"
    "    display: inline-block;"
    "    margin: 5px;"
    "  }"
    "  input, textarea {"
    "    background-color: #1e1e1e;"
    "    color: #ffffff;"
    "    border: 1px solid #555;"
    "    padding: 6px;"
    "  }"
    "</style>"

    "</head>"
    "<body>"
    "<a href=\"/1\">Resource 1</a><br>"
    "<a href=\"/2\">Resource 2</a><br>"
    "<a href=\"/3\">Resource 3</a><br>"
    "<form method=\"POST\" action=\"/POST\">"
        "<input type=\"text\" name=\"data\" placeholder=\"Write data here to POST it...\" required />"
        "<button type=\"submit\">POST</button>"
    "</form>"
    "<form method=\"GET\" action=\"/GET\">"
        "<input type=\"text\" name=\"data\" placeholder=\"Write data here to GET it...\" required />"
        "<button type=\"submit\">GET</button>"
    "</form>"
    "<form method=\"DELETE\" action=\"/DELETE\">"
        "<input type=\"text\" name=\"data\" placeholder=\"Write data here to DELETE it...\" required />"
        "<button type=\"submit\">DELETE</button>"
    "</form>"
    "<form method=\"POST\" action=\"/json\">"
        "<input type=\"text\" name=\"data\" placeholder=\"Write data here to POST(json) it...\" required />"
        "<textarea style=\"display: none;\" readonly>"
            "UNIMPLEMENTED IN THE CLIENT SIDE"
        "</textarea>"
        "<button type=\"submit\">POST JSON</button>"
    "</form>"
    "</body>"
    "</html>";


        ::write(new_socket, http_response.c_str(), http_response.size());
        ::close(new_socket);
		if (request.uri == "/shutdown")
			break ;
    }

    return 0;
}
