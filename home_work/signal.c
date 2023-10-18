#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

void sender(const char* input_file)
{
    FILE *input = fopen(input_file, "r");

    close(input);
}

void resiver(const char* output_file)
{
    FILE *output = fopen(output_file, "w");

    close(output);
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        printf("Enter input and outpul files\n");
        return 0;
    }

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork()");
        return 0;
    }
    else
    if (pid)
    {
        sender(argv[1]);
        return 0;
    }
    else
    {
        resiver(argv[2]);
        return 0;
    }
}