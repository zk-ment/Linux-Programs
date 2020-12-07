/*************************************************************************
> File Name: epoll.c
> Author: wangyu
> Mail: wangyu1092@163.com
> Created Time: Mon 07 Dec 2020 04:41:10 PM CST
************************************************************************/

#include "epoll.h"

// 初始化event
void eventSet(struct myevent_s *ev, int sockfd, void (*callback)(int, int, void *), void *arg) {
    ev ->fd = sockfd;
    ev ->events = 0;
    ev ->arg = arg;
    ev ->callback = callback;
    ev ->status = 0;
    memset(ev ->buff, 0, sizeof(ev ->buff));
    ev ->len = 0;
    ev ->last_active = time(NULL);
}

// 向epfd上添加需要监听的fd
void eventAdd(int events, struct myevent_s *ev) {
    struct epoll_event epv;
    int op;

    epv.data.ptr = ev;
    epv.events = ev ->events = events;

    if (ev ->status == 1) {
        op = EPOLL_CTL_MOD;
    } else {
        op = EPOLL_CTL_ADD;
        ev ->status = 1;
    }

    if (epoll_ctl(g_epfd, op, ev ->fd, (struct epoll_event *)&epv) < 0) {
        fprintf(stderr, "%s : %d) epoll_ctl()\n", __func__, __LINE__);
        exit(1);
    }
}

// 将在epfd上监听的用户fd删除
void eventDel(struct myevent_s *ev) {
    struct epoll_event epv;
    if (ev ->status != 1) {
        return ;
    }

    epv.data.ptr = ev;
    ev ->status = 0;
    epoll_ctl(g_epfd, EPOLL_CTL_DEL, ev ->fd, (struct epoll_event *)&epv);
}

// 向用户fd返回数据的回调函数
void sendData(int fd, int events, void *arg) {
    struct myevent_s *ev = (struct myevent_s *)arg;
    int len;

    len = send(fd, (void *)&ev ->buff, ev ->len, 0);

    eventDel(ev);

    if (len <= 0) {
        printf(YELLOW"<fd : %d) send 0 and will closed\n"NONE, fd);
        close(fd);
        return ;
    }
    
    eventSet(ev, fd, recvData, ev);
    eventAdd(EPOLLIN, ev);
}

// 接受用户fd发来数据的回调函数
void recvData(int fd, int events, void *arg) {
    struct myevent_s *ev = (struct myevent_s *)arg;
    int len;

    len = recv(fd, (void *)&ev ->buff, sizeof(ev ->buff), 0);
    
    eventDel(ev);
    
    if (len <= 0) {
        printf(YELLOW"<fd : %d) recv 0 and will closed\n"NONE, fd);
        close(fd);
        return ;
    }

    ev ->len = strlen(ev ->buff);
    printf(GREEN"<fd : %d) revc %d data: %s\n"NONE, fd, ev ->len, ev ->buff);
    strcat(ev ->buff, ", server return");
    ev ->len = strlen(ev ->buff);
    ev ->callback = sendData;
    ev ->last_active = time(NULL);

    // 这里不能使用 "eventSet(ev, fd, sendData, ev)", 初始化会将buff中的数据清除，用户收不到;
    eventAdd(EPOLLOUT, ev);
}

// 监听fd的回调函数
void acceptCon(int sockfd, int events, void *arg) {
    int newfd, i, flag;

    if ((newfd = accept(sockfd, NULL, NULL)) < 0) {
        perror("accept()");
        exit(1);
    }

    for (i = 0; i < CONNECT_SIZE; i++) {
        if (g_events[i].status == 0) {
            break;
        }
    }

    if (i == CONNECT_SIZE) {
        fprintf(stderr, "connect size %d limit\n", CONNECT_SIZE);
        return ;
    }
    
    // 设置用户fd的非阻塞IO
    if ((flag = fcntl(newfd, F_SETFL, O_NONBLOCK)) < 0) {
        perror("fcntl()");
        exit(1);
    }

    eventSet((struct myevent_s *)&g_events[i], newfd, recvData, (struct myevent_s *)&g_events[i]);
    eventAdd(EPOLLIN, (struct myevent_s *)&g_events[i]);
}

int initEpollSocket(int port) {
    int sockfd, opt = 1;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(1);
    }

    fcntl(sockfd, F_SETFL, O_NONBLOCK);  // 设置监听fd的非阻塞IO
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));  // 设置端口重用

    // 自定义myevent_s初始化
    eventSet((struct myevent_s *)&g_events[CONNECT_SIZE], sockfd, acceptCon, (struct myevent_s *)&g_events[CONNECT_SIZE]);
    
    // 将监听fd的event添加到epfd上
    eventAdd(EPOLLIN, (struct myevent_s *)&g_events[CONNECT_SIZE]);

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind()");
        exit(1);
    }

    if (listen(sockfd, CONNECT_SIZE) < 0) {
        perror("listen()");
        exit(1);
    }

    return sockfd;
}

