#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#ifndef BUF_SIZE
#warning "BUF_SIZE undefined"
#define BUF_SIZE (1048576)
#endif
extern unsigned int crc_pcl(unsigned char *buffer, int len,
			    unsigned int crc_init);

int main(int argc, char *argv[])
{
	int server_socket;
	int client_socket;
	socklen_t client_addr_size;

	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;

	unsigned int crc = 0;

	ssize_t total_size = BUF_SIZE * 1024;
	ssize_t read_size, ret;

	unsigned char *buf;
	buf = (unsigned char *)malloc(BUF_SIZE);

	if (argc < 2) {
		printf("usage: %s [PORT]\n", argv[0]);
		return 0;
	}

	server_socket = socket(PF_INET, SOCK_STREAM, 0);
	if (-1 == server_socket) {
		perror("socket()");
		exit(1);
	}

	int true = 1;
	if (setsockopt
	    (server_socket, SOL_SOCKET, SO_REUSEADDR, &true,
	     sizeof(int)) == -1) {
		perror("setsockopt()");
		exit(1);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons((unsigned short)atoi(argv[1]));
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (-1 ==
	    bind(server_socket, (struct sockaddr *)&server_addr,
		 sizeof(server_addr))) {
		perror("bind()");
		exit(1);
	}

	if (-1 == listen(server_socket, 5)) {
		perror("listen()");
		exit(1);
	}

	crc = 0;
	read_size = 0;

	client_addr_size = sizeof(client_addr);
	client_socket =
	    accept(server_socket, (struct sockaddr *)&client_addr,
		   &client_addr_size);

	if (-1 == client_socket) {
		perror("accept()");
		exit(1);
	}

	for (;;) {
		ret = read(client_socket, buf, BUF_SIZE);
		if (ret == -1)
			break;

		crc = crc_pcl(buf, ret, crc);

		read_size += ret;
		if (read_size == total_size)
			break;
	}
	free(buf);

	printf("buf CRC: 0x%08x\n", crc);

	close(client_socket);
	close(server_socket);

	shutdown(client_socket, SHUT_RDWR);
	shutdown(server_socket, SHUT_RDWR);

	return 0;
}
