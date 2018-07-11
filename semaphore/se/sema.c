#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h> 

#define QUEUE_SIZE 10

struct data
{
    char name[80]; 
};

struct flock lock, unlock;

int lock_open(int fd, int index)
{
	lock.l_start	= index; 
	lock.l_type = F_WRLCK;
	lock.l_len = 1;
	lock.l_whence = SEEK_SET;
	return fcntl(fd, F_SETLKW, &lock);
}

int lock_isopen(int fd, int index)
{
	lock.l_start	= index; 
	lock.l_type = F_WRLCK;
	lock.l_len = 1;
	lock.l_whence = SEEK_SET;
	return fcntl(fd, F_SETLK, &lock);
}

int lock_close(int fd, int index)
{
	unlock.l_start	= index; 
	unlock.l_type = F_UNLCK;
	unlock.l_len = 1;
	unlock.l_whence = SEEK_SET;
	return fcntl(fd, F_SETLK, &unlock);
}

void lock_init()
{
  lock.l_start  = 0;
  lock.l_type = F_WRLCK;
  lock.l_len = 1;
  lock.l_whence = SEEK_SET;
}
void unlock_init()
{
  unlock.l_start  = 0;
  unlock.l_type = F_UNLCK;
  unlock.l_len = 1;
  unlock.l_whence = SEEK_SET;
}

int main(int argc, char* argv[])
{
	int shmid;
	int i = 0;
	int offset = 0;
  int fd;

	void *shared_memory;
	struct data *ldata;    
	lock_init();
	unlock_init();

  if ((fd = open("addr.txt", O_RDWR)) < 0)
  {
  	perror("file open error ");
   	exit(0);
  }

	shmid = shmget((key_t)1234, sizeof(struct data)*QUEUE_SIZE, 0666);
	if (shmid == -1)
	{
		perror("shmget failed : ");
		exit(0);
	}

	shared_memory = (void *)malloc(sizeof(ldata)*QUEUE_SIZE);
	shared_memory = shmat(shmid, (void *)0, 0);
	if (shared_memory == (void *)-1)
	{
		perror("shmat failed : ");
		exit(0);
	}

	// 이 부분은 생산자가 가장 최근에 쓴 데이터의 인덱스를 
	// 찾아내기 위한 코드다.  
	// 잠금 파일의 레코드를 차례대로 검사하면서 잠금이 있는 부분을 검사한다. 
	// 잠금이 검사되면, 리턴한다.  
	while(1)
	{
		if(lock_isopen(fd, i)< 0)
		{
			if (errno == EAGAIN)
			{
				printf("Read index is %d %d %d\n", i, EAGAIN, errno);
				fcntl(fd, F_GETLK, &lock);  // 코드 1 
				offset = sizeof(struct data)*i; 
				break;
			}
			else
			{
				printf("Init Error\n");
				exit(0);
			}
		}
		lock_close(fd, i);
		i++;
		if (i == QUEUE_SIZE)
		{
			printf("Server Error\n");
		}
	}

	// 공유 메모리로 부터 데이터를 읽는다.
	while(1)
	{
		if (lock_open(fd, i) < 0)
		{
			perror("flock error");
		}
		ldata = (struct data *)(shared_memory+offset);
		printf("%s",ldata->name);	
		lock_close(fd, i);

		offset += sizeof(struct data);
		i++;
		if (i == QUEUE_SIZE) {i = 0;offset = 0;}	
	}
}

