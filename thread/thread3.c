#include <pthread.h>
#include <stdio.h>

int result =0;
void* thread_summation(void *arg);

int main(int argc,char* argv[])
{
	pthread_t id_t1,id_t2;
	int range1[2] = {1,10};
	int range2[2] = {11,20};
	
	pthread_create(&id_t1,NULL,thread_summation,(void *)range1);
	pthread_create(&id_t2,NULL,thread_summation,(void *)range2);

	pthread_join(id_t1,NULL);
	pthread_join(id_t1,NULL);

	printf("result %d\n",result);
	return 0;
}

void* thread_summation(void* arg)
{
	int start = ((int*)arg)[0];
	int end = ((int*)arg)[1];
	
	while(start<=end)
	{
		result +=start;
		start++;
	}
	return NULL;
}
