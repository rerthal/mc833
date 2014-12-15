#include "net.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct ClientNode {
    Peer client;
    int TCPport;
    char * username;
    struct ClientNode * next;
} * ClientNode;

ClientNode clientList;

void login(Peer client, Message message) {
    char *port, *username;
    ClientNode node  = (ClientNode) malloc(sizeof(struct ClientNode));
    int i = 0;

    username = message->message;
    while(username[i] != '\t') i++;
    username[i] = '\0';
    port        = &username[i+1];

    node->client     = new_peer(client->port, client->ip);
    node->username   = username;
    node->next       = clientList;
    node->TCPport    = atoi(port);
    clientList = node;
    printf("%s:%d se conectando com user name %s\n", client->ip, client->port, username);
}

void logout(Peer client) {
    ClientNode previous, node;
    if (!clientList) return;
    if (strcmp(clientList->client->ip, client->ip) == 0 && clientList->client->port == client->port) {
        node       = clientList;
        clientList = clientList->next;
        free(node);
    } else {
        previous = clientList;
        node     = clientList->next;
        while(node != NULL && !(strcmp(node->client->ip, client->ip) == 0 && node->client->port == client->port)) {
            previous = node;
            node     = node->next;
        }
        if (node != NULL) {
            previous->next = node->next;
            printf("%s:%d se desconectando\n", node->client->ip, node->client->port);
            free(node);
        }
    }
}

void list_users(Peer client) {
    ClientNode node;
    Message message = new_message(LIST_USERS_RESPONSE, (char *) malloc(sizeof(char) * MAX_SIZE));
    for (node = clientList; node != NULL; node = node->next) {
        strcat(message->message, node->username);
        strcat(message->message, "\n");
    }
    message->message[strlen(message->message) - 1] = '\0';
    send_message(message, client);
    free(message);
    printf("%s:%d requisitando listagem de usuários\n", client->ip, client->port);
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
    for (node = clientList; node != NULL; node = node->next) {
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
    printf("%s:%d enviando mensagem para %s:%d\n", source->client->ip, source->client->port, destiny->client->ip, destiny->client->port);
}

void file_transfer(Peer client, Message message) {
    ClientNode source, destiny, node;
    Message inbox;
    char *file, *username;
    int i = 0;

    username = message->message;
    while(username[i] != '\t') i++;
    username[i] = '\0';
    file        = &username[i+1];

    source = NULL;
    destiny = NULL;
    for (node = clientList; node != NULL; node = node->next) {
        if (strcmp(node->client->ip, client->ip) == 0 && node->client->port == client->port) source = node;
        if (strcmp(username, node->username) == 0) destiny = node;
    }
    
    if (!source || !destiny) return;
    inbox = new_message(FILE_TRANSFER_RESPONSE, (char *) malloc(sizeof(char) * MAX_SIZE));
    sprintf(inbox->message, "%s\t%s\t%d", file, destiny->client->ip, destiny->TCPport);
    send_message(inbox, source->client);
    free(inbox);
    printf("%s:%d enviando arquivo para %s:%d\n", source->client->ip, source->client->port, destiny->client->ip, destiny->client->port);
}

int main(int argc, char **argv) {
    Peer client;
    Message message;
    int port;

    printf("digite a porta do servidor:");
    if (argc < 2) {
        scanf("%d", &port);
    } else {
        port = atoi(argv[1]);
        printf("%d\n", port);
    }

    bind_port(port, 0);

    clientList = NULL;

    while(1) {
        receive_message(&message, &client);
        switch (message->type) {
            case LOGIN_REQUEST:
                login(client, message);
                break;
            case LIST_USERS_REQUEST:
                list_users(client);
                break;
            case INBOX_REQUEST:
                send_inbox(client, message);
                break;
            case FILE_TRANSFER_REQUEST:
                file_transfer(client, message);
                break;
            case LOGOUT_REQUEST:
                logout(client);
                break;
        }
        free(message);
        free(client);
    }

    return 0;
}
