#ifndef TASK_2_H
#define TASK_2_H

typedef struct Pipe_ Pipe;

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

void pipe_ctor(Pipe*);
void pipe_dtor(Pipe*);
void parent_process(Pipe*, FILE*, FILE*);
void child_process(Pipe*);

#endif