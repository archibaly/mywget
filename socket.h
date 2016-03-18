#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <stdint.h>

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN		16	/* xxx.xxx.xxx.xxx\0 */
#endif

int get_ip(const char *hostname, char *ip);
int tcp_server_init(uint16_t port);
int socket_connect(const char *host, uint16_t port);
int socket_recv(int sockfd, void *buff, int size);
int socket_send(int sockfd, const void *buff, int size);

#endif	/* _SOCKET_H_ */
