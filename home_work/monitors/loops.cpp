#include <stdio.h>
#include <pthread.h>

#define NUM_LOOPS 5000000

long long v;

void* f(void* args) {
    int i;
    int offset = *(int*) args;

    for (int i = 0; i < NUM_LOOPS; i++) {
        v += offset;
    }

    return NULL;
}

int main() {
    pthread_t tid1, tid2;
    int offset1 = 1;
    int offset2 = -1;

    pthread_create(&tid1, NULL, &f, &offset1);
    pthread_create(&tid2, NULL, &f, &offset2);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    printf("%lld\n", v);

    return 0;
}
