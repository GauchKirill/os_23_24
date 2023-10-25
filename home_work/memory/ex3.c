#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

int main(int c, const char* argv[])
{
    char *f;
    int sz;
    struct stat statinfo;    
    int fd = open(argv[1], O_RDONLY);
    fstat(fd, &statinfo);
    sz = statinfo.st_size;
    int i = 0;
    f = (char*) mmap(0, sz, PROT_READ, MAP_PRIVATE, fd, 0);
    for (; i < sz; i++)
    {
        printf("%c", f[i]);
    }
    munmap(f, sz);
    return 0;
}