#include "daemon.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>

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

int MonitorProc(int pid, const char* work_dir)
{
    
}