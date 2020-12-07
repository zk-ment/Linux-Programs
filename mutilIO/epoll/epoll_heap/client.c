/*************************************************************************
	> File Name: client.c
	> Author: wangyu
	> Mail: wangyu1092@163.com
	> Created Time: Fri 04 Dec 2020 10:03:11 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage : %s ip port!\n", argv[0]);
        exit(1);
    }

    int sockfd, port;
    char ip[20] = {0};

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(1);
    }
    
    strcpy(ip, argv[1]);
    port = atoi(argv[2]);

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port =  htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect()");
        exit(1);
    }

    while(1) {
        char buff[1024] = {0};
        scanf("%[^\n]s", buff);
        getchar();
        if (strcmp(buff, "exit") == 0) {
            close(sockfd);
            break;
        }
        send(sockfd, (void *)&buff, strlen(buff), 0);
        if (recv(sockfd, (void *)&buff, sizeof(buff), 0) < 0) {
            close(sockfd);
            break;
        }
        printf("%s\n", buff);
    }
    return 0;
}
