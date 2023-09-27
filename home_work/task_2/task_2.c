#include <stdio.h>
#include <stdlib.h>
#include "task_2.h"

int main()
{
    DuplexPipe *dup_pipe = ctor_dup_pipe();
    
    size_t n = 0;
    char *s = NULL;
    getline(&s, &n, stdin);
    send_msg(s, dup_pipe);
    free(s);
    dtor_dup_pipe(dup_pipe);
}