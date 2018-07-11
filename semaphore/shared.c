#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <string.h> 
#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>

int main()
{
    int shmid;
    int pid;

    int *cal_num;
    void *shared_memory = (void *)0;


    // 공유메모리 공간을 만든다.
    shmid = shmget((key_t)1234, sizeof(int), 0666|IPC_CREAT);

    if (shmid == -1)
    {
        perror("shmget failed : ");
        exit(0);
    }

    // 공유메모리를 사용하기 위해 프로세스메모리에 붙인다. 
    shared_memory = shmat(shmid, (void *)0, 0);
    if (shared_memory == (void *)-1)
    {
        perror("shmat failed : ");
        exit(0);
    }

    cal_num = (int *)shared_memory;
    pid = fork();
    if (pid == 0)
    {
        shmid = shmget((key_t)1234, sizeof(int), 0);
        if (shmid == -1)
        {
            perror("shmget failed : ");
            exit(0);
        }
        shared_memory = shmat(shmid, (void *)0, 0666|IPC_CREAT);
        if (shared_memory == (void *)-1)
        {
            perror("shmat failed : ");
            exit(0);
        }
        cal_num = (int *)shared_memory;
        *cal_num = 2;

        while(1)
        {
            *cal_num = *cal_num + 2;
            printf("child %d\n", *cal_num); 
            sleep(1);
        }
    }

    // 부모 프로세스로 공유메모리의 내용을 보여준다. 
    else if(pid > 0)
    {
        while(1)
        {
	    if(*cal_num>20){
		printf("break;");
		break;
	    }
            sleep(1);
            printf("%d\n", *cal_num);
        }
    }
}	

int childProcess(int clnt_sock,FILE* fcache,cache* nt,int fd1[],int i)
{
	int str_len;
	char buf[BUF_SIZE];
	DNSTable dns[line];    //공용 DNS 테이블
	cache ch[PQ_SIZE];     //Cache용 테이블
	struct hash* hashTable = (struct hash *) calloc(100, sizeof(struct hash));
	struct hash* hashDNS = (struct hash *) calloc(100, sizeof(struct hash));
	printf(" i : %d\n",i); // 각자 프로세스가 갖는 id 값
	
	//read(fd1[1],buf,2);
	//printf("buf : %s\n",buf);
	//id = atoi(buf);
	//printf("id : %d\n",id);
	//printf("clnt : %d\n",clntID);
	//현재지금쓰고잇는id clntID
	while(1){
		if(lock ==1 && i == clntID) //키를 가지고있으면서 자신의 차례가되면 들어감
		{
		//DNS테이블 업데이트용
			lock--; // 자신의차례니까 다음 프로세스가 오기전에 먼저 문을잠굼
			sleep(0);
	//		printf("closed lock :%d\n",lock);
			load_all_list(dns,line);
			Heaping(line,dns,ch);
			makeHashIPTable(line,dns,hashTable);
			makeHashDNSTable(line,dns,hashDNS);
			char *str = inet_ntoa(clnt_adr.sin_addr);
	//		printf("addr str: %s\n",str);
			lock++; // 볼일 다봤으니 다른 프로세스가 입장할수있게 도와줌
	//		printf("clntID = %d\n",clntID);
			//clntID++;
			printf("clntID  2= %d\n",clntID);
			printf("opend lock :%d\n",lock);	
			break;
		}
	}
	
	while(1){
		printf("child lock :%d\n",lock);
		int select = select_menu(clnt_sock);
		switch(select){
			case 0:
				return 0;
			case 1:
				memset(buf,0,BUF_SIZE);
				display(hashDNS,line);
				read(clnt_sock,buf,BUF_SIZE);
				printf("hash buf: %s\n",buf);
				searchInHash(buf,hashDNS,line,clnt_sock,ch); 
				memset(buf,0,BUF_SIZE);
				read(clnt_sock,buf,BUF_SIZE);//클라이언트에게 Thank you 라는 문자를받는다
				printf("Message from client: %s\n", buf);
				break;
			case 2:
				memset(buf,0,BUF_SIZE);
				display(hashTable,line);
				read(clnt_sock,buf,BUF_SIZE);
				printf("hash buf: %s\n",buf);
				searchInHash(buf,hashTable,line,clnt_sock,ch);
				memset(buf,0,BUF_SIZE);
				read(clnt_sock,buf,BUF_SIZE);//클라이언트에게 Thank you 라는 문자를받는다
				printf("Message from client: %s\n", buf);
				break;
			case 3:
				top10Send(clnt_sock,ch);
				read(clnt_sock,buf,BUF_SIZE);//클라이언트에게 Thank you 라는 문자를받는다
				printf("Message from client: %s\n", buf);
				break;	
		}
	}
}

