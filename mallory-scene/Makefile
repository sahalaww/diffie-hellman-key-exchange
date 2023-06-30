CC = gcc
CFLAGS = -Wall -Wextra -D_GNU_SOURCE

all: server client mallory

server: server.c
	$(CC) $(CFLAGS) -o server server.c

client: client.c
	$(CC) $(CFLAGS) -o client client.c

mallory: mallory.c
	$(CC) $(CFLAGS) -o mallory mallory.c

clean:
	rm -f server client mallory
