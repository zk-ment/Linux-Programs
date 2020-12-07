/*************************************************************************
	> File Name: server.c
	> Author: wangyu
	> Mail: wangyu1092@163.com
	> Created Time: Thu 26 Nov 2020 08:44:03 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char **argv) {
    int port, sockfd;
    if (argc != 2) {
        fprintf(stderr, "Usage : %s port\n", argv[0]);
        exit(1);
    }
    port  = atoi(argv[1]);
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(1);
    }
    struct sockaddr_in addr;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(0);
    addr.sin_family = AF_INET;

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind()");
        exit(1);
    }

    if (listen(sockfd, 10) < 0) {
        perror("listen()");
        exit(1);
    }

    while(1) {
        int newfd;
        char buff[1024] = {0};
        if ((newfd = accept(sockfd, NULL, NULL)) < 0) {
            perror("accept()");
            continue;
        }

        recv(newfd, buff, sizeof(buff), 0);
        printf("<recv> : %s\n", buff);
    }
    return 0;
}
