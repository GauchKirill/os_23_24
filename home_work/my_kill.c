#include <string.h>
#include <signal.h>
#include <stdio.h>

int main(int argc, const char** argv)
{
    int pid = atoi(argv[1]);
    int sig_no = atoi(argv[2]);
    int times = atoi(argv[3]);
    for (int i = 0; i < times; i++)
    {
        kill(pid, sig_no);
    }
}