#include <stdio.h>
#include <aio.h>
#include <unistd.h>
#include "task_2.h"

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        printf("Enter 2 name files in args. First - input file, second - ontput file\n");
        return 0;
    }
    FILE    *input  = fopen(argv[1], "r"),
            *output = fopen(argv[2], "w");
    
    if (!input || ! output) return 1;

    Pipe ppipe = {};
    pipe_ctor(&ppipe);

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");       
        return 0;
    }
    else
    if (pid)
        parent_process(&ppipe, input, output);
    else
    {
        for (int i = 0, temp; i < 2; i++)
        {
            temp = ppipe.fd_direct[i];
            ppipe.fd_direct[i] = ppipe.fd_back[i];
            ppipe.fd_back[i] = temp;
        }
        child_process(&ppipe);
    }
}