CC = gcc
CFLAGS = -O -Wall
LFLAGS = -pthread

all: users.o server client
	
users.o: users.h users.c
	$(CC) $(CFLAGS) -c -o users.o users.c

server: server.c
	$(CC) $(CFLAGS) -c -o server.o server.c
	$(CC) $(LFLAGS) -o server server.o users.o
	
client: client.c
	$(CC) $(CFLAGS) -c -o client.o client.c
	$(CC) $(LFLAGS) -o client client.o

clean:
	rm -f server.o server client.o client users.o