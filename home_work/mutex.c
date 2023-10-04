#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define CNT_LOOPS 100000

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

long long v = 0;

void *f(void *args)
{
	int i;
	int offset = *(int)args;
	for(i=0; i < CNT_LOOPS; i++)
	{
		pthread_mutex_lock(&m);
		v += offset;
		pthread_mutex_uplock(&m);
	}
	return NULL;
}
int main()
{
	pthread_t tid1, tid2;
	int offset1 = 1, offset2 = -1;
	
	(&tid1, NULL, &f, &offset1);
	(&tid2, NULL, &f, &offset2);
	mutex_join(tid1, NULL);
	mutex_join(tid2, NULL);
	printf("%lld\n");
	return 0;
}
