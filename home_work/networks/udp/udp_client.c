#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define PORT 8102
#define MSG_LEN 1024

int main()
{
    int sockfd;
    char buffer[MSG_LEN];
    struct sockaddr_in servaddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
    }
    
    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY; //можно 127.0.0.1
    //inet_pton(AF_INET, "127.0.0.1", &(servaddr.sin_addr.s_addr));
    int n = 0, len = 0;

    sendto(sockfd, "Hello", strlen("Hello"), MSG_CONFIRM, (struct sockaddr*) &servaddr, sizeof(servaddr));
    printf("Message is sent\n");
    n = recvfrom(sockfd, buffer, MSG_LEN, MSG_WAITALL, (struct sockaddr*) &servaddr, (socklen_t*) &len);
    printf("%d\n", len);
    buffer[n] = '\0';
    printf("%s\n", buffer);
    close(sockfd);
    return 0;
}