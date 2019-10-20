CC=gcc
#CFLAGS=-g -O3 -Wall -Wextra -DBUF_SIZE=1048576 -fsanitize=address -static-libasan
CFLAGS=-O3 -Wall -Wextra -DBUF_SIZE=1048576
SRC_DIR=src

all: server.o client.o crc_iscsi_v_pcl.o
	$(CC) $(CFLAGS) client.o crc_iscsi_v_pcl.o -o client.out
	$(CC) $(CFLAGS) server.o crc_iscsi_v_pcl.o -o server.out

server.o: $(SRC_DIR)/server.c
	$(CC) $(CFLAGS) -c $(SRC_DIR)/server.c -o server.o

client.o: $(SRC_DIR)/client.c
	$(CC) $(CFLAGS) -c $(SRC_DIR)/client.c -o client.o

clean:
	rm -f *.o *.out

crc_iscsi_v_pcl.o: $(SRC_DIR)/crc32c/crc_iscsi_v_pcl.asm
	yasm -f x64 -f elf64 -X gnu -g dwarf2 -D LINUX -o crc_iscsi_v_pcl.o $(SRC_DIR)/crc32c/crc_iscsi_v_pcl.asm
