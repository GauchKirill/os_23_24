#include <stdio.h>
#include <stdlib.h>
#include "server.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <sys/poll.h>

#define CMD_LEN 500
#define FILE_NAME 200

#define BUF_SZ 4096

int get_file(Client, const char*);

int main(int argc, const char** argv)
{
    if (argc != 2)
    {
        printf("Specify the name of the client in the arguments:\nclient.out <client's name>\n");
        return 0;
    }
    const char* cli_name = argv[1];
    char cmd[CMD_LEN] = "";
    char cli_dir[FILE_NAME] = "";
    sprintf(cli_dir, "client_%s", cli_name);
    sprintf(cmd, "mkdir %s", cli_dir);
    system(cmd);

    int fd_out = open(reg_fifo, O_RDWR | O_APPEND);
    int fd_in = open(ans_fifo, O_RDWR | O_APPEND);

    if (fd_in == -1 || fd_out == -1)
    {
        printf("Cannot open %s or %s\n", reg_fifo, ans_fifo);
        return 0;
    }

    char fifo_out[FILE_NAME] = "";
    sprintf(fifo_out, "%s_in.fifo", cli_name);
    char fifo_in[FILE_NAME] = "";
    sprintf(fifo_in, "%s_out.fifo", cli_name);

    mknod(fifo_in, __S_IFIFO | 0666, 0);
    mknod(fifo_out, __S_IFIFO | 0666, 0);
    
    char buf[BUF_SZ] = "";
    sprintf(buf, " REGISTER %s %s", fifo_out, fifo_in);
    write(fd_out, buf, sizeof(buf));

    size_t size = read(fd_in, buf, BUF_SZ - 1);
    if (strncmp(buf, "ASK", 4) != 0)
    {
        buf[size] = '\n';
        write(0, buf, size + 1);
        close(fd_in);
        close(fd_out);    
        return 0;
    }
    close(fd_in);
    close(fd_out);
    Client client;
    client.fd_in = open(fifo_in, O_RDWR | O_TRUNC);
    client.fd_out = open(fifo_out, O_RDWR | O_TRUNC);

    if (client.fd_in == -1 || client.fd_out == -1)
    {
        printf("Cannot open %s or %s\n", fifo_in, fifo_out);
        return 0;
    }

    while (get_file(client, cli_dir) == 1);

    sprintf(cmd, "rm -r %s", cli_dir);
    system(cmd);
    sprintf(cmd, "rm %s", fifo_in);
    system(cmd);
    sprintf(cmd, "rm %s", fifo_out);
    system(cmd);
    
    close(client.fd_in);
    close(client.fd_out);
    return 0;
}

int get_file(Client client, const char *cli_dir)
{
    printf("Which file receive (enter \"q\" to exit)?\n");
    char file_name[FILE_NAME] = "";
    scanf("%s", file_name);
    
    if (strncmp(file_name, "q", 2) == 0)
        return 0;

    char buf[BUF_SZ] = "";
    sprintf(buf, "GET %s", file_name);
    write(client.fd_out, buf, sizeof(buf));

    char end_file_name[2*FILE_NAME] = "";
    sprintf(end_file_name, "%s/%s", cli_dir, file_name);
    int fd = open(end_file_name, O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (fd == -1)
    {
        printf("Cannot open %s\n", end_file_name);
        return 1;
    }

    size_t size = 0;
    struct pollfd poll_fd;
    poll_fd.fd = client.fd_in;
    poll_fd.events = POLLIN;
    while(1)
    {
        int res = poll(&poll_fd, 1, 1);
        if (res == 0)
            break;
        
        size = read(client.fd_in, buf, BUF_SZ);
        write(fd, buf, size);
    }
    return 1;
}