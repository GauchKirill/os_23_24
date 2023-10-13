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

#define BUF_SZ 4095

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
        
        close(fd);
        exit(0);
    }
}

#elif defined(QUEUE)

#define KEY 10
#define TYPE 1

#define exit_word "q"

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

        strcpy(msg_buf.mtext, exit_word);
        size_t buf_len = strlen(msg_buf.mtext) + 1;
        msgsnd(msqid, &msg_buf, buf_len, 0);

        int status;
        waitpid(pid, &status, 0);
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
            if (strcmp(msg_buf.mtext, exit_word) == 0) {
                break;
            }
            size_t len = strlen(msg_buf.mtext);
            fwrite(msg_buf.mtext, sizeof(char), len, output);
        }
        exit(0);
    }
    
}

#elif defined(SHM)

#define KEY 10
#define NAME "sem"

int shm_way(FILE* input, FILE* output)
{
    sem_t* sem;
    sem = sem_open(NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 1);

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork()");
        return 1;
    }
    if (pid)
    {
        int res = shmget(KEY, BUF_SZ, 0644 | IPC_CREAT);
        if (res == -1) {
            perror("shmget");
            return 1;
        }
        char data[BUF_SZ];
        void* ptr = shmat(res, NULL, 0);

        while (1) {
            if (((char*) ptr)[BUF_SZ] == '*')
            {
                size_t num = 0;
                num = fread(data, sizeof(char), BUF_SZ - 1, input);
                data[num] = '\0';
                sprintf((char*) ptr, "%s", data);
                ((char*) ptr)[BUF_SZ] = '\0';
                if (num == 0) {
                    break;
                }
            }
        }

        shmdt(ptr);
        shmctl(KEY, IPC_RMID, NULL);

        int status;
        waitpid(pid, &status, 0);
        return 0;
    }
    else
    {
        int res = shmget(KEY, BUF_SZ, 0644 | IPC_CREAT);
        if (res == -1) {
            perror("shmget");
            return 1;
        }
        void* ptr = shmat(res, NULL, 0);

        ((char*) ptr)[BUF_SZ] = '*';

        while (1) {
            if (((char*) ptr)[BUF_SZ] == '\0')
            {
                size_t lenght = strlen((char*) ptr);
                if (lenght)
                {
                    fwrite((char*)ptr, sizeof(char), lenght, output);
                    ((char*) ptr)[BUF_SZ] = '*';
                }
                else
                    break;
            }
        }

        shmdt(ptr);
        shmctl(KEY, IPC_RMID, NULL);
        exit(0);
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

    #if  defined(FIFO)
        FILE* res_file = fopen("data/fifo_res.txt", "a");
    #elif defined(QUEUE)
        FILE* res_file = fopen("data/queue_res.txt", "a");
    #elif defined(SHM)
        FILE* res_file = fopen("data/shm_res.txt", "a");
    #endif

    dtv.tv_usec = tv2.tv_usec - tv1.tv_usec;
    dtv.tv_sec = tv2.tv_sec - tv1.tv_sec;
    fprintf(res_file, "%ld\n", dtv.tv_usec + 1000000 * dtv.tv_sec);

    fclose(input);
    fclose(output);
    fclose(res_file);
    return 0;
}