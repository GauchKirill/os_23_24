#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8123
#define MAX_SZ 4096

void f(int sockfd)
{
    char buf[MAX_SZ] = {};
    while (1)
    {
        memset(buf, 0, MAX_SZ);
        printf("Exter the massege...\n");
        scanf("%s", buf);
        buf[strlen(buf)] = '\0';
        write(sockfd, buf, strlen(buf));
        memset(buf, 0, MAX_SZ);
        read(sockfd, buf, MAX_SZ);
        printf("%s\n", buf);
        
    }
    
}


int main()
{
    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("sicket");
        return 1;
    }
        
    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);

    if (connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) != 0)
    {
        perror("connect");
        return 1;
    }

    printf("Connected...\n");
    f(sockfd);
    return 0;
}