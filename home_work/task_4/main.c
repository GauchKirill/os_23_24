#define _GNU_SOURCE
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <sched.h>

#include "settings.h"

static const long long unsigned CNT_THREADS         = THREADS_ON_SIDE_X * THREADS_ON_SIDE_Y;
static const long long unsigned THREADS_POINTS      = CNT_POINTS / CNT_THREADS;
static double square = 0.0;

static long numCore = 0;

unsigned int seed = 0;

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

double func(double x)
{
    return x;   
}

typedef struct Thread_args_
{
    double  low_x;
    double  step_x;
    double  low_y;
    double  step_y;
    long    numCore;
} Thread_args;

void* thread_func(void *data)
{
    Thread_args *thread_args = (Thread_args*) data;
    
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(thread_args->numCore, &cpuset);
    sched_setaffinity(0, sizeof(cpuset), &cpuset);
    
    register double x, y, thread_square = 0;
    const double    thread_low_x = thread_args->low_x,
                    thread_low_y = thread_args->low_y,
                    thread_step_x = thread_args->step_x,
                    thread_step_y = thread_args->step_y;
    srand((unsigned) time(NULL));
    for (long long unsigned i = 0; i < THREADS_POINTS; i++)
    {
        x = thread_low_x + (double) rand_r(&seed) / RAND_MAX * (thread_step_x);
        y = thread_low_y + (double) rand_r(&seed) / RAND_MAX * (thread_step_y);

        if (y < func(x)) 
            thread_square += 1;
    }
    pthread_mutex_lock(&m);
    square += thread_square;
    pthread_mutex_unlock(&m);    
    return NULL;
}


int experiment(double *exp_square, double *exp_time)
{
    const double left_x = 0.0;
    const double right_x = 1.0;  
    const double height_y = 1.0;
    const double low_y = 0.0;

    const double step_x = (right_x - left_x) / THREADS_ON_SIDE_X;
    const double step_y = (height_y - low_y) / THREADS_ON_SIDE_Y;

    pthread_t   tread[CNT_THREADS];
    Thread_args  tread_args[CNT_THREADS];

    numCore = sysconf(_SC_NPROCESSORS_ONLN);

    for (long long unsigned i = 0; i < CNT_THREADS; i++)
    {
        tread_args[i].low_x     = left_x + step_x * (i % THREADS_ON_SIDE_X);
        tread_args[i].step_x    = step_x;
        tread_args[i].low_y     = low_y + step_y * (i / THREADS_ON_SIDE_X);
        tread_args[i].step_y    = step_y;
        tread_args[i].numCore   = 1 << (i % numCore);
    }

    seed = (unsigned) time(NULL);

    struct timeval tv1, tv2;
    gettimeofday(&tv1, NULL);

    for(long long unsigned i = 0; i < CNT_THREADS; i++)
    {
        pthread_create(&tread[i], NULL, thread_func, tread_args + i);
    }

    for (long long unsigned i = 0; i < CNT_THREADS; i++)
        pthread_join(tread[i], NULL);

    gettimeofday(&tv2, NULL);
    square /= CNT_POINTS;

    *exp_square = square;
    *exp_time = (tv2.tv_sec - tv1.tv_sec) + (double) (tv2.tv_usec - tv1.tv_usec) / 1000000;

    return 0;
}

#define LENGTH_NAME_FILE 100

int main()
{
    double  exp_square      = 0,
            exp_time        = 0;

    experiment(&exp_square, &exp_time);

    char data_file[LENGTH_NAME_FILE] = "";
    sprintf(data_file, "./data/%s%lld_treads.txt", (CNT_THREADS < 10) ? "0" : "", CNT_THREADS);
    FILE* file = fopen(data_file, "a+");
    if (!file)
        return -1;
    fprintf(file, "%lf\n", exp_time);
    fclose(file);
    printf("square = %lf\n", square);
    return 0;
}
