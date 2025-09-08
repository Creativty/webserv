// C++ specific headers
#include <vector>
#include <iostream>

// C specific headers
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h> // NOTE: C++11 is when it was officially added, so we use the C one.
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>

typedef uint8_t		byte;

typedef int8_t		i8;
typedef uint8_t		u8;
typedef int16_t		i16;
typedef uint16_t	u16;
typedef int32_t		i32;
typedef uint32_t	u32;
typedef int64_t		i64;
typedef uint64_t	u64;

typedef	const char*	cstring;

// execve, pipe, strerror, gai_strerror, errno, dup,
// dup2, fork, socketpair, htons, htonl, ntohs, ntohl,
// select, poll, epoll (epoll_create, epoll_ctl, epoll_wait),
// socket, accept, listen, send, recv, chdir, bind, connect,
// getaddrinfo, freeaddrinfo, setsockopt, getsockname,
// getprotobyname, fcntl, close, read, write, waitpid,
// kill, signal, access, stat, open, opendir, readdir
// and closedir

struct bytes {
	byte	*data;
	u32		len;
};

class Timestamp {
public:
	static u64	now(void) {
		return (0);
	}
};

class TCP_Connection {
private:
	int	fd;
	u64	last_timestamp;
public:
	TCP_Connection(int fd): fd(fd), last_timestamp(Timestamp::now()) { };
	~TCP_Connection(void) { };
	TCP_Connection(const TCP_Connection& instance): fd(instance.fd), last_timestamp(instance.last_timestamp) { };
	TCP_Connection&	operator=(const TCP_Connection& instance) {
		if (this != &instance) {
			fd = instance.fd;
			last_timestamp = instance.last_timestamp;
		}
		return (*this);
	};

	void	close(void) {
		if (fd < 0) return ;
		::close(fd);
		fd = -1;
	}
	bool	up(u64 timeout) const {
		if (fd < 0) return (false);
		u64	timestamp = Timestamp::now();
		return (timestamp - last_timestamp <= timeout);
	}
};

class TCP_Tunnel {
private:
	static const int PROTOCOL_IP = 0;
private:
	i32							fd;
	u32							cap;
	u16							port;
	bool						running;
	i32							epfd;
public:
	std::vector<TCP_Connection>	connections;
public:
	TCP_Tunnel(uint16_t port, u32 cap = 10u): fd(-1), cap(cap), port(port), running(false), epfd(-1), connections() { };
	~TCP_Tunnel(void) { };
	TCP_Tunnel(const TCP_Tunnel& instance): fd(instance.fd), cap(instance.cap), port(instance.port), running(instance.running), epfd(instance.epfd), connections(instance.connections) { };
	TCP_Tunnel&	operator=(const TCP_Tunnel& instance) {
		if (this != &instance) {
			fd = instance.fd;
			cap = instance.cap;
			port = instance.port;
			running = instance.running;
			epfd = instance.epfd;
			connections = instance.connections;
		}
		return (*this);
	};

	std::string	strerror(void) {
		return (std::string(::strerror(errno)));
	}

	void		close(void) {
		if (epfd >= 0) ::close(epfd);
		epfd = -1;
		if (fd >= 0) ::close(fd);
		fd = -1;
	}
	bool		open(void) {
		if (running) return (false);

		fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, PROTOCOL_IP);
		if (fd < 0) return (false);

		struct sockaddr_in	address;
		address.sin_port = ::htons(port); // host to network short
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;

		if (::bind(fd, (struct sockaddr*)&address, sizeof(address)) < 0) return (close(), false);
		if (::listen(fd, (i32)cap) < 0) return (close(), false);

		epfd = ::epoll_create(32);
		if (epfd < 0) return (close(), false);
		else return (running = true);
	}
	bool		step_accept(void) { // NOTE(xenobas): Accept connection
		struct sockaddr_in	client_address;
		socklen_t			client_address_len = sizeof(client_address);
		int					client_fd = ::accept(fd, (struct sockaddr*)(&client_address), &client_address_len);
		if (client_fd < 0)
			return (errno == EAGAIN || errno == EWOULDBLOCK);
		std::cout << "client_fd :: " << client_fd << std::endl;
		return (true);
	}
	void		step(void) {
		if (fd < 0 || !running) return ;
		step_accept();
	}

private:
	void		connection_register(const TCP_Connection& conn) {
		connections.push_back(conn);
	}
	u64			connection_count(void) const {
		return (connections.size());
	}
};

int	main(const int argc, cstring *argv) {
	TCP_Tunnel	tunnel(8080);

	(void)argc;
	(void)argv;
	if (!tunnel.open()) {
		std::cerr << tunnel.strerror() << std::endl;
		return (1);
	}
	tunnel.step();
	tunnel.close();
	return (0);
}
