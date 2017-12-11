#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <pthread.h>

#include "users.h"

void* thread(void* argp);
void request_handler(int conn);
void broadcast_message(char* msg);

int verbose;

Node* userlist;
pthread_mutex_t userlist_mutex;

int listenfd, *conn, clientlen;
struct sockaddr_in client;
char* client_addr;
int client_port;
char nick[20];
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
        
        printf("adding %i to linked list\n", *conn);
        sprintf(nick, "Guest%i", client_port);
        
        pthread_mutex_lock(&userlist_mutex);
        userlist = add_user(userlist, nick, client_addr, client_port, conn);
        pthread_mutex_unlock(&userlist_mutex);
        
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
    userlist = userlist_init();
    pthread_mutex_init(&userlist_mutex, NULL);
            
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
    char client_request[1024];
    char server_reply[1024];
    const char welcome[1024] = "Welcome to cIRC!\n$ ";
    const char prompt[4] = "$ ";
    char* cmd_arg;
    User* user = NULL;
    
    // Send a welcome message
    send(conn, welcome, strlen(welcome), 0);
    
    while(recv(conn, client_request, 200, 0) != 0) {
        // Do the things
        printf("request from %s:%i: %s", client_addr, client_port, client_request);
        
        if(strcmp("HELO\r\n", client_request) == 0) 
        {
            strcpy(server_reply, "> hello!\r\n");
            printf("reply to %s:%i: %s", client_addr, client_port, server_reply);
        } 
        
        if (strstr(client_request, "MESG")) 
        {
            cmd_arg = &client_request[5];
//            strcpy(server_reply, "> ");
//            strcat(server_reply, cmd_arg);
            printf("reply to %s:%i: %s", client_addr, client_port, server_reply);
            broadcast_message(cmd_arg);
        } 
        
        if(strstr(client_request, "NICK"))
        {
            cmd_arg = &client_request[5];
            user = get_by_id(userlist, conn);
            cmd_arg[strlen(cmd_arg) - 1] = 0;    // remove trailing newline
            
            // If the user enters the command with no args, display current neck
            // otherwise, change the user nick to specified arg
            if(client_request[4] == '\r') {
                strcpy(server_reply, "> current nick: ");
                strcat(server_reply, user->nick);
                strcat(server_reply, "\n");
            } else {
                pthread_mutex_lock(&userlist_mutex);
                    change_nick(user, cmd_arg);
                pthread_mutex_unlock(&userlist_mutex);
                
                
                strcpy(server_reply, "> changed nick to: ");
                strcat(server_reply, cmd_arg);
                strcat(server_reply, "\n");
                
                printf("reply to %s:%i: %s", client_addr, client_port, server_reply);
            }
            user = get_by_id(userlist, client_port);
        }
        
        if(strstr(client_request, "LIST"))
        {
            print_user_list(userlist);
            printf("\n");
            
            get_user_list(userlist, server_reply);
        }
        
        if(strcmp("QUIT\r\n", client_request) == 0) 
        {
            strcpy(server_reply, "> goodbye!\r\n");
            send(conn, server_reply, strlen(server_reply), 0);
            printf("client %s:%i is disconnecting\n", client_addr, client_port);
            return;
        }
        
        send(conn, server_reply, strlen(server_reply), 0);
        send(conn, prompt, strlen(prompt), 0);
        
        // Clear variables for next iteration
        memset(client_request, '\0', sizeof(client_request));
        memset(server_reply, '\0', sizeof(server_reply));
        cmd_arg = NULL;
    }
}

void broadcast_message(char* msg) {
    printf("Broadcasting message: %s", msg);
    /* Pseudocode
     * foreach(conn in list) {
     *  send(user->conn, msg, strlen(msg), 0);
     * }
     */
    int i;
    Node* n = userlist;
    User* user = n->data;
    int length = userlist_length(userlist);
    
    strcat(msg, "\n");
    
    for(i = 0; i < length; i++) {
        send(user->conn, msg, strlen(msg), 0);
        n = n->next;
        user = n->data;
    }
}