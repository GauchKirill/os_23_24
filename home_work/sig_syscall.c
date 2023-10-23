#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

void sig_handler(int signo) {

}

int main() {
    int remain;

    signal(SIGUSR1, sig_handler);
    printf("PID: %d\n", getpid());
    sleep(10);
    fprintf(stderr, "Awake\n");
    fprintf (stderr, "Error %d\n", errno);

    return 0;
}
