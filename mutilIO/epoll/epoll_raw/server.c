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
#include <sys/epoll.h>

#define CONNECT_SIZE 5000

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage : %s port!\n", argv[0]);
        exit(1);
    }

    int sockfd, newfd, port, epfd, nready;
    struct epoll_event evts[100];

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

    if ((epfd = epoll_create(100)) < 0) {
        perror("epoll_create()");
        exit(1);
    }

    struct epoll_event eve;
    eve.data.fd = sockfd;
    eve.events = EPOLLIN;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, (struct epoll_event *)&eve) < 0) {
        perror("epoll_ctl()");
        exit(1);
    }

    while(1) {
        if ((nready = epoll_wait(epfd, evts, 100, -1)) < 0) {
            perror("select()");
            exit(1);
        }
        for (int i = 0; i < nready; i++) {
            if (evts[i].data.fd == sockfd) {
                if ((newfd = accept(sockfd, NULL, NULL)) < 0) {
                    perror("accept()");
                    exit(1);
                }
                struct epoll_event neweve;
                neweve.data.fd = newfd;
                neweve.events = EPOLLIN;
                if (epoll_ctl(epfd, EPOLL_CTL_ADD, newfd, (struct epoll_event *)&neweve) < 0) {
                    perror("epoll_ctl()");
                    exit(1);
                }
            } else {
                char buff[1024] = {0};
                int newfd2 = evts[i].data.fd;
                if (recv(newfd2, (void *)&buff, sizeof(buff), 0) == 0) {
                    epoll_ctl(epfd, EPOLL_CTL_DEL, newfd2, (struct epoll_event *)&evts[i]);
                    close(newfd2);
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
