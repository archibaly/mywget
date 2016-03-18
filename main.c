#include <stdio.h>
#include <stdlib.h>

#include "url.h"
#include "http.h"
#include "socket.h"

#define BUFF_SIZE	4096

int main(int argc, char **argv)
{
	if (argc != 2) {
		printf("usage: %s <url>\n", argv[0]);
		return -1;
	}

	url_field_t *url = url_parse(argv[1]);
	if (url == NULL)
		return -1;

	int sockfd;
	int port = url->port == NULL ? 80 : atoi(url->port);
	if ((sockfd = socket_connect(url->host, port)) < 0) {
		perror("socket_connect()");
		return -1;
	}

	char buff[BUFF_SIZE];
	int n = http_build_get_header(url->host, url->path, buff);
	url_free(url);

	if (socket_send(sockfd, buff, n) < 0) {
		perror("socket_send()");
		return -1;
	}

	/* store to file */
	FILE *fp = fopen("file", "w");
	if (fp == NULL) {
		perror("fopen()");
		return -1;
	}

	/* receive http header first */
	n = socket_recv(sockfd, buff, sizeof(buff));
	if (n < 0) {
		perror("socket_recv()");
		return -1;
	}

	int offset = http_get_body_pos(buff, n);
	fwrite(buff + offset, sizeof(char), n - offset, fp);

	for (;;) {
		n = socket_recv(sockfd, buff, sizeof(buff));
		if (n > 0) {
			fwrite(buff, sizeof(char), n, fp);
		} else if (n == 0) {	/* receive done */
			break;
		} else {
			perror("socket_recv()");
			return -1;
		}
	}
	fclose(fp);

	return 0;
}
