#include "net.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void login(Peer server, char * username, int port) {
    Message message = new_message(LOGIN_REQUEST, (char *) malloc(sizeof(char) * MAX_SIZE));
    sprintf(message->message, "%s\t%d", username, port);
    send_message(message, server);
    free(message);
}

void logout(Peer server) {
    Message message = new_message(LOGOUT_REQUEST, NULL);
    send_message(message, server);
    free(message);
}

void list_users(Peer server) {
    Message message = new_message(LIST_USERS_REQUEST, NULL);
    send_message(message, server);
    free(message);
}

void send_inbox(Peer server, char * username, char * msg) {
    Message message = new_message(INBOX_REQUEST, (char *) malloc(sizeof(char) * MAX_SIZE));
    strcat(message->message, username);
    strcat(message->message, "\t");
    strcat(message->message, msg);
    send_message(message, server);
    free(message);
}

void send_file(Peer server, char * username, char * file) {
    Message message = new_message(FILE_TRANSFER_REQUEST, (char *) malloc(sizeof(char) * MAX_SIZE));
    strcat(message->message, username);
    strcat(message->message, "\t");
    strcat(message->message, file);
    send_message(message, server);
    free(message);
}

void receive_inbox() {
    Message message;
    char *msg, *username, *file, *ip, *port;
    int i = 0;
    receive_message(&message, NULL);

    switch(message->type) {
        case LIST_USERS_RESPONSE: 
            printf("\r%s\n> ", message->message);
            break;
        case INBOX_RESPONSE:
            username = message->message;
            while(username[i] != '\t') i++;
            username[i] = '\0';
            msg         = &username[i+1];
            printf("\r%s: %s\n> ", username, msg);
            break;
        case FILE_TRANSFER_RESPONSE:
            file = message->message + 1;
            while(file[i] != '\t') i++;
            file[i] = '\0';
            ip      = &file[i+1];
            while(file[i] != '\t') i++;
            file[i] = '\0';
            port    = &file[i+1];   
            printf("\rtransferindo %s para %s:%s\n> ", file, ip, port);
            transer_file(file, ip, port);
            break;
    }
    fflush(stdout);
    fflush(stdin);
    free(message);
}

void get_command(Peer server) {
    char command[MAX_SIZE], username[MAX_SIZE], message[MAX_SIZE];
    scanf("%s", command);
    if (strcmp(command, "listar") == 0) list_users(server);
    if (strcmp(command, "mensagem") == 0) {
        scanf("%s", username);
        fgets(message, MAX_SIZE, stdin);
        message[strlen(message) - 1] = '\0';
        send_inbox(server, username, message);
    }
    if (strcmp(command, "enviar") == 0) {
        scanf("%s", username);
        fgets(message, MAX_SIZE, stdin);
        message[strlen(message) - 1] = '\0';
        send_file(server, username, message);
    }
    printf("> ");
}

int main(int argc, char **argv) {
    int serverPort, localUDPPort, localTCPPort;
    char * serverIp, * username;
    Peer server;

    printf("digite o ip do servidor:");
    if (argc < 2) {
        serverIp = (char *) malloc(sizeof(char) * MAX_SIZE);
        scanf("%s", serverIp);
    } else {
        serverIp = argv[1];
        printf("%s\n", serverIp);
    }

    printf("digite a porta do servidor:");
    if (argc < 3) {
        scanf("%d", &serverPort);
    } else {
        serverPort = atoi(argv[2]);
        printf("%d\n", serverPort);
    }

    printf("digite a porta local(udp):");
    if (argc < 4) {
        scanf("%d", &localUDPPort);
    } else {
        localUDPPort = atoi(argv[3]);
        printf("%d\n", localUDPPort);
    }

    printf("digite a porta local(tcp):");
    if (argc < 5) {
        scanf("%d", &localTCPPort);
    } else {
        localTCPPort = atoi(argv[4]);
        printf("%d\n", localTCPPort);
    }

    printf("digite o seu username:");
    if (argc < 6) {
        username = (char *) malloc(sizeof(char) * MAX_SIZE);
        scanf("%s", username);
    } else {
        username = argv[5];
        printf("%s\n", username);
    }

    printf("> ");

    server = new_peer(serverPort, serverIp);
    bind_port(localUDPPort, localTCPPort);
    login(server, username, localTCPPort);

    if (fork() == 0) {
        while(1) get_command(server);
    } else {
        while(1) receive_inbox();
    }
    
    return 0;
}
