#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <pthread.h>

void* thread(void* argp);
void request_handler(int conn);

int verbose;
int listenfd, *conn, clientlen;
struct sockaddr_in client;
char* client_addr;
int client_port;
pthread_t tid;

void create_connection(char* port) {
    int b_ReuseAddr = 1; // Specify to reuse socket address
    int b_bindSuccess, b_listenSuccess;
    
    // create the socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &b_ReuseAddr, sizeof(b_ReuseAddr));
    if(verbose) {
        if(listenfd <= 0) {
            perror("Error creating socket...\n");
            exit(EXIT_FAILURE);
        } else {
            printf("socket created\n");
        }
    }
    
     //Bind
    client.sin_family = AF_INET; /* Internet address family */
    client.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    client.sin_port = htons(atoi(port)); /* Local port */
    b_bindSuccess = bind(listenfd, (struct sockaddr *) &client, sizeof(client));
    if(verbose) {
        if(b_bindSuccess < 0)
        {
            perror("bind failed!\n");
            exit(EXIT_FAILURE);
        } else {
            printf("bind success\n");
        }
    }
    
    // listen
    b_listenSuccess = listen(listenfd, 1);
    if(verbose) {
        if(b_listenSuccess < 0) {
            perror("listen failed!\n");
            exit(EXIT_FAILURE);
        } else {
            printf("listen success\n");
        }
    }
    
    // accept incoming connections
    while (1) {
        printf("Waiting for incoming connection...\n");
        
        clientlen = sizeof(struct sockaddr_in);
        conn = malloc(sizeof(int));
        *conn = accept(listenfd, (struct sockaddr*) &client, (socklen_t*) &clientlen);
        client_addr = inet_ntoa(client.sin_addr);
        client_port = (int) ntohs(client.sin_port);
        pthread_create(&tid, NULL, thread, conn);
        
        if(conn < 0 ) {
            perror("accept failed\n");
            exit(EXIT_FAILURE);
        } else {
            printf("Connected to %s:%i\n", client_addr, client_port);
        }
        
        sleep(1);
    }
}

int main(int argc, char** argv) {
    char* port = argv[1];
    
    if(argc <= 1){
        printf("Usage: %s <port> [-v]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    if(argv[2] && strcmp(argv[2], "-v") == 0)
        verbose = 1;
    else
        verbose = 0;
    
    // Create connection which will run until
    // the last client disconnects or the server quits
    create_connection(port);
    
    return (EXIT_SUCCESS);
}

void* thread(void *vargp) {
    int conn = *((int*) vargp);
    pthread_detach(pthread_self());
    free(vargp);
    request_handler(conn);
    close(conn);    
    return NULL;
}

void request_handler(int conn) {
    //const char* hello = "hello\r\n";
    char client_request[200];
    char server_reply[100];
    
    while(recv(conn, client_request, 200, 0) != 0) {
        // Do the things
        printf("request from %s:%i: %s", client_addr, client_port, client_request);
        
        if(strcmp("HELO\r\n", client_request) == 0) {
            strcpy(server_reply, "> hello!\r\n");
            printf("reply to %s:%i: %s", client_addr, client_port, server_reply);
        } else if(strcmp("QUIT\r\n", client_request) == 0) {
            strcpy(server_reply, "> goodbye!\r\n");
            printf("client %s:%i is disconnecting\n", client_addr, client_port);
            return;
        }
        
        send(conn, server_reply, strlen(server_reply), 0);
        
        // Clear variables for next iteration
        memset(client_request, '\0', sizeof(client_request));
        memset(server_reply, '\0', sizeof(server_reply));
    }
}