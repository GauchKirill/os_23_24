#ifndef SERVER_H
#define SERVER_H

#include <aio.h>
#include <pthread.h>

static const char *reg_fifo = "registration.fifo";
static const char *ans_fifo = "answer.fifo";
#define MAX_NUM_CLIENTS 64

typedef struct Client_
{
    int fd_in;
    int fd_out;
} Client;


typedef struct Server_{
    Client clients[MAX_NUM_CLIENTS];
    pthread_t cli_threads[MAX_NUM_CLIENTS];
    pthread_t serv_thread;
    size_t clients_num;
    int reg_fd;
    int ans_fd;
} Server;

void* registration(void *data);
void* client_process(void *data);
int send(const char* file_name, const int fd_out);
#endif