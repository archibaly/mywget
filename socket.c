#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>

#include "socket.h"
#include "signal.h"

/**
 * get_ip - get ip by hostname
 * @hostname: in
 * @ip: out
 */
int get_ip(const char *hostname, char *ip)
{
	struct hostent *ht;
	ht = gethostbyname(hostname);
	
	if (ht == NULL)
		return -1;

	if (inet_ntop(AF_INET, ht->h_addr_list[0], ip, INET_ADDRSTRLEN) == NULL)
		return -1;

	return 0;
}

/**
 * socket_connect - connect to tcp server
 * @host: hostname or ip of the server
 */
int socket_connect(const char *host, host_type_t type, uint16_t port)
{
	char ip[INET_ADDRSTRLEN];
	int sockfd;
	struct sockaddr_in server_addr;

	memset(ip, 0, INET_ADDRSTRLEN);
	if (type == HOST_IPV4) {
		memcpy(ip, host, strlen(host));
	} else if (type == HOST_DOMAIN) {
		if (get_ip(host, ip) < 0) {
			return -1;
		}
	} else {
		return -1;
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1;

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	if (inet_pton(AF_INET, ip, &server_addr.sin_addr) < 0) {
		return -1;
	}

	if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
		return -1;

	return sockfd;
}

int tcp_server_init(uint16_t port)
{
	int fd;
	struct sockaddr_in server_addr;
	
	signal(SIGPIPE, SIG_IGN);

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return -1;
	}

	int reuse = 1;
	if ((setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))) < 0) {
		goto errout;
	}

	if (bind(fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0) {
		goto errout;
	}

	if (listen(fd, 5) < 0)
		goto errout;

	return fd;

errout:
	close(fd);
	return -1;
}

int socket_recv(int sockfd, void *buff, int size)
{
	int n;
	
	for (;;) {
		if ((n = recv(sockfd, buff, size, 0)) < 0) {
			if (errno == EINTR)
				continue;
			else
				return -1;
		} else {
			return n;
		}
	}
}

int socket_send(int sockfd, const void *buff, int size)
{
	int nleft;
	int nsent;
	const char *ptr;

	ptr = buff;
	nleft = size;

	while (nleft > 0) {
		if ((nsent = send(sockfd, ptr, nleft, 0)) <= 0) {
			if (nsent < 0 && errno == EINTR)
				nsent = 0;		/* and call write() again */
			else
				return -1;		/* error */
		}

		nleft -= nsent;
		ptr   += nsent;
	}
	return size;
}
