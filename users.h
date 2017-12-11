#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _User {
    char* nick;
    char* hostname;
    int id;
    int conn;
} User;

typedef struct _Node {
    struct _Node* prev;
    struct _Node* next;
    User* data;
} Node;

int userlist_length(Node*);
int print_user_list(Node*);
char* get_user_list(Node*, char*);
int change_nick(User*, char*);

Node* userlist_init();
Node* add_user(Node* userlist, char* nick, char* hostname, int id, int* conn);
Node* del_user(Node* userlist, char* nick);
User* get_by_id(Node*, int);
User* get_by_nick(Node*, char*);