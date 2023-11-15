#include <pthread.h>
#include <stdio.h>
//ipcs -- очереди смотреть
#define BUF_SIZE 10

int buffer[BUF_SIZE];

int add = 0;
int rem = 0;
int num = 0;

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t c_consumer = PTHREAD_COND_INITIALIZER;
pthread_cond_t c_producer = PTHREAD_COND_INITIALIZER;

void *consumer(void *args)
{
    int i;
    while (1)
    {
        pthread_mutex_lock(&m);
        if (num == 0)
            pthread_cond_wait(&c_consumer, &m);

        i = buffer[rem];
        rem = (rem + 1) % BUF_SIZE;
        num--;

        pthread_cond_signal(&c_producer);
        pthread_mutex_unlock(&m);
        printf("consume: %d\n", i);
    }
    return NULL;
}

void *producer(void *args)
{
    int i;
    for (i = 0; i < 50; i++)
    {
        pthread_mutex_lock(&m);
        if (num == BUF_SIZE)
            pthread_cond_wait(&c_producer, &m);
        
        buffer[add] = i;
        add = (add + 1) % BUF_SIZE;
        num++;

        pthread_cond_signal(&c_consumer);
        pthread_mutex_unlock(&m);
        printf("produce: %d\n", i);
    }
    return NULL;
}

int main()
{

    pthread_t tid1, tid2;

    pthread_create(&tid1, NULL, producer, NULL);
    pthread_create(&tid2, NULL, consumer, NULL);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    return 0;
}