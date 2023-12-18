#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sched.h>
#include <pthread.h>

#define PORT 8123
#define MAX_CORES 8
#define BUF_SIZE 4096

static unsigned numCore;
static double square = 0.0;
struct Task {
    double x1;
    double x2;
    double y1;
    double y2;
    long long num_of_points;
};

void integral(struct Task *task);

int main() {
    int udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == -1) {
        perror("UDP socket creation failed");
        exit(EXIT_FAILURE);
    }

    int reuse = 1;
    if (setsockopt(udpSocket, SOL_SOCKET, SO_REUSEADDR | SO_BROADCAST, &reuse, sizeof(reuse)) == -1) {
        perror("Setsockopt failed");
        close(udpSocket);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in udpReceiveAddr;
    memset(&udpReceiveAddr, 0, sizeof(udpReceiveAddr));
    udpReceiveAddr.sin_family = AF_INET;
    udpReceiveAddr.sin_addr.s_addr =  inet_addr("192.168.0.255");
    udpReceiveAddr.sin_port = htons(PORT);

    if (bind(udpSocket, (struct sockaddr *)&udpReceiveAddr, sizeof(udpReceiveAddr)) == -1) {
        perror("UDP bind failed");
        close(udpSocket);
        exit(EXIT_FAILURE);
    }

    printf("Listening for UDP messages...\n");

    // Receive UDP message to get client address
    char udpBuffer[BUF_SIZE];
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    if (recvfrom(udpSocket, udpBuffer, sizeof(udpBuffer), 0, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1) {
        perror("UDP receive failed");
        close(udpSocket);
        exit(EXIT_FAILURE);
    }

    udpBuffer[BUF_SIZE - 1] = '\0';

    // Get client IP and port from the received UDP message
    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
    int clientPort = ntohs(clientAddr.sin_port);

    printf("Received UDP message from %s:%d\n", clientIP, clientPort);

    // Get the number of CPU cores
    numCore = sysconf(_SC_NPROCESSORS_ONLN);

    // UDP socket for sending UDP response
    int udpSendSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSendSocket == -1) {
        perror("UDP send socket creation failed");
        close(udpSocket);
        exit(EXIT_FAILURE);
    }

    reuse = 1;
    if (setsockopt(udpSendSocket, SOL_SOCKET, SO_REUSEADDR | SO_BROADCAST, &reuse, sizeof(reuse)) == -1) {
        perror("Setsockopt failed");
        close(udpSendSocket);
        exit(EXIT_FAILURE);
    } 

    struct sockaddr_in udpSendAddr;
    memset(&udpSendAddr, 0, sizeof(udpSendAddr));
    udpSendAddr.sin_family = AF_INET;
    udpSendAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, clientIP, &udpSendAddr.sin_addr);

    sleep(1);
    // Send the number of CPU cores back through UDP
    char coresBuffer[16];
    snprintf(coresBuffer, sizeof(coresBuffer), "%d", numCore);
    if (sendto(udpSendSocket, coresBuffer, strlen(coresBuffer), MSG_CONFIRM, (struct sockaddr *)&udpSendAddr, sizeof(udpSendAddr)) == -1) {
        perror("UDP send failed");
        close(udpSendSocket);
        close(udpSocket);
        exit(EXIT_FAILURE);
    }

    printf("Sent number of cores: %d through UDP\n", numCore);

    close(udpSendSocket);
    close(udpSocket);

    // TCP socket for accepting TCP connections from the system
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == -1) {
        perror("TCP socket creation failed");
        exit(EXIT_FAILURE);
    }

    reuse = 1;
    if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR | SO_BROADCAST | SO_REUSEPORT, &reuse, sizeof(reuse)) == -1) {
        perror("Setsockopt failed");
        close(listenSocket);
        exit(EXIT_FAILURE);
    } 

    struct sockaddr_in slaveAddr;
    memset(&slaveAddr, 0, sizeof(slaveAddr));
    slaveAddr.sin_family = AF_INET;
    slaveAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    slaveAddr.sin_port = htons(PORT);

    if (bind(listenSocket, (struct sockaddr *)&slaveAddr, sizeof(slaveAddr)) == -1) {
        perror("TCP bind failed");
        close(listenSocket);
        exit(EXIT_FAILURE);
    }

    if (listen(listenSocket, 5) == -1) {
        perror("Listen failed");
        close(listenSocket);
        exit(EXIT_FAILURE);
    }

    printf("Slave is now listening for TCP connections...\n");
    
    struct sockaddr_in clientTcpAddr;
    socklen_t clientTcpAddrLen = sizeof(clientTcpAddr);

    int clientSocket = accept(listenSocket, (struct sockaddr *)&clientTcpAddr, &clientTcpAddrLen);
    if (clientSocket == -1) {
        perror("Accept failed");
        close(listenSocket);
        exit(EXIT_FAILURE);
    }

    reuse = 1;
    if (setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR | SO_BROADCAST, &reuse, sizeof(reuse)) == -1) {
        perror("Setsockopt failed");
        close(clientSocket);
        exit(EXIT_FAILURE);
    } 

    // Handle the TCP connection (For demonstration, simply print client information)
    char clientTcpIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientTcpAddr.sin_addr, clientTcpIP, INET_ADDRSTRLEN);
    int clientTcpPort = ntohs(clientTcpAddr.sin_port);
    sleep(2);
    char buff[BUF_SIZE];
    int len = read(clientSocket, buff, sizeof(buff));
    buff[len] = '\0';

    printf("Accepted TCP connection from %s:%d\n", clientTcpIP, clientTcpPort);
    printf ("Message from server %s", buff);
    struct Task task = {0};

    sscanf(buff, "%lf %lf %lf %lf %lld", &task.x1, &task.y1, &task.x2, &task.y2, &task.num_of_points);
    integral(&task);
    write(clientSocket, &square, sizeof(double));

    close(clientSocket);
    
    close(listenSocket);
    return 0;
}

unsigned int seed = 0;

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

double func(double x)
{
    return x;   
}

typedef struct Thread_args_
{
    double  low_x;
    double  step_x;
    double  low_y;
    double  step_y;
    long    numCore;
    long long unsigned num_points;
} Thread_args;

void* thread_func(void *data)
{
    Thread_args *thread_args = (Thread_args*) data;
    
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(thread_args->numCore, &cpuset);
    sched_setaffinity(0, sizeof(cpuset), &cpuset);
    
    register double x, y, thread_square = 0;
    const double    thread_low_x = thread_args->low_x,
                    thread_low_y = thread_args->low_y,
                    thread_step_x = thread_args->step_x,
                    thread_step_y = thread_args->step_y;
    srand((unsigned) time(NULL));
    for (long long unsigned i = 0; i < thread_args->num_points; i++)
    {
        x = thread_low_x + (double) rand_r(&seed) / RAND_MAX * (thread_step_x);
        y = thread_low_y + (double) rand_r(&seed) / RAND_MAX * (thread_step_y);

        if (y < func(x)) 
            thread_square += 1;
    }
    pthread_mutex_lock(&m);
    square += thread_square;
    pthread_mutex_unlock(&m);    
    return NULL;
}


void integral(struct Task *task)
{
    const double step_y = (task->y2 - task->y1) / numCore;

    pthread_t   *thread = (pthread_t*) calloc(numCore, sizeof(pthread_t));
    Thread_args  *thread_args = (Thread_args*) calloc(numCore, sizeof(Thread_args));

    for (unsigned i = 0; i < numCore; i++)
    {
        thread_args[i].low_x     = task->x1;
        thread_args[i].step_x    = task->x2 - task->x1;
        thread_args[i].low_y     = task->y1 + step_y * (i / numCore);
        thread_args[i].step_y    = step_y;
        thread_args[i].numCore   = 1 << (i % numCore);
        thread_args[i].num_points = task->num_of_points / numCore;
    }

    seed = (unsigned) time(NULL);

    for(long long unsigned i = 0; i < numCore; i++)
    {
        pthread_create(&thread[i], NULL, thread_func, thread_args + i);
    }

    for (long long unsigned i = 0; i < numCore; i++)
        pthread_join(thread[i], NULL);

    square /= task->num_of_points;

    return;
}