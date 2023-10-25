#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, const char* argv[])
{
    if (argc != 3)
        return 1;

    int input_fd = open(argv[1], O_RDONLY);
    FILE* output_file = fopen(argv[2], "w+");

    struct stat statinfo;
    fstat(input_fd, &statinfo);

    size_t size = statinfo.st_size;

    char* buf = (char*) mmap(0, size, PROT_READ, MAP_PRIVATE, input_fd, 0);
    fwrite(buf, sizeof(char), size, output_file); 
    
    munmap(buf, size);
    return 0;
}