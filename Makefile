CC=gcc
CFLAGS=-g -c -O2

all:server client

server:server.o
	$(CC) $< -o $@  -lsqlite3 -lpthread

client:client.o
	$(CC) $< -o $@

%*.o:%*.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY:clean

clean:
	$(RM) a.out server client *.o
