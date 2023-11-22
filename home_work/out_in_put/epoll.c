#include <sys/epoll.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

int main()
{
    struct epoll_event events[5];
    struct epoll_event event1;
    int epoll_fd = epoll_create1(0);
    char buf[4096];
    int len;

    event1.events = EPOLLIN;
    event1.data.fd = 0;

    struct epoll_event event2;
    event2.events = EPOLLIN;
    event2.data.fd = open("a.txt", O_RDWR | O_CREAT, 0666);

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, 0, &event1))
    {
        printf("fail\n");
    }
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, 0, &event2))
    {
        printf("fail\n");
    }

    int ev_count = epoll_wait(epoll_fd, events, 5, 5000);
    if (ev_count == -1)
    {
        printf("epoll error\n");
    }

    printf("Events count: %d\n", ev_count);
    for (int i = 0; i < ev_count; i++)
    {
        printf("Handling file descriptor: %u\n", events[i].data.fd);
        if (events[i].events & EPOLLIN)
        {
            len = read(events[i].data.fd, buf, sizeof(buf));
            buf[len] = '\0';
            printf("%s\n", buf);
        }
        
    }

    return 0;
}