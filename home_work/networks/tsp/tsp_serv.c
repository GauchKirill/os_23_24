#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>

#define PORT 8123
#define MAX_SZ 4096

void f(int connfd)
{
    char buf[MAX_SZ];
    while (1)
    {
        memset(buf, 0, MAX_SZ);
        int len = read(connfd, buf, MAX_SZ);
        printf("From client: %s\n", buf);
        write(connfd, buf, len);
    }
    
    return;
}

int main()
{
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
        return 1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0)
    {
        perror("bind");
        return 1;
    }

    if (listen(sockfd, 5) != 0)
    {
        perror("listen");
        return 1;
    }

    memset(&cliaddr, 0, sizeof(cliaddr));
    unsigned len = sizeof(cliaddr);
    int connfd = accept(sockfd, (struct sockaddr*) &cliaddr, &len);
    if (connfd < 0)
    {
        perror("accept");
    }

    printf("Client's IP:%s\n", inet_ntoa(cliaddr.sin_addr));
    f(connfd);

    return 0;
}