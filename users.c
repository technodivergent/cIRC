#include "users.h"

int print_user_list(Node* userlist) {
    Node* head = userlist;
    Node* n = head;
    
    if(userlist == NULL) { printf("Userlist empty.\n"); return 0; }
    
    printf("Printing users list\n");
    
    User* user = (User*) n->data;
    
    do
    {
        user = (User*) n->data;
        printf("\tNick:\t\t%s\n",         user->nick);
        printf("\tHostname:\t%s\n",    user->hostname);
        printf("\tID:\t\t%d\n",          user->id);
        printf("\tConnection:\t%d\n",  user->conn);
        printf("\t-----------\n");
        n = n->next;
    } while(n != head);
    return 0;
}

char* get_user_list(Node* userlist, char* msg) {
    Node* head = userlist;
    Node* n = head;
    
    if(userlist == NULL) { printf("Userlist empty.\n"); return 0; }
    
    sprintf(msg, "Printing users list\n");
    
    User* user = (User*) n->data;
    
    do
    {
        user = (User*) n->data;
        sprintf(msg + strlen(msg), "\tNick:\t\t%s\n",       user->nick);
        sprintf(msg + strlen(msg), "\tHostname:\t%s\n",     user->hostname);
        sprintf(msg + strlen(msg), "\tUser ID:\t%d\n",      user->id);
        sprintf(msg + strlen(msg), "\tConnection ID:\t%d\n",user->conn);
        sprintf(msg + strlen(msg), "\t-----------\n");
        n = n->next;
    } while(n != head);
    return msg;
}

Node* add_node(Node* userlist) {
    Node* node = malloc(sizeof(Node));
    if(userlist == NULL)
    { 
        userlist = node;
        userlist->prev = userlist;
        userlist->next = userlist;
    } 
    else
    {
        node->next = userlist;
        node->prev = userlist->prev;
        userlist->prev->next = node;
        userlist->prev = node;
        if(userlist->next == userlist) { userlist->next = node; }
        userlist = node;
    }
    userlist->data = malloc(sizeof(User));
    return userlist;
}

Node* del_node(Node* userlist) {
    if(userlist == NULL) { return NULL; }
    else if(userlist->next == userlist) { free(userlist); return NULL; }
    else
    {
        Node *n = userlist;
        userlist->prev->next = userlist->next;
        userlist->next->prev = userlist->prev;
        userlist = userlist->next;
        free(n);
        return userlist;
    }
}

Node* add_user(Node* userlist, char* nick, char* hostname, int id, int* conn) {
    if(userlist == NULL) { return userlist; }
    else {
        User *user = (User*) userlist->data;
        if(user->id != -1) {
            userlist = add_node(userlist);
        }
        user = (User*) userlist->data;
        user->id = id;
        user->conn = *conn;
        user->nick = malloc(strlen(nick) + 1);
        user->hostname = malloc(strlen(hostname)+1);
        strcpy(user->nick, nick);
        strcpy(user->hostname, hostname);
        return userlist;
    }
}

Node* del_user(Node* userlist, char* nick) {
    if(userlist == NULL) { return NULL; }
    else {
        Node *n = userlist;
        User* user = (User*) n->data;
        if(strcmp(user->nick, nick) == 0) {
            userlist = del_node(n);
            return userlist;
        }
        n = n->next;
        user = (User*) n->data;
        while(n != userlist) {
            if(strcmp(user->nick, nick) == 0) {
                userlist = del_node(n);
                return userlist;
            }
            n = n->next;
            user = (User*) n->data;
        }
        printf("User %s not found.\n", nick);
        return userlist;
    }
}

Node* userlist_init() {
    Node* users = malloc(sizeof(Node));
    users->next = users;
    users->prev = users;
    users->data = malloc(sizeof(User));
    
    User* user = (User*) users->data;
    user->id = -1;
    return users;
}

int userlist_length(Node* userlist) {
    if(userlist == NULL || (userlist->data)->id == -1) { return 0; }
    else {
        Node* head = userlist;
        Node* n = head;
        int i = 1;
        n = n->next;
        while(n != head) {
            n = n-> next;
            i += 1;
        }
        return i;
    }
}

User* get_by_id(Node* userlist, int conn) {
    int i;
    int length = userlist_length(userlist);
    Node* n = userlist;
    User* user = n->data;
    for(i = 0; i < length; i++) {
        if(user->conn == conn) {
            return user;
        }
        n = n->next;
        user = n->data;
    }
    return NULL;
}

User* get_by_nick(Node* userlist, char* nick) {
    int i;
    int length = userlist_length(userlist);
    Node *n = userlist;
    User* user = n->data;
    
    for(i = 0; i < length; i++) {
        if(strcmp(user->nick, nick) == 0) {
            return user;
        }
        n = n->next;
        user = n->data;
    }
    return NULL;
}

int change_nick(User* user, char new_nick[9]) {
    strcpy(user->nick, new_nick);
    return 0;
}