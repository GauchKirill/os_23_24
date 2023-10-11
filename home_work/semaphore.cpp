#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_LOOPS 1000000

long long v;
sem_t sem;
// pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

void* f(void* args) {
    int i;
    int offset = *(int*) args;

    for (int i = 0; i < 5; i++) {
        // pthread_mutex_lock(&m);
        sem_wait(&sem);
        if (offset == 1) {
            printf("HUI\n");
        } else {
            printf("HUILA\n");
        }
        v += offset;
        sem_post(&sem);
        // pthread_mutex_unlock(&m);
    }

    return NULL;
}

int main() {
    sem_init(&sem, 0, 1);
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
