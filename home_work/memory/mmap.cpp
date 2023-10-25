#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define PAGESZ 4096

int main(int argc, const char* argv[])
{
    if (argc < 2)
        return 1;

    write(1, "before", sizeof("before") + 1);
    char* p = (char*) malloc(PAGESZ*atoi(argv[1])); //при большых размерах вместе brk mmap
    write(1, "after", sizeof("after") + 1);

    free(p); //при больших munmap вместо brk
}