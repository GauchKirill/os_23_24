#include <aio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <wait.h>
#include <time.h>
#include "task_2.h"

#define BUF_SZ 4096

size_t send_msg_(Pipe *self);
size_t receive_msg_(Pipe *self);

typedef struct op_table
{
    size_t (*rcv)(Pipe *self); 
    size_t (*snd)(Pipe *self); 
} Ops;

typedef struct Pipe_
{
        char* data;
        int fd_direct[2];
        int fd_back[2];
        size_t len;
        Ops actions;
} Pipe;

DuplexPipe *ctor_dup_pipe()
{
    DuplexPipe *duplex_pipe = (DuplexPipe*) calloc(1, sizeof(DuplexPipe));
    duplex_pipe->parent = (Pipe*) calloc(1, sizeof(Pipe));
    duplex_pipe->child  = (Pipe*) calloc(1, sizeof(Pipe));
    duplex_pipe->parent->len = duplex_pipe->child->len = 0;

    pipe(duplex_pipe->parent->fd_direct);
    pipe(duplex_pipe->child->fd_direct);

    for (int i = 0; i < 2; i++)
    {
        duplex_pipe->parent->fd_back[i] = duplex_pipe->child->fd_direct[i];
        duplex_pipe->child->fd_back[i]  = duplex_pipe->parent->fd_direct[i];
    }

    duplex_pipe->parent->actions.rcv = duplex_pipe->child->actions.rcv = receive_msg_;
    duplex_pipe->parent->actions.snd = duplex_pipe->child->actions.snd = send_msg_;

    return duplex_pipe;
}

void dtor_dup_pipe(DuplexPipe *dup_pipe)
{
    if (!dup_pipe) return;
    if (dup_pipe->parent)   free(dup_pipe->parent);
    if (dup_pipe->child)    free(dup_pipe->child);
    free(dup_pipe);
}

size_t send_msg_(Pipe *self)
{
    if (!self->data) return 0;
    printf("sending... %s\n", self->data);
    size_t cnt_wrote = 0;
    const char *buf = self->data;
    size_t lenght = self->len;
    while ((cnt_wrote += write(self->fd_direct[1], buf, lenght - cnt_wrote)) < lenght);
    printf("sent\n");
}

size_t receive_msg_(Pipe *self)
{
    printf("receiving...\n");
    self->data = (char*) calloc(BUF_SZ, sizeof(char));
    size_t cnt_read = 0;
    while ((cnt_read = read(self->fd_back[0], self->data + self->len, BUF_SZ)) > 0)
    {
        printf("read: %ld\n", cnt_read);
        self->len += cnt_read;
        self->data = (char*) realloc(self->data, self->len + BUF_SZ);
    }
    self->data = (char*) realloc(self->data, self->len);
    printf("received\n");
    return self->len;
}

void send_msg(char* data, DuplexPipe *dup_pipe)
{
    if (!data) return;

    dup_pipe->parent->data = data;
    dup_pipe->parent->len = strlen(data);

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");       
        return;
    }
    else
    if (pid)
    {
        dup_pipe->parent->actions.snd(dup_pipe->parent);
        sleep(1);
        dup_pipe->parent->actions.rcv(dup_pipe->parent);
        int status = 0;
        waitpid(pid, &status, 0);
        if (WEXITSTATUS(status))
        {
            printf("child %d fiald: exit code %d\n", pid, WEXITSTATUS(status));
            exit(1);
        }
        else
        {
            if(dup_pipe->parent->data)
            {
                fgets(dup_pipe->parent->data, dup_pipe->parent->len, stdout);
                free(dup_pipe->parent->data);
            }
        }
        exit(0);
    }
    else
    {
        dup_pipe->child->actions.rcv(dup_pipe->child);
        dup_pipe->child->actions.snd(dup_pipe->child);
        if (dup_pipe->child->data)  free(dup_pipe->child->data);
        exit(0);
    }

}