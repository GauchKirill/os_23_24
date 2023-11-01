#include <sys/prctl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

int main()
{
    int res = prctl(PR_SET_CHILD_SUBREAPER, 1, 0, 0, 0); 
    if (res == -1)
        perror("prctl");
    
    pid_t pid = fork();
    printf("%d %d\n", getpid(), getppid());
    if (pid > 0)
        while(1) wait(NULL);
    else
    {
        pid_t pid_2 = fork();
        if (pid_2 > 0)
        {
            sleep(1);
            printf("Process %d: exit\n", getpid());
        }
        else
        {
            printf("Child: parent id is %d\n", getppid());
            sleep(2);
            printf("Child: parent id is %d\n", getppid());
        }
    }

    return 0;
}