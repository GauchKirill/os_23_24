#include <stdio.h>
#include <sys/poll.h>
#include <unistd.h>

#define TIMEOUT 5000 //ms

int main()
{
    struct pollfd fds[2];

    fds[0].fd = 0;
    fds[0].events = POLLIN;
    fds[1].fd = 1;
    fds[1].events = POLLOUT;
    while (1)
    {
        int res = poll(fds, 2, TIMEOUT);

        if (!res)
        {
            printf("Timeout\n");
            return 0;
        }
        if (fds[0].revents & POLLIN)
        {
            char str[256];
            read(fds[0].fd, str, 256);
            printf("%s", str);

        } else
        if (fds[1].revents & POLLOUT)
        {

        }
    }

    return 0;
}