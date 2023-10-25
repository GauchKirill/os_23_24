#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void handler(int sig_no) //обработчик
{
    printf("FIGNAL HANDLER\n");
}

int main()
{
    //signal(SIGINT, SIG_IGN); -- игнор сигнала
    signal(SIGINT, handler);
    int prev_mask = sigblock(sigmask(SIGINT)); // RT сигналы не блокируются, очень старая штука
    sigsetmask(prev_mask); //вернуть обработку
    while (1);
    signal(SIGINT, SIG_DFL); //-- дефолтный
    return 0;
}