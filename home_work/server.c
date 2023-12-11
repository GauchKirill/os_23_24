#include <stdio.h>
#include <unistd.h>
#include "server.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <errno.h>
#include <string.h>

#define BUF_SZ 4096
#define STR_SIZE 200

int main()
{
    Server server;
    server.clients_num = 0;
    mknod(reg_fifo, __S_IFIFO | 0666, 0);
    mknod(ans_fifo, __S_IFIFO | 0666, 0);
    server.ans_fd = open(ans_fifo, O_RDWR);
    server.reg_fd = open(reg_fifo, O_RDWR);

    struct pollfd fds[MAX_NUM_CLIENTS + 1];
    while (1)
    {
        fds[0].fd = server.reg_fd;
        fds[0].events = POLLIN;
        for (size_t curr_cli = 0; curr_cli < server.clients_num; curr_cli++)
        {
            fds[curr_cli + 1].fd = server.clients[curr_cli].fd_in;
            fds[curr_cli + 1].events = POLLIN;
        }
        int res = poll(fds, server.clients_num + 1, -1);
        if (res < 0)
        {
            perror("poll");
        }
        else
        if (res)
        {
            if (server.clients_num > 0)
            {
                for (size_t curr_cli = 0; curr_cli < server.clients_num; curr_cli++)
                {
                    if (fds[curr_cli + 1].revents & POLLIN)
                    {
                        pthread_create(&server.cli_threads[curr_cli], NULL, client_process, &server.clients[curr_cli]);
                    }
                }
            }
            if (fds[0].revents & POLLIN)
            {
                pthread_create(&server.serv_thread, NULL, registration, &server);
            }
            pthread_join(server.serv_thread, NULL);
        }
    }

    return 0;
}

void* registration(void *data)
{
    Server *server = (Server*) data;
    char buf[BUF_SZ];
    read(server->reg_fd, buf, BUF_SZ);
    char cmd[STR_SIZE]= "";
    char fifo_in[STR_SIZE] = "";
    char fifo_out[STR_SIZE] = "";
    char ans[STR_SIZE] = "";
    
    sscanf(buf, "%s %s %s", cmd, fifo_in, fifo_out);

    if (strncmp(cmd, "REGISTER", STR_SIZE) == 0)
    {
        int fd_in = open(fifo_in, O_RDWR);
        int fd_out = open(fifo_out, O_RDWR);
        if (fd_in == -1 || fd_out == -1)
        {
            sprintf(ans, "%s", "REGISTER_FAILED");
        }
        else
        {
            server->clients[server->clients_num].fd_in = fd_in;
            server->clients[server->clients_num].fd_out = fd_out;
            server->clients_num++;
            sprintf(ans, "%s", "ASK");
        }
    }
    else
    {
        sprintf(ans, "%s", "INVALID_ARGUMENTS");
    }
    write(server->ans_fd, ans, sizeof(ans));

    return NULL;
}

void* client_process(void *data)
{
    Client *client = (Client*) data;
    char buf[BUF_SZ] = "";
    read(client->fd_in, buf, BUF_SZ);
    
    char cmd[STR_SIZE] = "";
    char file_name[STR_SIZE] = "";
    sscanf(buf, "%s %s", cmd, file_name);

    if (strncmp(cmd, "GET", STR_SIZE) == 0)
    {
        int status = send(file_name, client->fd_out);
        if (status == -1) sprintf(buf, "CANNOT_OPEN_FILE");
        else return NULL;
    }
    else
    {
        sprintf(buf, "INVALID_ARGUMENTS");
    }
    write(client->fd_out, buf, sizeof(buf));
    return NULL;
}

int send(const char* file_name, const int fd_out)
{
    int fd_in = open(file_name, O_RDWR);
    if (fd_in == -1) return -1;

    char buf[BUF_SZ] = "";
    size_t size = 0;
    while ((size = read(fd_in, buf, BUF_SZ)) > 0)
    {
        write(fd_out, buf, size);
    }
    
    return 0;
}