#include <sys/wait.h>
#include <sys/utsname.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sched.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

int v;

int g(void* arg) {
    return 0;
}

void* func(void* arg) {
    v = 5;
    printf("PID: %lu, Inside func in thread: %lu\n", syscall(SYS_gettid), syscall(SYS_gettid));
}

int main() {
    v= 2;
    printf("v = %d\n", v);
    pthread_t tid;

    pthread_create(&tid, NULL, &func, NULL);
    printf("created pthread: %lu\n", tid);
    // pthread_join(tid, NULL);
    pthread_detach(tid);

    printf("%d\n", getpid());
    printf("v = %d\n", v);
    return 0;
}
