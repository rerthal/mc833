#include "net.h"

int sock, tcp_sock;
void wait_files(int);

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

void bind_port(int port, int tcp_port) {
    struct sockaddr_in addr;
    sock = new_socket();
    bzero(&addr, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0) return perror("bind");

    if (!tcp_port) return;
    tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_sock < 0) 
    bzero(&addr, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(tcp_port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(tcp_sock, (struct sockaddr*) &addr, sizeof(addr)) < 0) return perror("bind");
    if (listen(tcp_sock, 1) == -1) perror("listen");
    wait_files(tcp_sock);
    close(tcp_sock);
}

void send_message(Message message, Peer peer) {
    struct sockaddr_in addr;
    char * msg = malloc(sizeof(char) * MAX_SIZE);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(peer->port);
    sprintf(msg, "[%d]%s", message->type, message->message);
    if (inet_pton(AF_INET, peer->ip, &addr.sin_addr) <= 0) return perror("inetpton");
    if (sendto(sock, msg, strlen(msg), 0, (struct sockaddr*) &addr, sizeof(addr)) < 0) return perror("sendto");
}

void receive_message(Message * message, Peer * peer) {
    int size, len;
    struct sockaddr_in addr;
    char * msg = malloc(sizeof(char) * MAX_SIZE);
    len = sizeof(addr);
    bzero(&addr, sizeof(addr));
    if ((size = recvfrom(sock, msg, MAX_SIZE, 0, (struct sockaddr *) &addr, (socklen_t *)&len)) < 0) return perror("recvfrom");
    msg[size] = '\0';
    if (peer) *peer       = new_peer(ntohs(addr.sin_port), inet_ntoa(addr.sin_addr));
    if (message) *message = new_message(msg[1] - '0', msg + 3);
}

void wait_files(int sockfd) {
    int connfd;
    int i;
    char in[MAX_SIZE];
    socklen_t len;
    struct sockaddr_storage cliaddr;
    len = sizeof(cliaddr);
    FILE *fp;
    if (fork() == 0) {
        while (1) {
            fp = fopen("./received-file", "w");
            connfd = accept(sockfd, (struct sockaddr *) &cliaddr, &len);
            while((i = read(connfd, in, MAX_SIZE)) > 0) {
                in[i] = '\0';
                write(fileno(fp), in, strlen(in));
            }
            fclose(fp);
            close(connfd);
        }
    }
}

void transer_file(char* file, char* ip, char* port) {
    struct sockaddr_in addr;
    int sock, i;
    char * out = malloc(sizeof(char *) * MAX_SIZE);
    FILE *fp = fopen(file, "r");
    if (!fp) return perror("fopen");
    sock = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&addr, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(atoi(port));
    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) return perror("inet_pton");
    if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) return perror("connect");
    while((i = read(fileno(fp), out, MAX_SIZE)) > 0) write(sock, out, strlen(out));
}