/*************************************************************************
> File Name: server.c
> Author: wangyu
> Mail: wangyu1092@163.com
> Created Time: Fri 04 Dec 2020 09:52:09 PM CST
************************************************************************/

#include "epoll.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage : %s port!\n", argv[0]);
        exit(1);
    }

    int port, nready, sockfd, checkpos = 0;
    struct epoll_event evts[CONNECT_SIZE + 1];

    port = atoi(argv[1]);

    if ((g_epfd = epoll_create(CONNECT_SIZE + 1)) < 0) {
        perror("epoll_create()");
        exit(1);
    }

    sockfd = initEpollSocket(port);
    
    while(1) {

        long now = time(NULL);
        
        // 如果用户连接过久而没有发出数据，则剔除
        for (int i = 0; i < 100; i++, checkpos++) {
            if (checkpos == CONNECT_SIZE) {
                checkpos = 0;
            }
            if (g_events[checkpos].status != 1) {
                continue;
            }

            long duration = now - g_events[checkpos].last_active;

            // printf("%ld\n", duration);

            if (duration >= 60) {
                close(g_events[checkpos].fd);
                printf(RED"[fd = %d] : timeout for 60 seconds, refuse!\n"NONE, g_events[checkpos].fd);
                eventDel(&g_events[checkpos]);
            } else if (duration && duration % 15 == 0) {
                printf(YELLOW"[fd = %d] : %ld seconds no see!\n"YELLOW, g_events[checkpos].fd, duration);
            }
        }

        if ((nready = epoll_wait(g_epfd, evts, CONNECT_SIZE, 1000)) < 0) {
            perror("epoll_wait()");
            exit(1);
        }
        for (int i = 0; i < nready; i++) {

            struct myevent_s *ev = evts[i].data.ptr;

            if ((evts[i].events & EPOLLIN) && (ev ->events & EPOLLIN)) {
                ev ->callback(ev ->fd, evts[i].events, ev ->arg);
            }

            if ((evts[i].events & EPOLLOUT) && (ev ->events & EPOLLOUT)) {
                ev ->callback(ev ->fd, evts[i].events, ev ->arg);
            }
        }
    }
    close(sockfd);
    eventDel(&g_events[CONNECT_SIZE]);
    return 0;
}
