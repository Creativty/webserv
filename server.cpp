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
#include <sys/stat.h>
#include <fcntl.h>
#include <ctime>
#include <sstream>
#include <limits.h>



/* TODO(XENOBAS):
 * - "HTTP/1.0 501 Not Implemented"
 * - "HTTP/1.0 505 HTTP Version Not Supported"
 */

void	http_respond_html(int fd, std::string status, std::string type, const std::string &data) {
	std::string	response_line = "HTTP/1.0 " + status + "\r\n";
	std::string	headers = "Content-Type: " + type + "\r\n"
		"\r\n";
	::send(fd, response_line.c_str(), response_line.size(), MSG_DONTWAIT);
	::send(fd, headers.c_str(), headers.size(), MSG_DONTWAIT);
	if (data.size())
		::send(fd, data.c_str(), data.size(), MSG_DONTWAIT);
}

std::string getFileExtension(const std::string& type) {
    if (type == " text/html")                	return ".html";
    if (type == " text/css")                 	return ".css";
    if (type == " application/javascript")   	return ".js";
    if (type == " application/json")         	return ".json";
    if (type == " application/xml")          	return ".xml";
    if (type == " application/pdf")          	return ".pdf";
 
    // Images	 
    if (type == " image/png")                	return ".png";
    if (type == " image/jpeg")               	return ".jpg";
    if (type == " image/gif")                	return ".gif";
    if (type == " image/bmp")                	return ".bmp";
    if (type == " image/x-icon")             	return ".ico";
    if (type == " image/svg+xml")            	return ".svg";
    if (type == " image/webp")               	return ".webp";
 
    // Audio	 
    if (type == " audio/mpeg")               	return ".mp3";
    if (type == " audio/wav")                	return ".wav";
    if (type == " audio/ogg")                	return ".ogg";
 
    // Video	 
    if (type == " video/mp4")                	return ".mp4";
    if (type == " video/webm")               	return ".webm";
    if (type == " video/x-msvideo")          	return ".avi";
    if (type == " video/quicktime")          	return ".mov";
    if (type == " video/x-matroska")         	return ".mkv";
 
    // Fonts	 
    if (type == " font/ttf")                 	return ".ttf";
    if (type == " font/otf")                 	return ".otf";
    if (type == " font/woff")                	return ".woff";
    if (type == " font/woff2")               	return ".woff2";
 
    // Archives	 
    if (type == " application/zip")          	return ".zip";
    if (type == " application/vnd.rar")      	return ".rar";
    if (type == " application/x-7z-compressed")	return ".7z";
    if (type == " application/x-tar")        	return ".tar";
    if (type == " application/gzip")         	return ".gz";
 
    // Scripts /  Code	
    if (type == " application/x-httpd-php")  	return ".php";
    if (type == " text/x-python")            	return ".py";
    if (type == " text/x-c++src")            	return ".cpp";
    if (type == " text/x-csrc")              	return ".c";
    if (type == " application/x-sh")         	return ".sh";
    return ""; 
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

bool dirExists(std::string dir){
	struct stat st;
	if (stat(dir.c_str(), &st) == -1){
		if (mkdir(dir.c_str(), 0755) == -1){
			std::cerr << "mkdir failed: " << strerror(errno) << std::endl;
			return false;
		}
	}
	return true;
}

std::string generateRandomFileName(std::string ext){
    static bool seeded = false;
    if (!seeded) {
        std::srand(static_cast<unsigned int>(std::time(0)));
        seeded = true;
    }

    int r = std::rand();
    std::ostringstream oss;
    oss << "upload_" << r << ext;
    return oss.str();
}

int methods(http::Request request, int new_socket, struct server server){
    std::string path;
    bool file_ok;

    if (request.uri.find("/favicon.ico") == 0) return(1);
    if (request.method == http::HTTP_METHOD_POST){
        path = url_decode(request.body.substr(5));
		std::string dir = server.config.upload_dir;

		//TODO: remove space from content-type in getFileExtension
		std::string ext = getFileExtension(request.headers["content-type"]);

		if (dirExists(dir)){
			std::cout << "directory of uploads exists \n";

			char oldDir[PATH_MAX];
			getcwd(oldDir, sizeof(oldDir));
			std::cout << "oldDir: " << oldDir << std::endl;

			if (chdir(dir.c_str()) == -1)
				perror("chdir");
			else{

				std::string fileName = generateRandomFileName(ext);
				int fd = open(fileName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
				
				if (!fd)
					perror("open file");
				
				std::cout << "file name: " << fileName << std::endl;
				write(fd, request.body.c_str(), request.body.length());
				close(fd);

				chdir(oldDir);
			} 
		}
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


bool	is_first_connection(int fd, struct server *servers){
	for (int i = 0; i < 3; i++)
		if (fd == servers[i].fd) return 1;
	return 0;
}


struct server get_server(struct server *servers, int port){
	int i = 0;
	while (servers[i].config.port != port) i++;
	return (servers[i]);
}

int handle_requests(struct server *servers, int epfd, sockaddr_in address, size_t addrlen){
	while (true) {
		struct epoll_event events[100];
		int nfds = epoll_wait(epfd, events, 100, -1);
		if (nfds == -1) {
			perror("epoll_wait");
			continue;
		}

		bool				file_html_ok;
		const std::string	file_html = read_entire_file("pages/index.html", &file_html_ok);
		assert(file_html_ok && "could not load template html file");

		for (int i = 0; i < nfds; i++){
			int fd = events[i].data.fd;

			if (is_first_connection(fd, servers)){
				int new_socket = accept(fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
				std::cout << "new_socket " << new_socket << std::endl;
				if (new_socket < 0) {
					perror("In accept");
					return EXIT_FAILURE;
				}
				// fcntl(new_socket, F_SETFL, O_NONBLOCK);

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
				ssize_t valread = recv(fd, buffer, sizeof(buffer), MSG_DONTWAIT);

				// int new_socket = accept(...);

				sockaddr_in local_addr;
				socklen_t len = sizeof(local_addr);
				getsockname(fd, (sockaddr*)&local_addr, &len);
				int port = ntohs(local_addr.sin_port);

				std::cout << "Client came through port " << port << std::endl;

				

				if (valread > 0) {
					std::string	message(buffer, cast(size_t)valread);
					http::Request request;
					http::Parse_Error error = http::parse_request(message, request);
					if (error != http::PARSE_ERROR_NONE) {
						std::cout << "Could not parse request because of " << error << std::endl;
						//TODO
					}

					if (methods(request, fd, get_server(servers, port))){
							epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
							close(fd);
							continue;
					}
					http_respond_html(fd,"200 OK", "text/html", file_html);
					epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
					::close(fd);
				} else {
					std::cout << "No bytes are there to read" << std::endl;
				}		
			}
		}
	}
	return (1);
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

int	setup_servers(struct server *server, int num_server, toml::Config *config){
	for (int i = 0; i < num_server; i++){
		server[i].fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
		server[i].config = config[i];
		struct sockaddr_in address;
		size_t addrlen = sizeof(address);
		
		if (server[i].fd < 0) {
			std::cerr << "Socket creation failed" << std::endl;
			return 2;
		}
		{
			const int reuseaddr = 1;
			setsockopt(server[i].fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int));
		}
		std::memset((char*)&address, 0, addrlen);
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = htonl(INADDR_ANY);
		address.sin_port = htons(server[i].config.port);
		
		if (bind(server[i].fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
			perror("bind failed");
			return 2;
		}
		
		if (listen(server[i].fd, SOMAXCONN) < 0) {
			perror("In listen");
			return EXIT_FAILURE;
		}
	}
	return(1);
}


void setup_config(toml::Config *config){
	config[0].port = 8080;
	config[0].host = "127.0.0.1";
	config[0].upload_dir = "upload";

	config[1].port = 8081;
	config[1].host = "127.0.0.1";
	config[1].upload_dir = "upload1";

	config[2].port = 8082;
	config[2].host = "127.0.0.1";
	config[2].upload_dir = "upload2";
}

int server() {
	int	n_ser = 3;
	struct server servers[n_ser];
	int epfd = epoll_create1(0);
	//TODO: check is epoll failed to create


	//TODO: for test
	struct toml::Config config[n_ser];
	setup_config(config);


	setup_servers(servers, n_ser, config);


	//TODO: remove this
	struct sockaddr_in address;
	size_t addrlen = sizeof(address);
	

	for(int i = 0; i < 3; i++){
		struct epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = servers[i].fd;
		epoll_ctl(epfd, EPOLL_CTL_ADD, servers[i].fd, &ev);
	}

	handle_requests(servers, epfd, address, addrlen);

	close(servers[0].fd);

	return 0;
}
