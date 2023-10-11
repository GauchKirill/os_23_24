#include <aio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <wait.h>
#include "task_2.h"

#define BUF_SZ 4096

size_t send_msg(Pipe *self);
size_t receive_msg(Pipe *self);

void pipe_ctor(Pipe *ppipe)
{
    ppipe->len = 0;
    ppipe->data = (char*) calloc(BUF_SZ, sizeof(char));
    pipe(ppipe->fd_direct);
    pipe(ppipe->fd_back);

    ppipe->actions.rcv = receive_msg;
    ppipe->actions.snd = send_msg;    
}


void pipe_dtor(Pipe *ppipe)
{
    if (!ppipe || !ppipe->data) return;
    free(ppipe->data);
    ppipe->actions.rcv = ppipe->actions.snd = NULL;
    ppipe->len = 0;
    return;
}

size_t send_msg(Pipe *self)
{
    if (!self || !self->data) return 0;
    size_t wrote = write(self->fd_direct[1], self->data, self->len);
    self->len -= wrote;
    return wrote;
}

size_t receive_msg(Pipe *self)
{
    if (!self || !self->data) return 0;
    size_t cnt_read = 0;
    self->len = read(self->fd_back[0], self->data, BUF_SZ);
    return self->len;
}

void parent_process(Pipe* ppipe, FILE* input, FILE* output)
{
    close(ppipe->fd_direct[0]);
    close(ppipe->fd_back[1]);
    size_t cnt_read = 0;
    while (1)
    {
        cnt_read = fread(ppipe->data, sizeof(char), BUF_SZ, input);
        ppipe->len = cnt_read;
        if (cnt_read)
            ppipe->actions.snd(ppipe);
        else
            close(ppipe->fd_direct[1]);

        if(ppipe->actions.rcv(ppipe) == 0 && cnt_read == 0)
            break;
        else
            fwrite(ppipe->data, sizeof(char), ppipe->len, output);
    }
    close(ppipe->fd_back[0]);
    wait(NULL);
    fclose(input);
    fclose(output);
    exit(0);
}

void child_process(Pipe* ppipe)
{
    close(ppipe->fd_direct[0]);
    close(ppipe->fd_back[1]);
    while (ppipe->actions.rcv(ppipe))
        while (ppipe->actions.snd(ppipe));

    close(ppipe->fd_back[0]);
    close(ppipe->fd_direct[1]);
    exit(0);    
}