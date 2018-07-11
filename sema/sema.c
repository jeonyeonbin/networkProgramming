#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <stdlib.h>

#define SEMKEY      0x100
#define RECORD_FILE "record.txt"
int initsem(int semkey);
int delsem(int semid);
int p(int semid);
int v(int semid);
union semun {
  int val;               /* used for SETVAL only */
  struct semid_ds *buf;  /* for IPC_STAT and IPC_SET */
  ushort *array;         /* for GETALL and SETALL */
}; 

int initsem(int semkey) {
  int semid, status = 0;
  union semun sem_union;
   
  if((semid = semget(semkey, 1, 0666 | IPC_CREAT | IPC_EXCL)) == -1) {
    if(errno == EEXIST) {
      semid = semget(semkey, 1, 0);
    } 
  } else {
    sem_union.val = 1;
    status = semctl(semid, 0, SETVAL, sem_union); 
  }
  if (semid == -1 || status == -1) {
    perror("initsem");
    return -1;  
  }
  return semid; 
}

int delsem(int semid) {
  union semun sem_union;

  if (semctl(semid, 0, IPC_RMID, sem_union) == -1) {
    perror("semaphore delete");
    return -1;
  }
  return 0;
}

int p(int semid) {
  struct sembuf p_buf;

  p_buf.sem_num = 0;
  p_buf.sem_op = -1;
  p_buf.sem_flg = SEM_UNDO;
   
  if(semop(semid, &p_buf, 1) == -1) {
    perror("p(semid)");
    return -1;
  }
  return 0;
}

int v(int semid) {
  struct sembuf v_buf;
  
  v_buf.sem_num = 0;
  v_buf.sem_op = 1;
  v_buf.sem_flg = SEM_UNDO;
  
  if(semop(semid, &v_buf, 1) == -1) {
     perror("v(semid)");
     return -1;
  }
  return 0;
}
struct reg_list {
  pid_t pid;/* 프로세스 번호 */
  time_t start;   /* 프로세스 실행 시작 시간 */
  time_t end;     /* 프로세스 실행 종료 시간 */
};

struct reg_list reg;

void reg_start();
void reg_end(int);

void reg_start() {
  reg.pid = getpid();            /* 프로세스 번호 */
  reg.start = time(NULL);        /* 현재 시간 */
  reg.end = (time_t)0;
}

void reg_end(int sem_delete_flag) {
  int semid;
  FILE *fp;

  if ((semid = initsem(SEMKEY)) < 0) exit(1);

  reg.end = time(NULL);

  p(semid);       /* 임계영역의입구*/
  
  if ((fp = fopen(RECORD_FILE, "a")) == NULL) {
    perror(RECORD_FILE);
    return; /* 쓰기 실패 , SEM_UNDO 로 인해 세마포어값은 원래대로 돌아감 */
  }
  fwrite(&reg, sizeof(struct reg_list), 1, fp);
  fclose(fp);
      
  v(semid);
             
  if (sem_delete_flag) { /* 세마포어 삭제 플래그가 설정되었다면 제거 */
    delsem(semid);
  }
}

void main(int argc, char *argv[]) {
  reg_start();
  //sleep(2);        /* 실제에 있어서의 어떤 작업 */
 if (argc > 1) {/* 세마포어를 조건적으로 제거 */
    reg_end(1);
  } else {
    reg_end(0);
  }

    char tbuf1[20], tbuf2[20];
    FILE *fp;


    	if ((fp = fopen(RECORD_FILE, "rb")) == NULL) {
   	  perror(RECORD_FILE);
   	  exit(1);
   	}

	pid_t consumer1, consumer2;
	consumer1 = fork();
	if(consumer1==0)
	{
		//consumver_handler(tofd,getpid());
		exit(1);
	}
	else{
	   	while (fread(&reg, sizeof(struct reg_list), 1, fp) == 1) {
	   	  strftime(tbuf1, 20, "%m/%d %I:%M:%S", localtime(&(reg.start)));
	   	  strftime(tbuf2, 20, "%m/%d %I:%M:%S", localtime(&(reg.end)));
	   	  printf("프로세스 : %5d , 시작시간: %s , 종료시간 : %s\n", 
	   	               reg.pid, tbuf1, tbuf2);
	   	}
	    printf("\n");
   	}
}
