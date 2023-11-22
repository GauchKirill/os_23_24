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

int settings_process(int);
int MonitorProc(int);
void search_directory(const char *name, const char *mnt_dir);

static unsigned period = 1000;
static unsigned exit_val = 0;

enum modes
{
    INTERACTIVE_MODE = 1 << 0,
};

void help()
{
    printf(
    "Rules for submitting arguments:\n"
    "PID, WorkDirectory *flag\n"
    "* -- optional argument\n"
    "flags:\n"
    "-i -- interactive mode\n");
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
    const char* work_dir = argv[2];
    unsigned MODE = 0;

    if (argc == 4 && strncmp(argv[2], "-i", 3) == 0)
        MODE |= INTERACTIVE_MODE;
    
    if (MODE & INTERACTIVE_MODE)
    {
       return MonitorProc(pid);
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

        return MonitorProc(pid);
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

int MonitorProc(int pid)
{
    int new_pid = fork();

    if (new_pid < 0)
    {
        perror("fork");
        return -1;
    }

    if (new_pid)
        return settings_process(new_pid);
    
    union sigval val;
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);

    char cwd[PATH_MAX];
    sprintf(cwd, "/proc/%d/cwd/", pid);
    
    while (1)
    {
        sleep(period);
        if (exit_val)
        {
            return 0;
        }

        search_directory(cwd);
    }
}

int settings_process(int child_pid)
{
    FILE* settigs_file = fopen("daemon.cfg", "r+");
    if (!settigs_file)
    {
        kill(child_pid, SIGKILL);
        return -1;
    }

    union sigval val;
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;

    while (1)
    {
        while (fread(&period, sizeof(int), 1, settigs_file) == 0);
        
        if (period == 0)
        {
            kill(child_pid, SIGUSR2);
            fclose(settigs_file);
            return 0;
        }
        else
        {            
            val.sival_int = period;
            sigqueue(child_pid, SIGUSR1, val);
        }
    }
}

void search_directory(const char *name, const char *mnt_name)
{
    DIR *cur_dir = opendir(name);
    if (cur_dir)
    {
        char path[PATH_MAX], *end_ptr = path;
        struct stat info;
        struct dirent *dir;
        strcpy(path, name);
        end_ptr += strlen(name);
        while ((dir = readdir(cur_dir)) != NULL) {
            strcpy(end_ptr, dir -> d_name);

            if (!stat(path, &info)) { 
                if (S_ISDIR(info.st_mode)) {
                    if (!strcmp(dir -> d_name, ".." ) == 0 || !strcmp(dir -> d_name, "." ) == 0)
                        continue;

                    search_directory(strcat(path,"/"));
                } else if (S_ISREG(info.st_mode)) {
                    printf("reg_file: %s\n", path);
                }
            } else {
                perror("stat");
            }
        }
    }
    return;
}