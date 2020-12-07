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
#include <poll.h>

#define CONNECT_SIZE 5000

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage : %s port!\n", argv[0]);
        exit(1);
    }

    int sockfd, newfd, port, max_idx, nready;
    struct pollfd client[CONNECT_SIZE];

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

    max_idx = 0;
    for (int i = 1; i < CONNECT_SIZE; i++) {
        client[i].fd = -1;
    }

    client[0].fd = sockfd;
    client[0].events = POLLIN;

    while(1) {
        
        if ((nready = poll(client, max_idx + 1, -1)) < 0) {
            perror("select()");
            exit(1);
        }
        if (client[0].revents & POLLIN) {
            if ((newfd = accept(sockfd, NULL, NULL)) < 0) {
                perror("accept()");
                exit(1);
            }
            int i;
            for (i = 1; i < CONNECT_SIZE; i++) {
                if (client[i].fd < 0) {
                    client[i].fd = newfd;
                    break;
                }
            }
            client[i].events = POLLIN;
            if (i > max_idx) {
                max_idx = i;
            }
        }

        for (int i = 1; i <= max_idx; i++) {
            int newfd2;
            if ((newfd2 = client[i].fd) < 0) {
                continue;
            }
            if (client[i].revents & POLLIN) {
                char buff[1024] = {0};
                if (recv(newfd2, (void *)&buff, sizeof(buff), 0) == 0) {
                    close(client[i].fd);
                    client[i].fd = -1;
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
