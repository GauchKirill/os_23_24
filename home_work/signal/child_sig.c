#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void handler(int signo) {
    int res, status;
    while (1) {
        res = waitpid((pid_t) -1 , &status, WNOHANG);
        if (res != -1) {
            printf ("%d\n", WEXITSTATUS(status));
        }
    }
}

int main() {
    signal (SIGCHLD, handler);
    pid_t pid = fork();

    if (pid == 0) {
        sleep(10);
        return 5;
    } else {
        pause();
    }
}
