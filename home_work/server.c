#define _GNU_SOURCE
#include <bits/types/struct_timeval.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "server.h"
#include <poll.h>

int send_invite_to_slaves() {
	int sockfd;

    struct sockaddr_in servaddr; 
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { // creating a socket for broadcast
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_BROADCAST, &reuse, sizeof(reuse)) == -1) {
        perror("Setsockopt failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    } 

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("192.168.0.255");
	memset(servaddr.sin_zero, '\0', sizeof servaddr.sin_zero);
    
    int numbytes = 0;
    printf("Sending invites\n");
	if ((numbytes=sendto(sockfd, "Hello", strlen("Hello"), 0,
    	(struct sockaddr *)&servaddr, sizeof servaddr)) == -1) {
        perror("sendto");
        exit(1); 
    }

    close(sockfd);
    return 0;
}

int rcv_ip_from_slaves(struct Server* server) {
	int receiveSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (receiveSocket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int reuse = 1;
    if (setsockopt(receiveSocket, SOL_SOCKET, SO_REUSEADDR | SO_BROADCAST, &reuse, sizeof(reuse)) == -1) {
        perror("Setsockopt failed");
        close(receiveSocket);
        exit(EXIT_FAILURE);
    } 

    struct sockaddr_in receiveAddr;
    memset(&receiveAddr, 0, sizeof(receiveAddr));
    receiveAddr.sin_family = AF_INET;
    receiveAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    receiveAddr.sin_port = htons(PORT);

    if (bind(receiveSocket, (struct sockaddr*)&receiveAddr, sizeof(receiveAddr)) == -1) {
        perror("Bind failed");
        close(receiveSocket);
        exit(EXIT_FAILURE);
    }

    printf("Listening for responses...\n");

    char buffer[BUF_SIZE];
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    struct timeval startTime, currentTime;
    gettimeofday(&startTime, NULL);

    while (1) {
        gettimeofday(&currentTime, NULL);
        int elapsedSeconds = currentTime.tv_sec - startTime.tv_sec;

        if (elapsedSeconds >= WAIT_TIME) {
            printf("Received responses for %d seconds. Exiting...\n", WAIT_TIME);
            break;
        }

        struct pollfd fds;
        fds.fd = receiveSocket;
        fds.events = POLLIN;

        int ready = poll(&fds, 1, 0);

        if (ready == -1) {
            perror("Select error");
            exit(EXIT_FAILURE);
        }

        if (ready > 0) {
            ready = 0;
            ssize_t recvBytes = recvfrom(receiveSocket, buffer, BUF_SIZE - 1, 0, (struct sockaddr*)&clientAddr, &addrLen);
            if (recvBytes == -1) {
                perror("Recvfrom failed");
                exit(EXIT_FAILURE);
            }

            buffer[recvBytes] = '\0';

            // Store received slave information in the server struct
            if (server->num_of_slaves < MAX_SLAVES) {
                struct Slave newslave;
                strcpy(newslave.ip, inet_ntoa(clientAddr.sin_addr));
                newslave.num_of_cores = atoi(buffer);

                server->slaves[server->num_of_slaves] = newslave;
                server->num_of_slaves++;
				printf("New slave: num cores:%d, IP: %s\n", newslave.num_of_cores, newslave.ip);
            } else {
                printf("Maximum slavers reached. Ignoring additional responses.\n");
            }
        }
    }

    close(receiveSocket);
    return 0;
}

void establish_connections(struct Server *server) {
    for (int i = 0; i < server->num_of_slaves; ++i) {
        int slaveSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (slaveSocket == -1) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }

        int reuse = 1;
        if (setsockopt(slaveSocket, SOL_SOCKET, SO_REUSEADDR | SO_BROADCAST | SO_REUSEPORT, &reuse, sizeof(reuse)) == -1) {
            perror("Setsockopt failed");
            close(slaveSocket);
            exit(EXIT_FAILURE);
        } 

        struct sockaddr_in slaveAddr;
        memset(&slaveAddr, 0, sizeof(slaveAddr));
        slaveAddr.sin_family = AF_INET;
        slaveAddr.sin_port = htons(PORT);
        inet_pton(AF_INET, server->slaves[i].ip, &slaveAddr.sin_addr);

        if (connect(slaveSocket, (struct sockaddr *)&slaveAddr, sizeof(slaveAddr)) == -1) {
            perror("Connection failed");
            close(slaveSocket);
            exit(EXIT_FAILURE);
        }

        // Connection established, store the socket in the slave struct
        server->slaves[i].socket = slaveSocket;
    }
}

void split_tasks(struct Server* server, struct Task main_task) {
    int num_of_cores = 0;
    for (int i = 0; i < server->num_of_slaves; i++) {
        num_of_cores += server->slaves[i].num_of_cores;
    }

    for (int i = 0, loaded_cores = 0; i < server->num_of_slaves; i++) {
        server->slaves[i].task.x1 = (main_task.x2 - main_task.x1) / num_of_cores * (loaded_cores) + main_task.x1;
        server->slaves[i].task.y1 = main_task.y1;
        loaded_cores += server->slaves[i].num_of_cores;
        server->slaves[i].task.x2 = (main_task.x2 - main_task.x1) / num_of_cores * (loaded_cores) + main_task.x1;
        server->slaves[i].task.y2 = main_task.y2;
        server->slaves[i].task.num_of_points = main_task.num_of_points / num_of_cores * server->slaves[i].num_of_cores;
    }
}
int print_one_task(char* dest, struct Task *task) {
    return sprintf(dest, "%lf %lf %lf %lf %lld \n", task->x1, task->y1, task->x2, task->y2, task->num_of_points);
}

void send_task(struct Server *server) {
    char buffer[BUF_SIZE];
    for (int i = 0; i < server->num_of_slaves; ++i) {

        int len = print_one_task(buffer, &(server->slaves[i].task));

        write(server->slaves[i].socket, buffer, len);
    }
}


double gather_results(struct Server* server) {

    struct pollfd fds[server->num_of_slaves];
    for (int i = 0; i < server->num_of_slaves; i++)
    {
        fds[i].fd = server->slaves[i].socket;
        fds[i].events = POLLIN;
    }

    struct timeval startTime, endTime;
    gettimeofday(&startTime, NULL);

    int res = 0;
    while (res < server->num_of_slaves)
    {
        res = poll(fds, server->num_of_slaves, 0);
        gettimeofday(&endTime, NULL);
        if (endTime.tv_sec - startTime.tv_sec > WAIT_TIME) break;
    }
    
    if (res < 0)
    {
        perror("poll failed");
        exit(EXIT_FAILURE);
    }
    else
    if (res < server->num_of_slaves)
    {
        printf("Not all slaves have already done\n");
        exit(EXIT_FAILURE);
    }

    double resault = 0.0;
    for (int i = 0; i < server->num_of_slaves; ++i)
    {
        if (fds[i].revents & POLLIN)
        {
            double receivedValue;
            int bytesRead = recv(server->slaves[i].socket, &receivedValue, sizeof(double), 0);

            if (bytesRead == sizeof(double)) {
                printf("Received value from slave %d: %f\n", i + 1, receivedValue);
                resault += receivedValue;
            } else if (bytesRead == 0) {
                printf("slave %d closed the connection.\n", i + 1);
            } else {
                perror("Recv error");
            }
        }
        close(server->slaves[i].socket);
    }

    return resault;
}

int main()
{
    struct Task main_task = {.x1 = 0.0, .y1 = 0.0, .x2 = 3.0, .y2 = 3.0, .num_of_points = 100000};

    struct Server server = {0};
    send_invite_to_slaves();
    rcv_ip_from_slaves(&server);
    split_tasks(&server, main_task);
    establish_connections(&server);
    sleep(1);
    send_task(&server);
    double res = gather_results(&server);

    printf("Resault: %lf\n", res);

    return 0;
} 