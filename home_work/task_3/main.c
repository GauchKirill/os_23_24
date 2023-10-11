#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <wait.h>
#include <sys/msg.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <semaphore.h>

#define SHM

#define BUF_SZ 4096

#if defined(FIFO)

#define FIFO_NAME "fifo_stream"

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

#elif defined(QUEUE)

#define KEY 10
#define TYPE 1

typedef struct msgbuf {
    long    mtype;
    char    mtext[BUF_SZ];
} message_buf;

int queue_way(FILE* input, FILE* output)
{
    int msqid;
    key_t key = KEY;
    message_buf msg_buf;
    msg_buf.mtype = TYPE;

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork()");
        return 1;
    }
    else
    if (pid)
    {
        if ((msqid = msgget(key, IPC_CREAT | 0666)) < 0) {
            perror("msgget");
            exit(1);
        }

        size_t cnt_read = 0;
        while(cnt_read = fread(msg_buf.mtext, sizeof(char), BUF_SZ - 1, input)) {
            msg_buf.mtext[cnt_read] = '\0';
            size_t buf_len = cnt_read + 1;
            msgsnd(msqid, &msg_buf, buf_len, 0);
        }

        strcpy(msg_buf.mtext, "q");
        size_t buf_len = strlen(msg_buf.mtext) + 1;
        msgsnd(msqid, &msg_buf, buf_len, 0);

        int status;
        waitpid(pid, &status, 0);
        if (!WEXITSTATUS(status))
            return 3;
        return 0;
    }
    else
    {
        if ((msqid = msgget(key, 0666)) < 0) {
            perror("msgget");
            exit(1);
        }

        while(1) {
            if (msgrcv(msqid, &msg_buf, BUF_SZ, TYPE, 0) < 0) {
                continue;
            }
            msg_buf.mtext[BUF_SZ - 1] = '\0';
            if (strcmp(msg_buf.mtext, "q") == 0) {
                break;
            }
            size_t len = strlen(msg_buf.mtext);
            fwrite(msg_buf.mtext, sizeof(char), len, output);
        }
        return 0;
    }
    
}

#elif defined(SHM)

int shm_way(FILE* input, FILE* output)
{
    sem_t *sem;
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork()");
        return 1;
    }
    if (pid)
    {

    }
    else
    {
        
    }

}

#endif


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

    #if defined(FIFO)

        mknod(FIFO_NAME, S_IFIFO | 0666, 0);

    #elif defined(QUEUE)

    #elif defined(SHM)

    #endif

    struct timeval tv1, tv2, dtv;

    gettimeofday(&tv1, NULL);

    #if defined(FIFO)
        if (fifo_way(input, output))
            return 0;
    #elif defined(QUEUE)
        if (queue_way(input, output))
            return 0;
    #elif defined(SHM)
        if (shm_way(input, output))
            return 0;
    #endif

    gettimeofday(&tv2, NULL);

    fclose(input);
    fclose(output);

    #if  defined(FIFO)
        printf("fifo ran: ");
    #elif defined(QUEUE)
        printf("queue ran: ");
    #elif defined(SHM)
        printf("map ran: ");
    #endif

    dtv.tv_usec = tv2.tv_usec - tv1.tv_usec;
    dtv.tv_sec = tv2.tv_sec - tv1.tv_sec;
    printf("%ld us\n", dtv.tv_usec + 1000000 * dtv.tv_sec);

    return 0;
}