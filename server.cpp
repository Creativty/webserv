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
#include <sys/wait.h>
#include <cstdlib>

/*TODO:
* - ADD QUERY_STRING to environment variables
* - remove space from content-type
* - 
*/

static std::map<int, int> cgi_map;
static std::map<int, std::string> htmlStatus;




/* TODO(XENOBAS):
 * - "HTTP/1.0 501 Not Implemented"
 * - "HTTP/1.0 505 HTTP Version Not Supported"
 */

#include <string>

// This is a C++98-compatible way to declare a large, multi-line string.
// The compiler automatically joins all these pieces into one std::string.
std::string GetDefaultPage()
{
    std::string htmlContent =
        "<!DOCTYPE html>\n"
        "<html lang=\"en\">\n"
        "<head>\n"
        "    <meta charset=\"UTF-8\">\n"
        "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
        "    <title>Welcome to Webserv!</title>\n"
        "    <style>\n"
        "        body {\n"
        "            font-family: Arial, sans-serif;\n"
        "            display: flex;\n"
        "            justify-content: center;\n"
        "            align-items: center;\n"
        "            height: 100vh;\n"
        "            margin: 0;\n"
        "            background-color: #f4f4f4;\n"
        "        }\n"
        "        .container {\n"
        "            text-align: center;\n"
        "            padding: 50px;\n"
        "            border-radius: 10px;\n"
        "            background-color: #ffffff;\n"
        "            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);\n"
        "        }\n"
        "        h1 {\n"
        "            color: #333;\n"
        "        }\n"
        "        p {\n"
        "            color: #555;\n"
        "            font-size: 1.2em;\n"
        "        }\n"
        "    </style>\n"
        "</head>\n"
        "<body>\n"
        "\n"
        "    <div class=\"container\">\n"
        "        <h1>Hello from Webserv!</h1>\n"
        "        <p>This is a default page.</p>\n"
        "    </div>\n"
        "\n"
        "</body>\n"
        "</html>\n";

    return htmlContent;
}



std::string statusCode(int code){
	if(code == 404) return "404 Not Found";
	if(code == 400) return "400 Bad Request";
	if(code == 500) return "500 Internal Server Error";
	if(code == 201) return "201 Created";
	if(code == 204) return "204 No Content";
	if(code == 403) return "403 Forbidden";
	if(code == 405) return "405 Method Not Allowed";
	if(code == 301) return "301 Moved Permanently";
	if(code == 413) return "413 Payload Too Large";
	if(code == 501) return "501 Not Implemented";
	if(code == 102) return "102 Processing";
	return "200 OK";
}

void	http_respond_html(int fd, int code, std::string type, const std::string &data) {
	std::string	response_line = "HTTP/1.0 " + statusCode(code) + "\r\n";
	std::string	headers = "Content-Type: " + type + "\r\n"
		"\r\n";
	std::cout << "client_fd in http_respond: " << fd << std::endl;
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



std::string	getMethod(http::HTTP_Method method){
	if (method == http::HTTP_METHOD_DELETE) return "DELETE";
	if (method == http::HTTP_METHOD_POST) return "POST";
	if (method == http::HTTP_METHOD_GET) return "GET";
	return "";
}



void PrepareEnvirement(http::Request request){
	if(setenv("REQUEST_METHOD", getMethod(request.method).c_str(), 1))
		std::cerr << "Error setting environment variable" << std::endl;

	std::map<std::string, std::string>::const_iterator it;
	for(it = request.headers.begin(); it != request.headers.end(); it++){
		std::string tmp = it->first;
		for (size_t i=0 ; i < tmp.length(); i++){
			if (tmp[i] == '-') tmp[i] = '_';
			tmp[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(tmp[i])));
		}
		std::cout<< tmp << "=" << (it->second).substr(1) << std::endl;
		if(setenv(tmp.c_str(), it->second.c_str(), 1))
			std::cerr << "Error setting environment variable" << std::endl;
    }
}




bool isDirectory(std::string path){
	struct stat s;

	std::cout << "the path on isDirectory is :" << path << std::endl;
	if( stat(path.c_str(), &s) == 0 ){
		if(s.st_mode & S_IFREG)
			return (false);
		if( s.st_mode & S_IFDIR )
			return (true);
	}
	return false;
}



int methods(http::Request request, int new_socket, struct server server, int epfd){
    std::string path = url_decode(request.uri);
    bool file_ok;

    if (request.uri.find("/favicon.ico") == 0) return(204);





	//CGI
	if (path.find(server.config.cgi_path) == 0){
		path = path.substr(1);
		server.config.cgi_path = server.config.cgi_path.substr(1);
		std::cout << path << std::endl;
		if (!dirExists(server.config.cgi_path)) return 500;

		std::string typ = " " + getContentType(path);
		std::string ext = getFileExtension(typ);
		char *argv[] = {
			(char*)server.config.cgi_scripts[ext].c_str(),
			(char*)path.c_str(),
			NULL
		};
		if (strlen(argv[0]) == 0){
			return 501;
		}

		PrepareEnvirement(request);

		int inPipe[2], outPipe[2];
		if (pipe(inPipe) == -1 || pipe(outPipe) == -1){
			perror("pipe");
			return 500;
		}
					
		int pid = fork();
		if (pid < 0) return(perror("fork"), 500);

		if (pid == 0){
			if (request.method == http::HTTP_METHOD_POST){
				write(inPipe[1], request.body.c_str(), request.body.length());
			}
			close(inPipe[1]);

			close(inPipe[1]);
			dup2(inPipe[0], STDIN_FILENO);
			close(inPipe[0]);
			
			close(outPipe[0]);
			dup2(outPipe[1], STDOUT_FILENO);
			close(outPipe[1]);

			execv(argv[0], argv);
			perror("execve");
			const char* err_msg = "Status: 500 Internal Server Error\r\n"
                                    "Content-Type: text/html\r\n\r\n"
                                    "<html><body><h1>500 Internal Server Error</h1>"
                                    "<p>CGI script execution failed (execve).</p>"
                                    "</body></html>";
                
			write(STDOUT_FILENO, err_msg, strlen(err_msg));
            exit(1);
		}
		else {
			close(inPipe[0]);
			close(outPipe[1]);
			close(inPipe[1]);

			struct epoll_event ev;
			ev.events = EPOLLIN | EPOLLET;
			ev.data.fd = outPipe[0];
			epoll_ctl(epfd, EPOLL_CTL_ADD, outPipe[0], &ev);

			cgi_map[outPipe[0]] = new_socket;
			return 102;
		}
	}


	
    if (request.method == http::HTTP_METHOD_POST){
        path = url_decode(request.body.substr(1));
		std::string dir = server.config.upload_dir;

		//TODO: remove space from content-type in getFileExtension
		std::string ext = getFileExtension(request.headers["content-type"]);

		for (std::map<std::string, std::string>::iterator it = request.headers.begin();it != request.headers.end(); ++it) {
        	std::cout << it->first << " => " << it->second << std::endl;
    	}
		// std::cout << request.body<< std::endl;

		if (!dirExists(dir)){
			std::cout << "directory of uploads not exists \n";
			return 500;
		}
		char oldDir[PATH_MAX];
		if (getcwd(oldDir, sizeof(oldDir)) == NULL) return (perror("getcwd"),500);

		std::cout << "oldDir: " << oldDir << std::endl;

		if (chdir(dir.c_str()) == -1)
			return (perror("chdir"), 500);
		else{

			std::string fileName = generateRandomFileName(ext);
			int fd = open(fileName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (fd < 0)
				return (perror("open file for upload"), 500);
			
			std::cout << "file name: " << fileName << std::endl;
			write(fd, request.body.c_str(), request.body.length());
			close(fd);

			chdir(oldDir);
		}
		// close(new_socket);
		return 201;
    }



    if (request.method == http::HTTP_METHOD_DELETE){
		path = url_decode(request.uri.substr(1));
        if (access(path.c_str(), F_OK)){
            std::cout << "ERROR: cannot open " << path << std::endl;
			return 404;
        }

		if (remove(path.c_str())){
			perror("remove");
			return 403;
		}
		// http_respond_html(new_socket, 204, "text/pain", "");
		return(204);
    }


    if (request.method == http::HTTP_METHOD_GET){
		//TODO: substr(1) 
        path = url_decode(request.uri.substr(1));
        std::string res_file = read_entire_file(path, &file_ok);
		std::cout << "path is: " << path<< std::endl;
		std::cout << "file_ok: " << file_ok<< std::endl;
		std::cout << "path: " << path<< std::endl;
        if (file_ok){
            assert(file_ok && "could not load template html file");            
            http_respond_html(new_socket, 200, getContentType(path), res_file);
			close(new_socket);
			return (102);
        }
		if (!path.length())
        	return(200);// for default page
		if (isDirectory(path))
			return (403);// forbidden (when the user wants to get directory not file)
		return (404);
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

		// bool				file_html_ok;
		// const std::string	file_html = read_entire_file("pages/index.html", &file_html_ok);
		// assert(file_html_ok && "could not load template html file");

		for (int i = 0; i < nfds; i++){
			int fd = events[i].data.fd;

			if (is_first_connection(fd, servers)){
				int new_socket = accept(fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
				std::cout << "new_socket " << new_socket << std::endl;
				if (new_socket < 0) {
					perror("In accept");
					return EXIT_FAILURE;
				}

				struct epoll_event client_ev;
				client_ev.events = EPOLLIN;
				client_ev.data.fd = new_socket;
				if (epoll_ctl(epfd, EPOLL_CTL_ADD, new_socket, &client_ev) == -1) {
					perror("epoll_ctl failed");
					close(new_socket);
					continue;
				}

			}
			else if(cgi_map.count(fd)){
				int client_fd = cgi_map[fd];
				char buffer[1024 * 1024];
				std::memset(buffer, 0, sizeof(buffer));
				ssize_t valread = read(fd, buffer, sizeof(buffer));

				if (valread > 0){
					std::cout << "client_fd: \n" << client_fd << std::endl;
					std::cout << "fd: \n" << fd << std::endl;
					std::cout << "file content: \n" << buffer << std::endl;
					http_respond_html(client_fd, 200, "text/html", buffer);
				}
				else{
					std::cout << "\nnothing to read\n\n";
					epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
					close(client_fd);
					cgi_map.erase(fd);
					close(fd);
				}
			}
			else if(!cgi_map.count(fd)){
				char buffer[1024 * 1024];
				std::memset(buffer, 0, sizeof(buffer));
				ssize_t valread = recv(fd, buffer, sizeof(buffer), MSG_DONTWAIT);

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
						http_respond_html(fd,400, "text/html", "");
						close(fd);
						continue;
						// std::cout << "Could not parse request because of " << error << std::endl;
					}

					int status = methods(request, fd, get_server(servers, port), epfd);
					
					std::cout << "\n\nstatus : " << status << "\n\n";

					// if (status != 200){
					epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
					if(status != 102){
						http_respond_html(fd,status, "text/html", htmlStatus[status]);
						close(fd);
					}



					// if (status){
					// 		epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
					// 		if (status != 102)
					// 			close(fd);
					// 		continue;
					// }
					// http_respond_html(fd,200, "text/html", file_html);
					// epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
					// ::close(cgi_map[fd]);
					// ::close(fd);
				} else {
					std::cout << "No bytes are there to read" << std::endl;
					//TODO: send respond
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
	htmlStatus[200] =GetDefaultPage();
	htmlStatus[404] ="\r\n\r\n<h1>" + statusCode(404) + "</h1>";
	htmlStatus[400] ="\r\n\r\n<h1>" + statusCode(400) + "</h1>";
	htmlStatus[403] ="\r\n\r\n<h1>" + statusCode(403) + "</h1>";
	htmlStatus[500] ="\r\n\r\n<h1>" + statusCode(500) + "</h1>";
	htmlStatus[201] ="\r\n\r\n<h1>" + statusCode(201) + "</h1>";
	htmlStatus[204] ="\r\n\r\n<h1>" + statusCode(204) + "</h1>";
	htmlStatus[405] ="\r\n\r\n<h1>" + statusCode(405) + "</h1>";
	htmlStatus[301] ="\r\n\r\n<h1>" + statusCode(301) + "</h1>";
	htmlStatus[413] ="\r\n\r\n<h1>" + statusCode(413) + "</h1>";
	htmlStatus[501] ="\r\n\r\n<h1>" + statusCode(501) + "</h1>";
	htmlStatus[301] ="\r\n\r\n<h1>" + statusCode(301) + "</h1>";
	htmlStatus[102] ="\r\n\r\n<h1>" + statusCode(102) + "</h1>";

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
	config[0].cgi_path = "/CGI/";
	config[0].cgi_scripts[".py"] = "/usr/bin/python3"; 
	config[0].cgi_scripts[".php"] = "/usr/bin/php"; 
	
	config[1].port = 8081;
	config[1].host = "127.0.0.1";
	config[1].upload_dir = "upload1";
	config[1].cgi_path = "/CGI/";
	
	config[2].port = 8082;
	config[2].host = "127.0.0.1";
	config[2].upload_dir = "upload2";
	config[2].cgi_path = "/CGI/";
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

	for (int i = 0; i < n_ser; i++)
		close(servers[i].fd);

	return 0;
}