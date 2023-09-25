#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

const char*     exit_word   = "q"; 
const char*     delim       = " ";

char*** parse_str(char* str)
{
    size_t cnt_args = 1, cnt_words = 0;

    char*** cmd = (char***) calloc(1, sizeof(char**));
    for (char *p = strtok(str, delim); p != NULL; p = strtok(NULL, delim))
    {
        switch (*p)
        {
        case '|':
        case '>':   
            cmd[cnt_args-1] = (char**) realloc(cmd[cnt_args-1], ++cnt_words*sizeof(char*));
            cmd[cnt_args-1][cnt_words-1] = NULL;

            cmd = (char***) realloc(cmd, ++cnt_args*sizeof(char**));
            cnt_words = 0;
            break;
        default:
            cmd[cnt_args-1] = (char**) realloc(cmd[cnt_args-1], ++cnt_words*sizeof(char*));
            cmd[cnt_args-1][cnt_words-1] = p;
            break;
        }
    }

    cmd = (char***) realloc(cmd, ++cnt_args*sizeof(char**));
    cmd[cnt_args-1] = NULL;

    return cmd;
}

void run_cmd(char* str)
{
    char*** cmd = parse_str(str);

    int     fd[2];
    pid_t   pid;
    int     fd_in = 0;
    size_t  i = 0;

    while (cmd[i] != NULL)
    {
        pipe(fd);

        if ((pid = fork()) < 0)
        {
            exit(1);

        } else
        if (pid == 0)
        {
            if (i > 0)
                dup2(fd_in, 0);

            if (cmd[i+1] != NULL)
                dup2(fd[1], 1);
            
            close(fd[0]);
            execvp((cmd)[i][0], cmd[i]);
            exit(2);

        } else
        {
            wait(NULL);

            close(fd[1]);
            if (i > 0)
                close(fd_in);

            fd_in = fd[0];
                i++;
        }
    }
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

        if (strncmp(str, exit_word, 2) == 0)
            break;
        else
            run_cmd(str);
    }
    free(str);

    return 0;
}