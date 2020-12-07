/*************************************************************************
> File Name: server.c
> Author: wangyu
> Mail: wangyu1092@163.com
> Created Time: Fri 04 Dec 2020 09:52:09 PM CST
************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage : %s port!\n", argv[0]);
        exit(1);
    }

    int sockfd, newfd, maxfd, port, max_idx, nready;
    int client[FD_SETSIZE];
    fd_set rset, allset;

    port = atoi(argv[1]);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(1);
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind()");
        exit(1);
    }

    if (listen(sockfd, 25) < 0) {
        perror("listen()");
        exit(1);
    }

    maxfd = sockfd;
    max_idx = -1;
    for (int i = 0; i < FD_SETSIZE; i++) {
        client[i] = -1;
    }

    FD_ZERO(&allset);
    FD_SET(sockfd, &allset);

    while(1) {
        rset = allset;
        if ((nready = select(maxfd + 1, &rset, NULL, NULL, NULL)) < 0) {
            perror("select()");
            exit(1);
        }
        if (FD_ISSET(sockfd, &rset)) {
            if ((newfd = accept(sockfd, NULL, NULL)) < 0) {
                perror("accept()");
                exit(1);
            }
            int i;
            for (i = 0; i < FD_SETSIZE; i++) {
                if (client[i] < 0) {
                    client[i] = newfd;
                    break;
                }
            }
            FD_SET(newfd, &allset);
            if (newfd > maxfd) {
                maxfd = newfd;
            }
            if (i > max_idx) {
                max_idx = i;
            }
        }

        for (int i = 0; i <= max_idx; i++) {
            int newfd2;
            if ((newfd2 = client[i]) < 0) {
                continue;
            }
            if (FD_ISSET(newfd2, &rset)) {
                char buff[1024] = {0};
                if (recv(newfd2, (void *)&buff, sizeof(buff), 0) == 0) {
                    close(newfd2);
                    FD_CLR(newfd2, &allset);
                    client[i] = -1;
                    continue;
                } 
                strcat(buff, "server");
                send(newfd2, (void *)&buff, strlen(buff), 0);
            }       
        }
    }
    close(sockfd);
    return 0;
}
