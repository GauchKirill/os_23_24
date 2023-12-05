#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define PORT 8102
#define MSG_LEN 1024

int main()
{
    int sockfd;
    char buffer[MSG_LEN];
    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY; //можно 127.0.0.1

    int n = 0, len = 0;

    if (bind(sockfd, (const struct sockaddr*) &servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind");
    }

    n = recvfrom(sockfd, (char*)buffer, MSG_LEN, MSG_WAITALL, (struct sockaddr*) &cliaddr, (socklen_t*) &len);
    printf("Client: IP: %s\nlen = %d\n", inet_ntoa(cliaddr.sin_addr), len);
    buffer[n] = '\0';
    printf("%s\n", buffer);
    inet_pton(AF_INET, "127.0.0.1", &(cliaddr.sin_addr.s_addr));
    int ret = sendto(sockfd, buffer, strlen(buffer), MSG_CONFIRM, (struct sockaddr*) &cliaddr, sizeof(cliaddr));
    perror("sendto");
    printf("send to, %d\n", ret);
    close(sockfd);
    return 0;
}