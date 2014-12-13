#include "net.h"

int sock;

int new_socket() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) exit(1);
    return sockfd;
}

Peer new_peer(int port, char *ip) {
    Peer peer  = (Peer) malloc(sizeof(struct Peer));
    peer->port = port;
    peer->ip   = ip;
    return peer;
}

Message new_message(int type, char *message) {
    Message msg  = (Message) malloc(sizeof(struct Message));
    msg->type    = type;
    msg->message = message;
    return msg;
}

void bind_port(int port) {
    struct sockaddr_in addr;
    sock = new_socket();
    bzero(&addr, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0) perror("bind");
}

void send_message(Message message, Peer peer) {
    struct sockaddr_in addr;
    char * msg = malloc(sizeof(char) * MAX_SIZE);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(peer->port);
    sprintf(msg, "[%d]%s", message->type, message->message);
    if (inet_pton(AF_INET, peer->ip, &addr.sin_addr) <= 0) perror("inetpton");
    if (sendto(sock, msg, strlen(msg), 0, (struct sockaddr*) &addr, sizeof(addr)) < 0) perror("sendto");
}

void receive_message(Message * message, Peer * peer) {
    int size, len;
    struct sockaddr_in addr;
    char * msg = malloc(sizeof(char) * MAX_SIZE);
    len = sizeof(addr);
    bzero(&addr, sizeof(addr));
    if ((size = recvfrom(sock, msg, MAX_SIZE, 0, (struct sockaddr *) &addr, (socklen_t *)&len)) < 0) perror("recvfrom");
    msg[size] = '\0';
    if (peer) *peer       = new_peer(ntohs(addr.sin_port), inet_ntoa(addr.sin_addr));
    if (message) *message = new_message(msg[1] - '0', msg + 3);
}
