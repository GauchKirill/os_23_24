#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

void daemon_routine(int fd)
{
    write(fd, "msg", sizeof("msg"));
    return 0;
}

int main(int argc, const char* argv[])
{
    pid_t pid;
    int status;

    if (argc < 2)
        return -1;
    int fd = open("demon_log.txt", O_CREAT | O_RDWR | O_APPEND | 0666);
    if (strcmp(argv[1], "-d") == 0)
    {
        pid = fork();
        if (pid == -1)
        {
            return -1;
        }
        else
        if (pid == 0)
        {
            // /proc/self, self - текущий процесс. Тут его статистика
            setsid();
            printf("%d\n", getpid());
            close(0);
            close(1);
            close(2);
            // chdir("/");
            // umask(0);

            while (1)
            {
                sleep(1);
                daemon_routine(fd);
            }
        }
    }
    else
    {
        while (1)
        {
            sleep(1);
            daemon_routine(fd);
        }        
    }
    

    return 0;
}