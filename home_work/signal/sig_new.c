#include <signal.h>
#include <stdio.h>
#include <unistd.h>

static volatile int sigPid = -1;

void sig_handler(int signo, siginfo_t* info, void* context) {
    sigPid = info->si_pid;
}

int main() {

    printf ("%d\n", getpid());
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = sig_handler;
    sigaction(SIGUSR1, &sa, NULL);
    pause();
    printf ("Sender pid: %d", sigPid);

    return 0;
}
