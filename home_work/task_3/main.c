#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <wait.h>

#define FIFO_NAME "fifo_stream"

#define BUF_SZ 4096



int fifo_way(FILE* input, FILE* output)
{
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return 1;
    }
    else
    if (pid)
    {
        int fd = open(FIFO_NAME, O_WRONLY);

        if (!fd) return 2;
        
        size_t size = 0;
        char buf[BUF_SZ] = "";
        while ((size = fread(buf, sizeof(char), BUF_SZ, input)) > 0)
            write(fd, buf, size);

        close(fd);
        int status;
        waitpid(pid, &status, 0);
        if (!WEXITSTATUS(status))
            return 3;
        return 0;
    }
    else
    {
        int fd = open(FIFO_NAME, O_RDONLY);
        if (!fd) exit(1);

        size_t size = 0;
        char buf[BUF_SZ] = "";
        do
        {
            size = read(fd, buf, BUF_SZ);
            fwrite(buf, sizeof(char), size, output);
        } while (size == BUF_SZ);
        
        return 0;
    }
}



int main(int argc, char** argv)
{
    if (argc != 3)
    {
        printf("Enter 2 name files in args. First - input file, second - ontput file\n");
        return 0;
    }
    FILE *input = fopen(argv[1], "r");
    FILE *output = fopen(argv[2], "w");
    
    if (!input || ! output) return 0;

    mknod(FIFO_NAME, S_IFIFO | 0666, 0);


    struct timeval tv1, tv2, dtv;

    gettimeofday(&tv1, NULL);

    #if defined(SHM)
        if (shm_way(input, output))
            return 0;
    #elif defined(QUEUE)
        if (queue_way(input, output))
            return 0;
    #elif defined(FIFO)
        if (fifo_way(input, output))
            return 0;
    #endif

    gettimeofday(&tv2, NULL);

    fclose(input);
    fclose(output);

    #ifdef defined(SHM)
        printf("map ran: ");
    #elif defined(QUEUE)
        printf("queue ran: ");
    #elif defined(FIFO)
        printf("fifo ran: ");
    #endif

    dtv.tv_usec = tv2.tv_usec - tv1.tv_usec;
    dtv.tv_sec = tv2.tv_sec - tv1.tv_sec;
    printf("%ld us\n", dtv.tv_usec + 1000000 * dtv.tv_sec);

    return 0;
}