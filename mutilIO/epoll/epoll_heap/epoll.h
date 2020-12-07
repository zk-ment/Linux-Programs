/*************************************************************************
	> File Name: epoll.h
	> Author: wangyu
	> Mail: wangyu1092@163.com
	> Created Time: Mon 07 Dec 2020 04:33:42 PM CST
 ************************************************************************/

#ifndef _EPOLL_H
#define _EPOLL_H

#include "head.h"

struct myevent_s {
    int fd;
    int events;
    void *arg;
    void (*callback)(int fd, int events, void *arg);
    int status;
    char buff[BUFF_SIZE];
    int len;
    long last_active;
};

struct myevent_s g_events[CONNECT_SIZE + 1];
int g_epfd;

void eventAdd(int events, struct myevent_s *ev);

void eventSet(struct myevent_s *ev, int sockfd, void (*callback)(int, int, void *), void *arg);

void eventDel(struct myevent_s *ev);

void sendData(int fd, int events, void *arg);

void recvData(int fd, int events, void *arg);

void acceptCon(int sockfd, int events, void *arg);

int initEpollSocket(int port);



#endif
