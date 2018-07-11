/*----------------------------------------------------------------------*/
//		2006 Operating System Term project		    //
//	Bounded Buffer Program Using Multi Process Method	    //
//					2001037466 Choi Jin Han    //
/*----------------------------------------------------------------------*/

/*include*/
#include<unistd.h>
#include<errno.h>
#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include<stdlib.h>
#include<signal.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/wait.h>

#define BLKSIZE 3 //define buffer size
#define READ_FLAGS O_RDONLY
#define WRITE_FLAGS (O_WRONLY | O_CREAT | O_EXCL)
#define WRITE_PERMS (S_IRUSR | S_IWUSR)
#define PERM (S_IRUSR | S_IWUSR)
#define FLAGS (O_CREAT | O_EXCL)
#define ANY 0

//define semaphore name
#define FULL 0
#define EMPTY 1
#define MUTEX 2

//global variable
char *shared_memory;
int shmid, sem_id;

//semaphore struct
static struct sembuf full_sub = {FULL, -1, 0};		//wait(full)
static struct sembuf full_add = {FULL, 1, 0};		//signal(full)
static struct sembuf empty_sub = {EMPTY, -1, 0};	//wait(empty)
static struct sembuf empty_add = {EMPTY, 1, 0};		//signal(empty)
static struct sembuf mutex_lock = {MUTEX, -1, 0};	//wait(mutex)
static struct sembuf mutex_release = {MUTEX, 1, 0};	//signal(mutex)

//process handler function
int producer_handler(int fromfd);
int consumer_handler(int tofd, int consumer_num);

int main(int argc, char *argv[])
{
	pid_t consumer1, consumer2;
	int fromfd, tofd,status;
	char buffer[BLKSIZE];

	if(argc!=3)
	{
		fprintf(stderr, "Usage: %s input_file output_file\n",argv[0]);
		return 1;
	}
	
	/*----------------shared memory initialize-------------------*/
	shmid = shmget(IPC_PRIVATE, sizeof(buffer), 0600|IPC_CREAT);
	if(shmid == -1){
		perror("shmget failed");
		exit(0);
	}
	shared_memory = shmat(shmid, (char *)0, 0);
	if(shared_memory == (char *)-1){
		perror("shmat failed");
		exit(0);
	}
	printf("Memory attached at %s\n", shared_memory);
	
	/*------------input and output file open------------------*/
	if((fromfd = open(argv[1], READ_FLAGS)) == -1)
	{
		perror("Failed to open input file");
		return 1;
	}

	if((tofd = open(argv[2], WRITE_FLAGS)) == -1)
	{
		perror("Failed to open output file");
		return 1;
	}
	/*--------semaphore initialization-----------*/
	sem_id = semget(IPC_CREAT, 3, 0600|IPC_CREAT);
	if(sem_id == -1){
		perror("semget failed");
		exit(0);
	}

	printf("sem_id %d\n", sem_id);
		
	semctl(sem_id, FULL, SETVAL, 0);
	semctl(sem_id, EMPTY, SETVAL, 10);
	semctl(sem_id, MUTEX, SETVAL, 1);

	/*-------create operation process--------*/
	consumer1 = fork();

	/*--allocate process job--*/
	
	if(consumer1==0)
	{
		consumer_handler(tofd,1);//if child process, call consumer_handler
		exit(1);
	}

	else
	{
		consumer2 = fork();
		if(consumer2 == 0)
		{
			consumer_handler(tofd,2);//if child process, call consumer_handler
			exit(1);
		}
		else{
			producer_handler(fromfd);
		}
	}
	
	/*------wait process completition------*/
	while(wait(&status) != -1);

	/*--------remove semaphore----------*/
	semctl(sem_id, ANY, IPC_RMID, ANY);

	/*--remove shared_memory--*/
	shmctl(shmid, IPC_RMID, NULL);

	/*-------close input output file----------*/
	close(fromfd);
	close(tofd);

	printf("All processes are terminated and all resources are released\n");

	exit(1);
}

int producer_handler(int fromfd)
{
	char temp[1];
	int readbyte, end=0;
	int input_counter=0;

	while(1){
	/*------------produce item----------------*/
		printf("process_producer\n");
		readbyte = read(fromfd, temp, 1);

		if(temp[0]==';') end=end+1;

		semop(sem_id, &empty_sub, 1);//wait(empty)
		semop(sem_id, &mutex_lock, 1);//wait(mutex)

	/*-----------critical section-------------*/
		printf("process_producer in critical section\n");
		shared_memory[input_counter] = temp[0];
		printf("temp: %c input_counter: %d\n",temp[0],input_counter);
		input_counter= (input_counter+1)%10;
	/*-----------critical section-------------*/

		semop(sem_id, &mutex_release, 1);//signal(full)
		semop(sem_id, &full_add, 1);//signal(mutex)

		printf("process_producer out of critical section\n");

		if(end==2) break;//when ';' is readed two time, producer stop operation
		//sleep(1);
	}
	printf("process_producer operation is completed\n");
}

int consumer_handler(int tofd, int consumer_num)
{
	char temp[1];
	int writebyte, output_counter;

	while(1){
		printf("process_consumer%d\n",consumer_num);
		
		semop(sem_id, &full_sub, 1);//wait(full)
		semop(sem_id, &mutex_lock, 1);//wait(mutex)

	/*------------critical section-------------*/
		printf("process_consumer%d in critical section\n",consumer_num);
		output_counter=(int)shared_memory[10];//there is ouput_pointer in bounded buffer 
		temp[0] = shared_memory[output_counter];
		printf("temp: %c output_counter: %d\n",temp[0],output_counter);
		output_counter = (output_counter+1)%10;
		shared_memory[10] = (char)output_counter;
		writebyte = write(tofd, temp, 1);
	/*------------critical section-------------*/

		semop(sem_id, &mutex_release, 1);//signal(mutex)
		semop(sem_id, &empty_add, 1);//signal(empty)

		printf("process_consumer%d out of critical section\n",consumer_num);
		
		if(temp[0]==';') break;
	}
	printf("process_consumer%d completed\n", consumer_num);
}
