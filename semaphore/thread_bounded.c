/*----------------------------------------------------------------------*/
//					2006 Operating System Term project					//
//			Bounded Buffer Program Using Multi thread Method			//
//											2001037466 Choi Jin Han		//
/*----------------------------------------------------------------------*/

/*include*/
#include<unistd.h>
#include<sys/types.h>
#include<errno.h>
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<string.h>
#include<fcntl.h>
#include<semaphore.h>

#define	BLKSIZE	10 //define buffer size
#define	READ_FLAGS O_RDONLY
#define WRITE_FLAGS (O_WRONLY | O_CREAT | O_EXCL)
#define WRITE_PERMS (S_IRUSR | S_IWUSR)

//define global variable
char buffer[BLKSIZE];
int input_counter=0;
int output_counter=0;
int fromfd, tofd;

//counting semaphore
sem_t full,empty;

//binary mutex lock
sem_t mutex;

//thread handler
void producer_handler(void *ptr);
void consumer_handler(void *ptr);

int main(int argc, char *argv[])
{
	pthread_t producer, consumer1, consumer2;

	int i[3]; //determine thread number
	i[0]=0;
	i[1]=1;
	i[2]=2;
	
	if(argc!=3)
	{
		fprintf(stderr, "Usage: %s input_file output_file\n", argv[0]);
		return 1;
	}

	/*------------input and output file open--------------*/
	if((fromfd=open(argv[1], READ_FLAGS))==-1)
	{
		perror("Failed to open input file");
		return 1;
	}
	
	if((tofd=open(argv[2], WRITE_FLAGS))==-1)
	{
		perror("Failed to creat output file");
		return 1;
	}

	/*---------semaphore initialize-------------*/
	sem_init(&full, 0, 0);
	sem_init(&empty, 0, 10);
	sem_init(&mutex, 0 ,1);

	/*---------create thread and allocate job------------*/
	pthread_create(&producer, NULL, (void*) &producer_handler, (void*)&i[0]);
	pthread_create(&consumer1, NULL, (void*) &consumer_handler, (void*)&i[1]);
	pthread_create(&consumer2, NULL, (void*) &consumer_handler, (void*)&i[2]);

	/*---------wait thread completition-----------*/
	pthread_join(producer, NULL);
	pthread_join(consumer1, NULL);
	pthread_join(consumer2, NULL);

	/*---------destroy semaphore----------*/
	sem_destroy(&full);
	sem_destroy(&empty);
	sem_destroy(&mutex);

	exit(0);
}

void producer_handler(void *ptr)
{
	char buf[1];
	int readbyte, end=0;

	while(1){
	/*----------Produce item-------------*/	
		printf("thread_producer\n");
		readbyte = read(fromfd, buf, 1);

		if(buf[0]==';') end++;

		sem_wait(&empty);
		sem_wait(&mutex);
	
	/*---------Critical section----------*/
		printf("thread_producer in critical section\n");
		buffer[input_counter] = buf[0];
		input_counter=(input_counter+1)%10;
	/*---------Critical section----------*/
		printf("thread_producer out of critical section\n");
		sem_post(&mutex);
		sem_post(&full);
		
		if(end==2) break;//when ';' is readed two times, producer stop operation
		//sleep(1);
	}
	printf("thread_producer completed\n");
}
		
void consumer_handler(void *ptr)
{
	char buf[1];
	int writebyte;
	
	while(1){
		printf("thread_consumer\n");
		sem_wait(&full);
		sem_wait(&mutex);
	
	/*---------Critical section----------*/
		printf("thread_consumer in critical section\n");
		buf[0] = buffer[output_counter];
		output_counter = (output_counter+1)%10;
		writebyte = write(tofd, buf, 1);
	/*---------Critical section----------*/
		printf("thread_consumer out of critical section\n");
		sem_post(&mutex);
		sem_post(&empty);
		
		if(buf[0]==';') break;
	}
	printf("thread_consumer completed\n");		
}
