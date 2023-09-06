#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

const char*     exit_word   = "q"; 
const char*     delim       = " ";
#define         LENGTH_CMD  100

char** parse_str(char* str)
{
    unsigned cnt_ptr = 0;

    for (const char* p = str; p != NULL; p = strpbrk(++p, " "), cnt_ptr++);

    char** args = (char**) calloc(cnt_ptr+1, sizeof(char*));
    args[cnt_ptr] = NULL;
    unsigned cnt = 0;
    for (char* p = strtok(str, delim); p != NULL; p = strtok(NULL, delim), cnt++)
    {
        args[cnt] = (char*) calloc(strlen(p) + 1, sizeof(char));
        strcpy(args[cnt], p);
    }

    return args;
}

void run_cmd(char* str)
{
    int pid = fork();

    if (pid < 0)
    {
        perror("fork(): failed\n");
        return;
    }
    if (pid)
    {
        int status = 0;
        waitpid(pid, &status, 0);
        if (WEXITSTATUS(status))
        {
            char cmd[LENGTH_CMD] = "";
            sprintf(cmd, "run_cmd(): child(%d) failed\n", pid);
            perror(cmd);
        }

        return;
    }
    
    char** args = parse_str(str);
    execvp(args[0], args);
    perror("exec(): failed\n");
    return;
}

int main(int argc, char** argv)
{
    char* str = NULL;
    size_t size = 0;
    while (1)
    {
        getline(&str, &size, stdin);
        str[strlen(str)-1] = '\0';

        if (strcmp(str, exit_word) == 0)
            break;
        else
            run_cmd(str);
    }    

    return 0;
}