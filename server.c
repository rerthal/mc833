#include "net.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct ClientNode {
    Peer client;
    char * username;
    struct ClientNode * next;
} * ClientNode;

typedef struct ClientList {
    ClientNode head;
    int size;
} * ClientList;

ClientList clientList;

void login(Peer client, char * username) {
    ClientNode node  = (ClientNode) malloc(sizeof(struct ClientNode));
    node->client     = new_peer(client->port, client->ip);
    node->username   = username;
    node->next       = clientList->head;
    clientList->head = node;
}

void list_users(Peer client) {
    ClientNode node;
    Message message = new_message(LIST_USERS_RESPONSE, (char *) malloc(sizeof(char) * MAX_SIZE));
    for (node = clientList->head; node != NULL; node = node->next) {
        strcat(message->message, node->username);
        strcat(message->message, "\n");
    }
    send_message(message, client);
    free(message);
}

void send_inbox(Peer client, Message message) {
    ClientNode source, destiny, node;
    Message inbox;
    char *msg, *username;
    int i = 0;

    username = message->message;
    while(username[i] != '\t') i++;
    username[i] = '\0';
    msg         = &username[i+1];

    source = NULL;
    destiny = NULL;
    for (node = clientList->head; node != NULL; node = node->next) {
        if (strcmp(node->client->ip, client->ip) == 0 && node->client->port == client->port) source = node;
        if (strcmp(username, node->username) == 0) destiny = node;
    }
    
    if (!source || !destiny) return;
    inbox = new_message(INBOX_RESPONSE, (char *) malloc(sizeof(char) * MAX_SIZE));
    strcat(inbox->message, source->username);
    strcat(inbox->message, "\t");
    strcat(inbox->message, msg);
    send_message(inbox, destiny->client);
    free(inbox);
}

int main(void) {
    Peer client;
    Message message;
    bind_port(8080);

    clientList = (ClientList) malloc(sizeof(struct ClientList));
    clientList->head = NULL;
    clientList->size = 0;

    while(1) {
        receive_message(&message, &client);
        switch (message->type) {
            case LOGIN_REQUEST:
                login(client, message->message);
                break;
            case LIST_USERS_REQUEST:
                list_users(client);
                break;
            case INBOX_REQUEST:
                send_inbox(client, message);
                break;
            default:
                exit(1);
                break;
        }
        free(message);
        free(client);
    }

    return 0;
}
