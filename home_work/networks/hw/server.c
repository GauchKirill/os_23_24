#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>

#define PORT 8123
#define MAX_SZ 4096
#define CNT_THREADS 100

typedef struct Thread_args_
{
    size_t num_thread;
    int connfd;
} Thread_args;

void* func(void *data)
{
    Thread_args *thread_args = (Thread_args*) data;
    char buf[MAX_SZ];
    while (1)
    {
        memset(buf, 0, MAX_SZ);
        int len = read(thread_args->connfd, buf, MAX_SZ);
        printf("From client %ld: %s\n", thread_args->num_thread, buf);
        write(thread_args->connfd, buf, len);
    }
    return NULL;
}

int main()
{
    int sockfd;
    struct sockaddr_in servaddr;
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

    const int one = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, (int*)&one, sizeof(int)) < 0)
    {
        perror("set opt");
        return 1;
    }

    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0)
    {
        perror("bind");
        return 1;
    }

    pthread_t   thread[CNT_THREADS];
    Thread_args thread_args[CNT_THREADS];
    size_t i = 0;
    for (; i < CNT_THREADS; i++)
    {
        if (listen(sockfd, 5) != 0)
        {
            perror("listen");
            continue;
        }

        struct sockaddr_in cliaddr;
        memset(&cliaddr, 0, sizeof(cliaddr));
        unsigned len = sizeof(cliaddr);
        thread_args[i].connfd = accept(sockfd, (struct sockaddr*) &cliaddr, &len);
        if (thread_args[i].connfd < 0)
        {
            perror("accept");
            continue;
        }
        printf("Client's IP:%s\n", inet_ntoa(cliaddr.sin_addr));
        
        thread_args[i].num_thread = i;
        pthread_create(&thread[i], NULL, func, thread_args + i);
        pthread_detach(thread[i]);
    }
    for (size_t j = 0; j < i; j++)
        pthread_join(thread[j], NULL);
    
    return 0;
}