#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>

int verbose;
int listenfd, conn, clientlen;
struct sockaddr_in client;

void start(char* port) {
    int b_ReuseAddr = 1; // Specify to reuse socket address
    int b_bindSuccess, b_listenSuccess;
    
    // create the socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &b_ReuseAddr, sizeof(b_ReuseAddr));
    if(verbose) {
        if(listenfd <= 0) {
            perror("[P] Error creating socket...\n");
            exit(EXIT_FAILURE);
        } else {
            printf("[P] socket created\n");
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
            perror("[P] bind failed!\n");
            exit(EXIT_FAILURE);
        } else {
            printf("[P] bind success\n");
        }
    }
    
    // listen
    b_listenSuccess = listen(listenfd, 1);
    if(verbose) {
        if(b_listenSuccess < 0) {
            perror("[P] listen failed!\n");
            exit(EXIT_FAILURE);
        } else {
            printf("[P] listen success\n");
        }
    }
}

int main(int argc, char** argv) {
    char* port = argv[1];
    char client_request[200];
    char server_reply[100];
    const char* hello = "hello\r\n";
    
    if(argc <= 1){
        printf("Usage: %s <port> [-v]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    if(argv[2] && strcmp(argv[2], "-v") == 0)
        verbose = 1;
    else
        verbose = 0;
    
    // SETUP THE CONNECTION
    start(port);
    
    // accepting incoming connections
    while (1) {
        printf("[P] Waiting for incoming connection...\n");
        
        clientlen = sizeof(struct sockaddr_in);
        conn = accept(listenfd, (struct sockaddr*) &client, (socklen_t*) &clientlen);
        
        if(conn < 0 ) {
            perror("[P] accept failed\n");
            exit(EXIT_FAILURE);
        } else {
            printf("[P] Connection accepted\n");
        }
        
        // Child handles requests/replies
        if(fork() == 0) {
            close(listenfd); // close parent listen socket
            
            printf("[C] Waiting for incoming request...\n");
            
            // Clean variables from previous requests/replies
            memset(client_request, '\0', sizeof(client_request));
            memset(server_reply, '\0', sizeof(server_reply));
            
            // Receive a request
            if(recv(conn, client_request, 200, 0) < 0) {
                printf("[C] recv failed!\n");
                break;
            } else {
                printf("[C] Client request: %s\n", client_request);
            }
            
            // Determine what to send
            if(strcmp(hello, client_request) == 0) {
                strcpy(server_reply, "hi!\n");
            } else {
                strcpy(server_reply, "what?\n");
            }

            // Send a reply
            if(send(conn, server_reply, strlen(server_reply), 0) < 0) {
                printf("[C] Send failed!\n");
                exit(EXIT_FAILURE);
            } else {
                printf("[C] Server reply: %s\n", server_reply);
            }
            
            // close child connection with client and exit successfully
            printf("[C] Closing connection...\n");
            close(conn);
            exit(EXIT_SUCCESS);
        }
        
        close(conn);
        sleep(1);
    }
    
    return (EXIT_SUCCESS);
}

