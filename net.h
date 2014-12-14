#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#ifndef NET_H
#define NET_H 1
#define MAX_SIZE 2048

#define LOGIN_REQUEST 0
#define LOGIN_RESPONSE 1
#define LOGOUT_REQUEST 2
#define LOGOUT_RESPONSE 3
#define LIST_USERS_REQUEST 4
#define LIST_USERS_RESPONSE 5
#define INBOX_REQUEST 6
#define INBOX_RESPONSE 7
#define FILE_TRANSFER_REQUEST 8
#define FILE_TRANSFER_RESPONSE 9

typedef struct Peer {
    int port;
    char * ip;
} * Peer;

typedef struct Message {
    int type;
    char * message;
} * Message;

Peer new_peer(int, char*);
Message new_message(int, char*);
void bind_port(int, int);
void send_message(Message, Peer);
void receive_message(Message*, Peer*);
void transer_file(char*, char*,char*);

#endif