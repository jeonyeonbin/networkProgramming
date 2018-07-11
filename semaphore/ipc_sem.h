#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

int initsem(int semkey);
int delsem(int semid);
int p(int semid);
int v(int semid);

int initsem(int semkey) {
  intsemid, status = 0;
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
