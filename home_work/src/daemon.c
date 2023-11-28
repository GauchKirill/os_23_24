#include "daemon.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <signal.h>
#include <linux/limits.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/inotify.h>

int settings_process(int);
int MonitorProc(int, const char*);
void watch_directory(const char *name, const char* dump);
void search_directory(const char* name, const char* dump);

static unsigned period = 1;
static unsigned exit_val = 0;

#define BUF_SZ 100
#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUF_LEN     (1024 * (EVENT_SIZE + 16))

enum modes
{
    INTERACTIVE_MODE,
    DAEMON_MODE,
};

void help()
{
    printf(
    "Rules for submitting arguments:\n"
    "PID, work_dir, *flag\n"
    "* -- optional argument\n"
    "flags:\n"
    "-i -- interactive mode\n"
    "-d -- launch daemon\n");
    return;
}

void SetPidFile(char* filename)
{
    FILE* file = NULL;

    file = fopen(filename, "w+");
    if (file)
    {
        fprintf(file, "%u", getpid());
        fclose(file);
    }
}

int run_daemon(int argc, const char** argv)
{
    if (argc < 3 || argc > 4)
    {
        help();
        return -1;
    }

    int pid = atoi(argv[1]);
    unsigned MODE = INTERACTIVE_MODE;

    if (argc == 4 && strncmp(argv[3], "-d", 3) == 0)
        MODE = DAEMON_MODE;
    
    if (MODE == INTERACTIVE_MODE)
    {
       return MonitorProc(pid, argv[2]);
    }
    else
    {
        int daemon_pid = fork();

        if (daemon_pid < 0)
        {
            perror("fork");
            return -1;
        }
        if (daemon_pid)    
            return 0;
        
        umask(0);
        setsid();
        SetPidFile("this_daemon.pid");
        chdir("/");

        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        return MonitorProc(pid, argv[2]);
    }
    return 0;
}

void handler(int signo, siginfo_t *info, void *context)
{
    switch(signo)
    {
        case SIGUSR1:
            period = info->si_value.sival_int;
            break;
        case SIGUSR2:
            exit_val = 1;
            break;
    }
    return;
}

int MonitorProc(int pid, const char* dump)
{
    int new_pid = fork();
    if (new_pid < 0)
    {
        perror("fork");
        return -1;
    }

    if (new_pid == 0)
        return settings_process(getppid());
    
    union sigval val;
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);

    char pathname[PATH_MAX];
    sprintf(pathname, "/proc/%d/cwd/", pid);
    char dump_dir[PATH_MAX];
    sprintf(dump_dir, "%s/%d_dump", dump, pid);

    while (1)
    {
        sleep(period);
        if (exit_val)
        {
            return 0;
        }

        watch_directory(pathname, dump_dir);
    }
    return 0;
}

int settings_process(int reciever_pid)
{
    int file_fd = open("daemon.cfg", O_CREAT | O_RDONLY);
    if (file_fd < 0)
    {
        kill(reciever_pid, SIGKILL);
        return -1;
    }

    union sigval val;
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;

    char buf[BUF_SZ] = {0};
    while (1)
    {
        while (read(file_fd, buf, sizeof(int)) == 0);
        if (sscanf(buf, "%d", &period) == 0)
        {
            kill(reciever_pid, SIGUSR2);
            break;   
        }
        val.sival_int = period;
        sigqueue(reciever_pid, SIGUSR1, val);
    }
    return 0;
}

void watch_directory(const char *name, const char* dump)
{
    search_directory(name, dump);
    size_t length = 0, i = 0;
    int fd, wd;
    char event_buffer[BUF_LEN];

    fd = inotify_init();
    if (fd < 0)
    {
        perror("inotify_init");
    }

    wd = inotify_add_watch(fd, ".",
        IN_MODIFY | IN_CREATE | IN_DELETE);
    length = read(fd, event_buffer, BUF_LEN);

    if (length < 0) {
        perror("read");
    }

    while (i < length) {
        struct inotify_event *event =
            (struct inotify_event *) &event_buffer[i];
        if (event->len) {
            if (event->mask & IN_CREATE) {
                printf("The file %s was created.\n", event->name);
            } else if (event->mask & IN_DELETE) {
                printf("The file %s was deleted.\n", event->name);
            } else if (event->mask & IN_MODIFY) {
                printf("The file %s was modified.\n", event->name);
            }
        }
        i += EVENT_SIZE + event->len;
    }

    (void) inotify_rm_watch(fd, wd);
    (void) close(fd);

    return 0;
}

void search_directory(const char* name, const char* dump)
{
    DIR* dir = opendir(name);

    if (dir)
    {
        
    }

    return;
}