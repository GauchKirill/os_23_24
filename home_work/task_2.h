#ifndef TASK_2_H
#define TASK_2_H

typedef struct Pipe_ Pipe;

typedef struct DuplexPipe_
{
    Pipe *parent;
    Pipe *child;
} DuplexPipe;

DuplexPipe *ctor_dup_pipe();
void dtor_dup_pipe(DuplexPipe*);
void send_msg(char*, DuplexPipe*);

#endif