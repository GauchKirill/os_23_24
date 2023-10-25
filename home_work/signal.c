#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

#define BUF_SZ

void sig_handler(int signo, siginfo_t *info, void *context)
{
    return;
}

void sender(int pid, const char* input_file)
{
    char data[BUF_SZ] = "";
    FILE* input = fopen(input_file, "rb");
    if (input)
    {
        int cnt_read = 0;
        while ((cnt_read = fread(data, sizeof(int), BUF_SZ, input)) > 0)
        {
            /* code */
        }
        
        close(input);
    }
}

void resiver(int pid, const char* output_file)
{

}

int main(int argc, const char** argv)
{
    if (argc != 3)
    {
        printf("Enter input and outpul files\n");
        return 0;
    }

    pid_t pid = fork();

    if (pid < 0)
        perror("fork()");
    else
    if (pid)
        sender(pid, argv[1]);
    else
        resiver(getppid(), argv[2]);

    return 0;
}