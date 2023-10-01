#include <stdio.h>

int main()
{
    #if defined(PIPE)
        printf ("pipe\n");
    #elif defined(FIFO)
        printf ("fifo\n");
    #elif defined(MMAP)
        printf("mmap\n");
    #endif
    return 0;
}