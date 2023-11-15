#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

int main(int argc, const char *argv[])
{
    long int size = 0;
    struct stat st;

    if (stat(argv[1], &st) == 0)
        printf("%lld\n", st.st_size);
    else
        perror("Error is stat()");

    return 0;
}