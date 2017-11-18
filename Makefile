CC = gcc
CFLAGS = -O -Wall 
LFLAGS = 

all: client server
	
server: server.c
	$(CC) $(CFLAGS) -c -o server.o server.c
	$(CC) $(LFLAGS) -o server server.o
	
client: client.c
	$(CC) $(CFLAGS) -c -o client.o client.c
	$(CC) $(LFLAGS) -o client client.o

clean:
	rm -f server.o server client.o client