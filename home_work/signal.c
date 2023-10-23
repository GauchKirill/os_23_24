#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

FILE *input = NULL;
FILE *output = NULL;

void sig_handler(int signo, siginfo_t *info, void *context)
{
    switch (signo)
    {
    case :
        /* code */
        break;
    
    default:
        break;
    }
}

void sender(int child_pid)
{
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = sig_handler;

    while (1)
    {
        pause();
    }
    
    return;
}

void resiver(int parent_pid)
{
    while (1)
    {
        pause();
    }
    return;
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        printf("Enter input and outpul files\n");
        return 0;
    }

    input = fopen(argv[1], "r");
    output = fopen(argv[2], "w");
    pid_t pid = fork();

    if (pid < 0)
        perror("fork()");
    else
    if (pid)
        sender();
    else
        resiver();

    close(input);
    close(output);
    return 0;
}