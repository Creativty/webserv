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
#include <map>
#include <sys/epoll.h>

/* TODO(XENOBAS):
 * - HTTP_Writer interface for safely constructing responses.
 * - Event loop of the server is currently in chaos.
 */

static std::map<int, int>			cgi_map;
static std::map<int, std::string>	htmlStatus;

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

enum HttpStatus {
    OK = 200,
    CREATED = 201,
    BAD_REQUEST = 400,
    UNAUTHORIZED = 401,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    SERVER_ERROR = 500,
	NO_CONTENT = 204,
	METHOD_NOT_ALLOWED = 405,
	MOVED_PERMANETLY = 301,
	PAYLOAD_TOO_LARGE = 413,
	NOT_IMPLEMENTED = 501,
	PROCESSING = 102
};

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

	// std::cout << "client_fd in http_respond: " << fd << std::endl;
	::send(fd, response_line.c_str(), response_line.size(), MSG_DONTWAIT);
	::send(fd, headers.c_str(), headers.size(), MSG_DONTWAIT);

	if (data.size())
		::send(fd, data.c_str(), data.size(), MSG_DONTWAIT);
	// std::cout << response_line << headers << data << std::endl;
}

std::string getFileExtension(const std::string& type) {
    if (type == "text/html")                	return ".html";
    if (type == "text/css")                 	return ".css";
    if (type == "application/javascript")   	return ".js";
    if (type == "application/json")         	return ".json";
    if (type == "application/xml")          	return ".xml";
    if (type == "application/pdf")          	return ".pdf";
 
    // Images	 
    if (type == "image/png")                	return ".png";
    if (type == "image/jpeg")               	return ".jpg";
    if (type == "image/gif")                	return ".gif";
    if (type == "image/bmp")                	return ".bmp";
    if (type == "image/x-icon")             	return ".ico";
    if (type == "image/svg+xml")            	return ".svg";
    if (type == "image/webp")               	return ".webp";
 
    // Audio	 
    if (type == "audio/mpeg")               	return ".mp3";
    if (type == "audio/wav")                	return ".wav";
    if (type == "audio/ogg")                	return ".ogg";
 
    // Video	 
    if (type == "video/mp4")                	return ".mp4";
    if (type == "video/webm")               	return ".webm";
    if (type == "video/x-msvideo")          	return ".avi";
    if (type == "video/quicktime")          	return ".mov";
    if (type == "video/x-matroska")         	return ".mkv";
    
	// Fonts	 
    if (type == "font/ttf")                 	return ".ttf";
    if (type == "font/otf")                 	return ".otf";
    if (type == "font/woff")                	return ".woff";
    if (type == "font/woff2")               	return ".woff2";
 
    // Archives	 
    if (type == "application/zip")          	return ".zip";
    if (type == "application/vnd.rar")      	return ".rar";
    if (type == "application/x-7z-compressed")	return ".7z";
    if (type == "application/x-tar")        	return ".tar";
    if (type == "application/gzip")         	return ".gz";
 
    // Scripts /  Code	
    if (type == "application/x-httpd-php")  	return ".php";
    if (type == "text/x-python")            	return ".py";
    if (type == "text/x-c++src")            	return ".cpp";
    if (type == "text/x-csrc")              	return ".c";
    if (type == "application/x-sh")         	return ".sh";
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

/* TODO(xenobas): Slotted for removal */
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

/* TODO(xenobas): Slotted for removal */
bool dirExists(std::string dir){
	struct stat st;
	if (stat(dir.c_str(), &st) == -1){
		if (mkdir(dir.c_str(), 0755) == -1){
			std::cerr << "mkdir failed: "<< strerror(errno) << std::endl;
			return false;
		}
	}
	return true;
}

/* TODO(xenobas): Slotted for removal */
std::string generateRandomFileName(std::string ext){
    static bool seeded = false;
    if (!seeded) {
        std::srand(static_cast<unsigned int>(std::time(0)));
        seeded = true;
    }

    int r = std::rand();
    std::ostringstream oss;
    oss << "upload_"<< r << ext;
    return oss.str();
}

b32		PrepareEnvironment(const WEBSERV_Instance& instance, const HTTP_Request& request){
	i32					ret_env_gateway = ::setenv("GATEWAY_INTERACE", "CGI/1.1", 1);
	if (ret_env_gateway == -1) {
		CLI_show_error_runtime("Could not set env variable \"GATEWAY_INTERFACE\"");
		CLI_show_extra("Reason", "%m");

		return (0);
	}

	/* TODO(xenobas): PATH_INFO: Unimplemented */

	/* NOTE(xenobas): PATH_TRANSLATED: There are no guarantees that PATH_INFOs are file system related, so it is left as empty */
	/* NOTE(xenobas): REMOTE_ADDR: Not required thus ignored */
	/* NOTE(xenobas): REMOTE_HOST: Not required thus ignored */
	/* NOTE(xenobas): REMOTE_IDENT: Not required thus ignored */
	/* NOTE(xenobas): REMOTE_USER: Not required thus ignored */

	/* */
	char				cstr_value[8 * 1024] = { 0 };

	const string_view&	str_uri = request.uri.str;
	string_view			str_query = "";
	i32					idx_query = str_uri.index("?");
	if (idx_query >= 0) {
		str_query = str_uri.slice(idx_query + 1);
	}
	MEM_copy((byte*)str_uri.text, str_uri.len, (byte*)cstr_value, 8 * 1024);
	cstr_value[str_uri.len] = '\0';

	i32					ret_env_query = ::setenv("QUERY_STRING", cstr_value, 1);
	if (ret_env_query == -1) {
		CLI_show_error_runtime("Could not set env variable \"QUERY_STRING\"");
		CLI_show_extra("Reason", "%m");

		return (0);
	}

	const char*	cstr_method		= WEBSERV_method_cstr(request.method);
	i32			ret_env_method	= ::setenv("REQUEST_METHOD", cstr_method, 1);
	if (ret_env_method == -1) {
		CLI_show_error_runtime("Could not set env variable \"REQUEST_METHOD\"");
		CLI_show_extra("Reason", "%m");

		return (0);
	}

	/* TODO(xenobas): SCRIPT_NAME: Unimplemented */

	string_view	str_name		= instance.host;
	if (str_name) { /* SERVER_NAME */
		MEM_copy((byte*)str_name.text, str_name.len, (byte*)cstr_value, 8 * 1024);
		cstr_value[str_name.len] = '\0';

		i32			ret_env_name	= ::setenv("SERVER_NAME", cstr_value, 1);
		if (ret_env_name == -1) {
			CLI_show_error_runtime("Could not set env variable \"SERVER_NAME\"");
			CLI_show_extra("Reason", "%m");

			return (0);
		}
	}

	u16			u16_port		= instance.port;
	cstring_write_u64(cstr_value, 8 * 1024, cast(u64)u16_port);

	i32			ret_env_port	= ::setenv("SERVER_PORT", cstr_value, 1);
	if (ret_env_port == -1) {
		CLI_show_error_runtime("Could not set env variable \"SERVER_PORT\"");
		CLI_show_extra("Reason", "%m");

		return (0);
	}

	string_view	str_protocol		= request.protocol;
	MEM_copy((byte*)str_protocol.text, str_protocol.len, (byte*)cstr_value, 8 * 1024);
	cstr_value[str_protocol.len] = '\0';

	i32			ret_env_protocol	= ::setenv("SERVER_PROTOCOL", cstr_value, 1);
	if (ret_env_protocol == -1) {
		CLI_show_error_runtime("Could not set env variable \"SERVER_PROTOCOL\"");
		CLI_show_extra("Reason", "%m");

		return (0);
	}
	
	if (request.headers.has("Authorization")) {
		const string_view&	str = request.headers.get("Authorization");
		MEM_copy((byte*)str.text, str_uri.len, (byte*)cstr_value, 8 * 1024);
		cstr_value[str.len] = '\0';

		i32					ret = ::setenv("AUTH_TYPE", cstr_value, 1);
		if (ret == -1) {
			CLI_show_error_runtime("Could not set env variable \"AUTH_TYPE\"");
			CLI_show_extra("Reason", "%m");

			return (0);
		}
	}
	if (request.headers.has("Content-Length")) {
		const string_view&	str = request.headers.get("Content-Length");
		MEM_copy((byte*)str.text, str_uri.len, (byte*)cstr_value, 8 * 1024);
		cstr_value[str.len] = '\0';

		i32					ret = ::setenv("CONTENT_LENGTH", cstr_value, 1);
		if (ret == -1) {
			CLI_show_error_runtime("Could not set env variable \"CONTENT_LENGTH\"");
			CLI_show_extra("Reason", "%m");

			return (0);
		}
	}
	if (request.headers.has("Content-Type")) {
		const string_view&	str = request.headers.get("Content-Type");
		MEM_copy((byte*)str.text, str_uri.len, (byte*)cstr_value, 8 * 1024);
		cstr_value[str.len] = '\0';

		i32					ret = ::setenv("CONTENT_TYPE", cstr_value, 1);
		if (ret == -1) {
			CLI_show_error_runtime("Could not set env variable \"CONTENT_TYPE\"");
			CLI_show_extra("Reason", "%m");

			return (0);
		}
	}

	return (1);
}

/* TODO(xenobas): Slotted for removal */
bool	isDirectory(std::string path){
	struct stat s;

	// std::cout << "the path on isDirectory is :"<< path << std::endl;
	if( stat(path.c_str(), &s) == 0 ){
		if(s.st_mode & S_IFREG)
			return (false);
		if( s.st_mode & S_IFDIR )
			return (true);
	}
	return false;
}

/* TODO(xenobas): Slotted for removal */
bool fileExists(const std::string& filename) {
    std::ifstream file(filename.c_str());
    return file.good();
}

u8* read_one_chunk(HTTP_Request req){
	HTTP_Chunk chunk = req.chunk;
	byte* body = (byte *) calloc((u64)(chunk.size + 1), sizeof(byte));
	if (!body) return NULL;
	for (int i = 0; i < chunk.size; i++){
		body[i] = req.buff[chunk.index + i];
	}
	return body;
}

u8* GetBody(HTTP_Request req, i64 &l){
	dynamic_array<HTTP_Chunk>	chunks = req.chunks;
	u64							len = 0;
	u64							index = 0;
	
	if (!req.chunked){
		l = req.chunk.size;
		return (read_one_chunk(req));
	}

	for (int i = 0; i < chunks.len; i++)
		len += (u64)chunks[i].size;

	byte* body = (byte *) calloc((len + 1), sizeof(byte));
	if (!body) return NULL;
	for(int i = 0; i < chunks.len; i++){
		// byte* buff = &req.buff[chunks[i].index]; (safer)
		byte* buff = req.buff.data + chunks[i].index;
		for (int j = 0; j < chunks[i].size && index <= len; j++){
			body[index] = buff[j];
			index++;
		}
	}
	l = (i64)len;
	return body;
}

int methods(HTTP_Request request, int new_socket, struct WEBSERV_Instance instance, int epfd){
	const WEBSERV_URI&	uri = request.uri;
	CLI_debug("Processing request at path \"%.*s\"", uri.str.len, uri.str.text);

    std::string	path = url_decode(request.uri.str.to_string());
	// std::cout << "path is :\t"<< path << std::endl;

	string_view		key = WEBSERV_http_route_pick(instance, string_view(path.c_str()));
	WEBSERV_Route	route = instance.routes.get(key);
	if (route.kind == WEBSERV_ROUTE_CGI){
		path = request.uri.str.to_string().substr(1);

		// std::cout << path << std::endl;
		if (!dirExists(path)) return SERVER_ERROR;

		if (!fileExists(path)) return NOT_FOUND;

		std::string typ = getContentType(path);
		std::string ext = getFileExtension(typ);
		std::cout << typ << std::endl << ext << std::endl;
		ext = ext.substr(1);
		char *argv[] = {
			(char*)path.c_str(),
			(char*) route.CGI.interpreters.get(string_view(ext.c_str())).text,
			NULL
		};
		if (strlen(argv[0]) == 0){
			return NOT_IMPLEMENTED;
		}

		PrepareEnvironment(instance, request);

		int inPipe[2], outPipe[2];
		if (pipe(inPipe) == -1 || pipe(outPipe) == -1){
			perror("pipe");
			return SERVER_ERROR;
		}
					
		int pid = fork();
		if (pid < 0) return(perror("fork"), SERVER_ERROR);

		if (pid == 0){
			if (request.method == WEBSERV_METHOD_POST){
				write(inPipe[1], request.buff.data , (size_t) request.buff.len);
			}
			close(inPipe[1]);

			close(inPipe[1]);
			dup2(inPipe[0], STDIN_FILENO);
			close(inPipe[0]);
			
			close(outPipe[0]);
			dup2(outPipe[1], STDOUT_FILENO);
			close(outPipe[1]);

            /* TODO(sennakhl): Check for permission */
			execv(argv[0], argv);
			perror("execv");
			const char* err_msg = "Status: 500 Internal Server Error\r\n"
                                    "Content-Type: text/html\r\n\r\n"
                                    "<html><body><h1>500 Internal Server Error</h1>"
                                    "<p>CGI script execution failed (execve).</p>"
                                    "</body></html>";
                
			write(STDOUT_FILENO, err_msg, strlen(err_msg));
			close(new_socket);
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
			return PROCESSING;
		}
	}
	
    if (request.method == WEBSERV_METHOD_POST){
		std::string dir =  route.Upload.directory.to_string();


		char oldDir[PATH_MAX];
		if (getcwd(oldDir, sizeof(oldDir)) == NULL) 	return (perror("getcwd"), SERVER_ERROR);
		if (chdir(dir.c_str()) == -1)					return (perror("chdir"), SERVER_ERROR);

		i64 len;
		u8* body = GetBody(request, len);

		std::string ext = getFileExtension(request.headers.get("Content-Type").to_string());
		std::string fileName = generateRandomFileName(ext);
		int fd = open(fileName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (fd < 0)
			return (perror("open file for upload"), SERVER_ERROR);
		
		// std::cout << "file name: " << fileName << std::endl;
		write(fd, body, (size_t)len);
		close(fd);

		chdir(oldDir);
		free(body);
		// close(new_socket);
		return CREATED;
    }

    if (request.method == WEBSERV_METHOD_DELETE){
		path = url_decode(request.uri.str.to_string().substr(1));
        if (access(path.c_str(), F_OK)){
            std::cerr << "ERROR: cannot open " << path << std::endl;
			return NOT_FOUND;
        }

		if (remove(path.c_str())){
			perror("remove");
			return FORBIDDEN;
		}
		// http_respond_html(new_socket, 204, "text/pain", "");
		return(NO_CONTENT);
    }

    if (request.method == WEBSERV_METHOD_GET){
		//TODO: substr(1) 
        path = url_decode(request.uri.str.to_string().substr(1));
        string_view res_file;
		b32			file_ok = OS_read_file(string_view(path.c_str()), res_file);
		// std::cout << "path is: " << path<< std::endl;
		// std::cout << "file_ok: " << file_ok<< std::endl;
        if (file_ok){
            assert(file_ok && "could not load template html file");            
            http_respond_html(new_socket, 200, getContentType(path), res_file.to_string());
			close(new_socket);
			return (PROCESSING);
        }
		if (!path.length())
        	return(OK);// for default page
		if (isDirectory(path))
			return (FORBIDDEN);// forbidden (when the user wants to get directory not file
		return (NOT_FOUND);
    }
    return(METHOD_NOT_ALLOWED);
}

bool	is_first_connection(int fd, dynamic_array<WEBSERV_Instance> instances){
	for (int i = 0; i < instances.len; i++)
		if (fd == instances[i].fd) return 1;
	return 0;
}

WEBSERV_Instance& get_server(dynamic_array<WEBSERV_Instance>& instances, u16 port){
	/* TODO(xenobas): This is just wrong... It needs to check the address it came on, and the Host as well, not just port */
	int i = 0;
	while (instances[i].port != port)
		++i;
	return (instances[i]);
}

int handle_requests(dynamic_array<WEBSERV_Instance>& instances, int epfd, struct sockaddr_in address, size_t addrlen){
	std::map<int, HTTP_Request>	fd_request;

	#define ITERATIONS_SECS 600 /* 3 seconds */
	#define ITERATIONS ((ITERATIONS_SECS * 1000) / 41)
	CLI_debug("handle_request with limit of %d seconds", ITERATIONS_SECS);
	for (i32 iteration = 0; iteration < ITERATIONS; ++iteration) {
		struct epoll_event	events[100];
		i32					nfds = epoll_wait(epfd, events, 100, 41);
		if (nfds == -1) {
			perror("epoll_wait");
			continue;
		}

		for (int i = 0; i < nfds; i++){
			int fd = events[i].data.fd;

			if (is_first_connection(fd, instances)){
				int new_socket = accept(fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
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

				CLI_debug("New connection at file descriptor: %d", new_socket);
			}
			else if(cgi_map.count(fd)){
				int client_fd = cgi_map[fd];
				char buffer[1024 * 1024];
				std::memset(buffer, 0, sizeof(buffer));
				ssize_t valread = read(fd, buffer, sizeof(buffer));

				if (valread > 0){
					// std::cout << "client_fd: \n" << client_fd << std::endl;
					// std::cout << "fd: \n" << fd << std::endl;
					// std::cout << "file content: \n" << buffer << std::endl;
					http_respond_html(client_fd, 200, "text/html", buffer);
				}
				else{
					// std::cout << "\nnothing to read\n\n";
					epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
					close(client_fd);
					cgi_map.erase(fd);
					close(fd);
				}
			}
			else if(!cgi_map.count(fd)){
				CLI_debug("Incoming data not inteded for CGI on file descriptor: %d", fd);

				char	buffer[1024 * 1024] = { 0 }; /* NOTE(xenobas): 1MiB is far too much */
				ssize_t valread = recv(fd, buffer, sizeof(buffer), MSG_DONTWAIT);

				sockaddr_in	local_addr;
				socklen_t	len = sizeof(local_addr);
				getsockname(fd, (sockaddr*)&local_addr, &len);

				u16			port = ntohs(local_addr.sin_port);
				if (valread > 0) {
					if (fd_request.find(fd) == fd_request.end()) {
						fd_request[fd] = HTTP_request_make();
					}
					HTTP_Request& request = fd_request[fd];

					if (!HTTP_request_read(request, cast(const byte*)buffer, cast(i32)valread)) {
						epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
						std::cerr << htmlStatus[BAD_REQUEST] << std::endl;
						http_respond_html(fd, BAD_REQUEST, "text/html", htmlStatus[BAD_REQUEST]);
						fd_request.erase(fd);
						close(fd);
						break;
					}
                    if (!HTTP_request_is_closed(request)) {
                        if (request.buff_stage != HTTP_REQUEST_STAGE_BODY) {
                            break ;
                        }
                        HTTP_request_close(request);
                    }

					int status = methods(request, fd, get_server(instances, port), epfd);
					
					// std::cout << "\n\nstatus : " << status << "\n\n";
					epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
					if(status != PROCESSING){
						// std::cout << htmlStatus[status] << std::endl;
						http_respond_html(fd, status, "text/html", htmlStatus[status]);
						fd_request.erase(fd);
						close(fd);	
					}

				} else {
					// std::cout << "No bytes are there to read" << std::endl;
					http_respond_html(fd, BAD_REQUEST, "text/html", htmlStatus[BAD_REQUEST]);
					fd_request.erase(fd);
					close(fd);
				}
			}
		}
	}
	return (1);
}

int	setup_servers(dynamic_array<WEBSERV_Instance> instances, int num_server){
	htmlStatus[200] = GetDefaultPage();
	htmlStatus[404] = "\r\n\r\n<h1>" + statusCode(404) + "</h1>";
	htmlStatus[400] = "\r\n\r\n<h1>" + statusCode(400) + "</h1>";
	htmlStatus[403] = "\r\n\r\n<h1>" + statusCode(403) + "</h1>";
	htmlStatus[500] = "\r\n\r\n<h1>" + statusCode(500) + "</h1>";
	htmlStatus[201] = "\r\n\r\n<h1>" + statusCode(201) + "</h1>";
	htmlStatus[204] = "\r\n\r\n<h1>" + statusCode(204) + "</h1>";
	htmlStatus[405] = "\r\n\r\n<h1>" + statusCode(405) + "</h1>";
	htmlStatus[301] = "\r\n\r\n<h1>" + statusCode(301) + "</h1>";
	htmlStatus[413] = "\r\n\r\n<h1>" + statusCode(413) + "</h1>";
	htmlStatus[501] = "\r\n\r\n<h1>" + statusCode(501) + "</h1>";
	htmlStatus[301] = "\r\n\r\n<h1>" + statusCode(301) + "</h1>";
	htmlStatus[102] = "\r\n\r\n<h1>" + statusCode(102) + "</h1>";

	for (int i = 0; i < num_server; i++){
		instances[i].fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

		struct sockaddr_in address;
		size_t addrlen = sizeof(address);
		
		if (instances[i].fd < 0) {
			std::cerr << "Socket creation failed" << std::endl;
			return 2;
		}
		{
			const int reuseaddr = 1;
			setsockopt(instances[i].fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int));
		}
		std::memset((char*)&address, 0, addrlen);
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = htonl(INADDR_ANY);
		address.sin_port = htons(instances[i].port);
		
		if (bind(instances[i].fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
			perror("bind failed");
			return 2;
		}
		
		if (listen(instances[i].fd, SOMAXCONN) < 0) {
			perror("In listen");
			return EXIT_FAILURE;
		}
	}
	return(1);
}

int server(WEBSERV_Config&	config) {
	i32	epfd = epoll_create(64);
	if (epfd == -1) {
		CLI_show_error_runtime("Could not create epoll handle");
		CLI_show_extra("Reason", "%m");

		return (0);
	}

	dynamic_array<WEBSERV_Instance>&	instances = config.instances;
	setup_servers(instances, instances.len);

	for(int i = 0; i < instances.len; i++){
		struct epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = instances[i].fd;
		epoll_ctl(epfd, EPOLL_CTL_ADD, instances[i].fd, &ev);
	}
	CLI_debug("HTTP Servers have been created");

	struct sockaddr_in address;
	size_t addrlen = sizeof(address);
	handle_requests(instances, epfd, address, addrlen);

	for (int i = 0; i < instances.len; i++)
		close(instances[i].fd);
	close(epfd);

	return 0;
}
