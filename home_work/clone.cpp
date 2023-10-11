#include <sys/wait.h>
#include <sys/utsname.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sched.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define STACK_SIZE 4096

int v;

int func(void* arg) {
    printf("PID: %lu, Inside func in thread: %lu\n", syscall(SYS_gettid), syscall(SYS_gettid));
    v = 5;
    sleep(5);

    return 0;
}

int main() {
    v = 2;
    printf("v = %d\n", v);
    char* child_stack = (char*) malloc(STACK_SIZE);
    int thread_id = clone(&func, child_stack + STACK_SIZE, CLONE_VM | CLONE_THREAD | CLONE_SIGHAND, NULL);

    if (thread_id < 0) {
        perror("clone");
    }
    printf("%d\n", getpid());
    sleep(7);
    printf("v = %d\n", v);

    return 0;
}
