#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

#ifndef BUF_SIZE
#warning "BUF_SIZE undefined"
#define BUF_SIZE (1048576)
#endif
extern unsigned int crc_pcl(unsigned char *buffer, int len,
			    unsigned int crc_init);

static inline uint64_t ts_to_ns(struct timespec* ts) {
	return ts->tv_sec * (uint64_t)1000000000L + ts->tv_nsec;
}

int main(int argc, char *argv[])
{
	int client_socket;
	int tmp;
	unsigned int crc = 0;

	struct sockaddr_in server_addr;

	struct timespec start, end;
	uint64_t trans_time = 0;
	ssize_t send_size, ret;
	ssize_t total_size = 0;
	ssize_t i;

	unsigned char *buf;

	if (argc < 3) {
		printf("usage: %s [SERVER_IP] [PORT]\n", argv[0]);
		return 0;
	}

	client_socket = socket(PF_INET, SOCK_STREAM, 0);
	if (-1 == client_socket) {
		perror("socket()");
		exit(1);
	}

	send_size = BUF_SIZE * 1024;
	/*
	   send_size = atoll(argv[3]);
	   if (send_size / BUF_SIZE * BUF_SIZE != send_size) {
	   send_size = send_size / BUF_SIZE * BUF_SIZE;
	   printf("Rounded SEND_SIZE to %ld\n", send_size);
	   }
	 */

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons((unsigned short)atoi(argv[2]));

	int opt = 1;
	setsockopt(client_socket, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));

	if (-1 ==
	    connect(client_socket, (struct sockaddr *)&server_addr,
		    sizeof(server_addr))) {
		perror("connect()");
		exit(1);
	}

	buf = (unsigned char *)malloc(BUF_SIZE);
	if (buf == NULL) {
		printf("Failed to allocate memory!\n");
		exit(1);
	}

	srand(time(NULL));
	for (;;) {
		for (i = 0; i < (ssize_t) (BUF_SIZE / sizeof(int)); i++) {
			tmp = rand();
			memcpy(buf + (i * sizeof(int)), (void *)&tmp, sizeof(int));
		}
		crc = crc_pcl(buf, BUF_SIZE, crc);

		clock_gettime(CLOCK_MONOTONIC_RAW, &start);
		ret = write(client_socket, buf, BUF_SIZE);
		clock_gettime(CLOCK_MONOTONIC_RAW, &end);
		trans_time += (ts_to_ns(&end) - ts_to_ns(&start));

		if (ret == -1)
			break;

		total_size += ret;
		if (total_size == send_size)
			break;
	}

	printf("Transmission time: %ld.%.9ld\n", trans_time / 1000000000L, trans_time % 1000000000L);
	printf("buf CRC: 0x%08x\n", crc);

	close(client_socket);
	free(buf);

	return 0;
}
