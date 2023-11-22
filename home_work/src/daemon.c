#include "daemon.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <signal.h>

int settings_process(int);

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

int MonitorProc(int pid, const char* work_dir);

int daemon(int argc, const char** argv)
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
       MonitorProc(pid, work_dir);
       return 0; 
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

        return MonitorProc(pid, work_dir);
    }
    return;
}

void handler(int signo, siginfo_t *info, void *context)
{
    switch(signo)
    {
        case SIGUSR1:
            period = info->si_value.sigval_int;
            break;
        case SIGUSR2:
            exit_val = 1;
            break;
    }
    return;
}

int MonitorProc(int pid, const char* work_dir)
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

    while (1)
    {
        sleep(period);
        if (exit_val)
        {

        }

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
            return 0;
        }
        else
        {            
            val.sival_int = period;
            sigqueue(child_pid, SIGUSR1, val);
        }
    }
}