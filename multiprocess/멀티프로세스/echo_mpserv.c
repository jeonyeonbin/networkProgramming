#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#define BUF_SIZE 70
#define MAX_QUEUE_SIZE 100
#define PQ_SIZE 10
#define SEMKEY      0x100
typedef int KeyType;
typedef int element;

typedef struct Menu{		  // 서버와 클라이언트를 맞추기위한 메뉴 구조체
	int number;		  //클라이언트에서 넘어온 메뉴번호를 atoi 
	char clnt_num[BUF_SIZE];  //클라이언트에서 넘어오는 메뉴번호
}Menu;
typedef struct
{
	element key; //우선순위(priority)
	int value;			 //부속 데이터 필드 추가
} HeapElement;
//힙 구조체 정의
typedef struct{
	HeapElement heap[MAX_QUEUE_SIZE];
	int size;
} HeapType;

typedef struct DNSTable{
	char ip[30];
	char name[30];
	char alias[20];
	int hit;
}DNSTable;
typedef struct cache{
	char ip[30];
	char name[30];
	char alias[20];
	int hit;
}cache;

int eleCount = 0;
struct node {
    char key[100]; 
    int hit;
    char alias[100];
    char name[100];
    struct node *next;

};
struct hash {

    struct node *head;

    int count;

};
union semun {
  int val;               /* used for SETVAL only */
  struct semid_ds *buf;  /* for IPC_STAT and IPC_SET */
  ushort *array;         /* for GETALL and SETALL */
}; 
int lock = 1;
static int clntID =1;
static int i;
int semid;
struct sockaddr_in clnt_adr;
void error_handling(char *message);
void read_childproc(int sig);
int search_cache_Domain(DNSTable* nt,char* domain,int semid,int line);
void searchDomain(cache* nt,char* domain);
int search_cache_IP(DNSTable* nt,char* IP,int semid,int line);
int multiprocess(int clnt_sock,int serv_sock);
int childProcess(int clnt_sock,FILE* fcache,cache* nt,int fd1[],int i);
int select_menu(int clnt_sd);
void searchAll(cache* nt,int clnt);
int compare(HeapElement a, HeapElement b);
void init_pq(HeapType *H);
void insert(HeapType *H, HeapElement item);
HeapElement remove_pq(HeapType *H);
HeapElement minvalue(HeapType *H);
int is_empty_pq(HeapType H);
void Heaping(int line,DNSTable* dns,cache* ch,int semid);
void top10Send(int clnt,cache* ch);
struct node * createNode(char* key, char *name, int hit,char *alias);
void insertToHash(char* key, char *name, int hit,char* alias,struct hash* hashTable,int line);
void deleteFromHash(char* key,struct hash* hashTable,int line);
void searchInHash(char* key,struct hash* hashTable,int line,int clnt,cache* ch); 
void display(struct hash* hashTable,int line);
void makeHashIPTable(int line,DNSTable *dns,struct hash* hashTable);
void makeHashDNSTable(int line,DNSTable* dns,struct hash * hashTable);
int initsem(int semkey);
int delsem(int semid);
int p(int semid);
int v(int semid);
void load_all_list(DNSTable* nt,int line,int semid);
void load_keyvalue(int line,HeapElement *e,int semid);
int line_check(int semid);
void caching(FILE *f,cache* nt,int semid);
void write_log(char *msg,int semid);
void write_search_log(char *msg,char *search,int semid);
int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr;
	
	pid_t pid;
	struct sigaction act;
	socklen_t adr_sz;
	int str_len, state;
	//char buf[BUF_SIZE];
	//FILE* fcache;
	cache nt[5];
	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	act.sa_handler=read_childproc;
	sigemptyset(&act.sa_mask);
	act.sa_flags=0;
	state=sigaction(SIGCHLD, &act, 0);
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");

	//caching(fcache,nt);
	pid =fork();
	int fd1[2];
	pipe(fd1);
	char buffer[BUF_SIZE];
	
	int semid;
  	if ((semid = initsem(SEMKEY)) < 0) exit(1);

	if(pid==-1)
	{
		close(clnt_sock);
		return 0;
	}
	if(pid > 0){
		while(1)
		{
			adr_sz=sizeof(clnt_adr);
			clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
			if(clnt_sock==-1)
				continue;
			else
				puts("new client connected...");
	//		multiprocess(clnt_sock,serv_sock);
			pid_t pid;
			FILE *fcache;
			cache nt[5];
		
			char id_num[BUF_SIZE];
			char buf[BUF_SIZE];
			pid=fork();
			printf("present pid :%d\n",i);
			//lock=1;
			i++;
			clntID = i;
			printf("clnt ID 1: %d\n",clntID);
			printf("i :%d\n",i);
			if(pid==-1)
			{
				close(clnt_sock);
				return 0;
			}
			if(pid==0)
			{
				close(serv_sock);
			
				//childProcess(clnt_sock,fcache,nt,fd1,i);
				int str_len;
				char buf[BUF_SIZE];
				int line = line_check(semid);
				DNSTable dns[line];    //공용 DNS 테이블
				cache ch[PQ_SIZE];     //Cache용 테이블
				struct hash* hashTable = (struct hash *) calloc(100, sizeof(struct hash));
				struct hash* hashDNS = (struct hash *) calloc(100, sizeof(struct hash));
				printf(" i : %d\n",i); // 각자 프로세스가 갖는 id 값
				char *str = inet_ntoa(clnt_adr.sin_addr);
				write_log(str,semid);
	
				//read(fd1[1],buf,2);
				//printf("buf : %s\n",buf);
				//id = atoi(buf);
				//printf("id : %d\n",id);
				//printf("clnt : %d\n",clntID);
				//현재지금쓰고잇는id clntID
				while(1){
					//if(lock ==1 && i == clntID) //키를 가지고있으면서 자신의 차례가되면 들어감
					//{
					//DNS테이블 업데이트용
						lock--; // 자신의차례니까 다음 프로세스가 오기전에 먼저 문을잠굼
						sleep(0);
				//		printf("closed lock :%d\n",lock);
						load_all_list(dns,line,semid);
						Heaping(line,dns,ch,semid);
						makeHashIPTable(line,dns,hashTable);
						makeHashDNSTable(line,dns,hashDNS);
						//write(fd1[0],str,BUF_SIZE);
						
				//		printf("addr str: %s\n",str);
						lock++; // 볼일 다봤으니 다른 프로세스가 입장할수있게 도와줌
				//		printf("clntID = %d\n",clntID);
						//clntID++;
						printf("clntID  2= %d\n",clntID);
						printf("opend lock :%d\n",lock);	
						break;
					//}
				}
	
				while(1){
					printf("child lock :%d\n",lock);
					int select = select_menu(clnt_sock);
					if(select == 0)
						break;
					switch(select){
						case 0:
							break;
						case 1:
							memset(buf,0,BUF_SIZE);
							display(hashDNS,line);
							read(clnt_sock,buf,BUF_SIZE);
							printf("hash buf: %s\n",buf);
							searchInHash(buf,hashDNS,line,clnt_sock,ch); 
							search_cache_Domain(dns,buf,semid,line);
							write_search_log(str,buf,semid);
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
							search_cache_IP(dns,buf,semid,line);
							write_search_log(str,buf,semid);
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
				close(clnt_sock);
				puts("client disconnected...");
				//return 0;
			}
			else {                         //부모프로세스의 역할!!!!!!
				//sprintf(id_num,"%d",i);//길이 복사
				//printf("id_num : %s",id_num);
				//write(fd1[0],id_num,BUF_SIZE);

				close(clnt_sock);
				continue;
			}
		
		}
	}
	else{
		close(serv_sock);
		close(clnt_sock);
		int len;
		while(1){	
			lock= 0;
			len =read(fd1[1],buffer,BUF_SIZE);
			if(len==0)
				break;
			
		}
		printf("buffer :%s\n",buffer);
	}
	close(serv_sock);
	return 0;
}

		//read(fd1[1],buf,2);
		//printf("buf : %s\n",buf);
		//int id; //새로운 클라이언트가 들어왓을때의 id
		//id = atoi(buf);
		//printf("id : %d\n",id);
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
void write_log(char *msg,int semid)
{
	struct tm *t;
  	time_t timer;

  	timer = time(NULL);    // 현재 시각을 초 단위로 얻기
  	t = localtime(&timer); // 초 단위의 시간을 분리하여 구조체에 넣기

	char buf[BUF_SIZE];
	FILE *f;
	p(semid);
	if((f=fopen("Log.txt","a"))==NULL)
	{	
		printf("error\n");
		exit(1);
	}
	
	sprintf(buf,"%s 님이 %04d-%02d-%02d %02d:%02d:%02d에 입장하엿습니다.",msg,t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
	fprintf(f,"%s\n",buf);
	
	fclose(f);
	v(semid);

}
void write_search_log(char *msg,char *search,int semid)
{
	struct tm *t;
  	time_t timer;

  	timer = time(NULL);    // 현재 시각을 초 단위로 얻기
  	t = localtime(&timer); // 초 단위의 시간을 분리하여 구조체에 넣기

	char buf[150];
	FILE *f;
	p(semid);
	if((f=fopen("Log.txt","a"))==NULL)
	{	
		printf("error\n");
		exit(1);
	}
	
	sprintf(buf,"%s 님이 %04d-%02d-%02d %02d:%02d:%02d에 %s 검색하엿습니다.",msg,t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec,search);
	fprintf(f,"%s\n",buf);
	
	fclose(f);
	v(semid);
		
}


int select_menu(int clnt_sd)
{
	Menu menu;

	//클라이언트가 보내는 menu의 번호를 읽어온다.
	int read_size =read(clnt_sd,menu.clnt_num,1); //메뉴번호는 하나만읽기때문에 하나만사용
	menu.clnt_num[read_size] = '\0';// 다시초기화 허나 read_size는 1
	menu.number = atoi(menu.clnt_num); //클라이언트에서온 번호를 반환해 값을 int형태로 사용 => 위의 switch case문사용하기위해
	printf("menu.number :%d\n",menu.number);
	/////////////////////////////////////////////
	return menu.number;
}


void read_childproc(int sig)
{
	pid_t pid;
	int status;	
	pid=waitpid(-1, &status, WNOHANG);
	printf("removed proc id: %d \n", pid);
}
void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
void caching(FILE *f,cache* nt,int semid)
{
	int i = 0;
	p(semid);
	if((f=fopen("addr.txt","rb"))==NULL)
	{	
		printf("error\n");
		exit(1);
	}
	
	while(!feof(f)){
		if(i==5)
		   break;
		fscanf(f,"%17s%12s%19s%3d", nt[i].name, nt[i].alias, nt[i].ip, &nt[i].hit);
		i++;
	}

	
	fclose(f);
	v(semid);
}

void top10Send(int clnt,cache* ch)
{
	printf("top10\n");
	char buf[BUF_SIZE];
	char quit = 'q';
	for(int i =0; i<PQ_SIZE; i++)
	{
		sprintf(buf,"%17s%12s%19s%3d", ch[i].name, ch[i].alias, ch[i].ip, ch[i].hit);
		write(clnt,buf,strlen(buf));
	}
	write(clnt,&quit,1);

}

int search_cache_Domain(DNSTable* nt,char* domain,int semid,int line)
{
	for(int i=0;i<line;i++){
		if(strcmp(nt[i].name,domain)==0){
			nt[i].hit++;
			break;
		}
	}
	FILE *fp;
	FILE *f;
	p(semid);
	if((f=fopen("addr.txt","wb"))==NULL)
	{	
		printf("error\n");
		exit(1);
	}
	for(int i=0;i<line;i++){
		fprintf(f,"%17s%12s%19s%3d\n", nt[i].name, nt[i].alias, nt[i].ip, nt[i].hit);
	}
	fclose(f);
	if((fp=fopen("KeyValue.txt","wb"))==NULL)
	{	
		printf("error\n");
		exit(1);
	}
	
	for(int i=0;i<line;i++){
		fprintf(f,"%d %d\n",i,nt[i].hit);
	}

	fclose(fp);
	v(semid);
	
	return 0;
}

int search_cache_IP(DNSTable* nt,char* IP,int semid,int line)
{
	for(int i=0;i<line;i++){
		if(strcmp(nt[i].ip,IP)==0){
			nt[i].hit++;
			break;
		}
	}
	
	FILE *fp;
	FILE *f;
	p(semid);
	if((f=fopen("addr.txt","wb"))==NULL)
	{	
		printf("error\n");
		exit(1);
	}
	for(int i=0;i<line;i++){
		fprintf(f,"%17s%12s%19s%3d\n", nt[i].name, nt[i].alias, nt[i].ip, nt[i].hit);
	}
	fclose(f);

	if((fp=fopen("KeyValue.txt","wb"))==NULL)
	{	
		printf("error\n");
		exit(1);
	}
	
	for(int i=0;i<line;i++){
		fprintf(f,"%d %d\n",i,nt[i].hit);
	}

	fclose(fp);
	
	v(semid);
	return 0;
}


int compare(HeapElement a, HeapElement b) {
	if (a.key >= b.key) return -1;
	if (a.key < b.key) return 1;
	return 0;
	//혹은 return b.key – a.key;
}
//힙 초기화 함수

void init_pq(HeapType *H) {
	H->size = 0;
}

//힙에서의 삽입 함수
void insert(HeapType *H, HeapElement item)
{
	int k = ++(H->size);  //힙크기를 1증가시키고, k가 마지막 노드를 가리키게 한다.
						  //힙을 올라가며, 부모노드와 값을 비교하여 필요하면 교환!

	while (k != 1 && compare(H->heap[k / 2], item) >= 0)
	{
		H->heap[k] = H->heap[k / 2];
		k /= 2;
	}
	//k번째가 item의 위치!
	H->heap[k] = item;
	
	//printf("insert (%d, %d) \n", item.key, item.value);
	//printf("size : %d\n",k);
}

//힙에서의 삭제 함수
HeapElement remove_pq(HeapType *H) {
	HeapElement item, tmp;
	int parent = 1, child = 2;
	//printf("compare size %d ", H->size);
	if (H->size == 0)
	{
		fprintf(stderr, "An emtpy heap!\n");
		exit(1);
	}
	item = H->heap[1];  //반환할 항목
	tmp = H->heap[H->size];  //마지막 항목을 tmp에 복사
	H->size--;  //힙 사이즈 1감소
	while (child <= H->size)
	{
		//최대힙 :자식노드 중 더 큰 노드(최소힙 : 더 작은 노드)를 child로!!!
		if (child + 1 <= H->size)  //만일 오른쪽 자식도 존재하면
		{
			if (compare(H->heap[child], H->heap[child + 1]) > 0) //만일 오른쪽이면
				child++;
		}
		if (compare(tmp, H->heap[child]) <= 0) break;  //tmp의 자리를 찾음!
		H->heap[parent] = H->heap[child];
		parent = child;
		child *= 2;
	}
	H->heap[parent] = tmp;
	//printf("remove (%d, %d) \n", item.key, item.value);
	return item;
}


HeapElement minvalue(HeapType *H) {
	return H->heap[1];
}

int is_empty_pq(HeapType H) {

	return (H.size == 0);
}
int line_check(int semid)
{
	int Num_of_Line = 0;
	char ch;

	FILE *f;
	p(semid);
	if((f=fopen("KeyValue.txt","rb"))==NULL)
	{	
		printf("error\n");
		exit(1);
	}
	
	while (1) {// 반복문을 시작합니다. 
		ch = (char)fgetc(f);

		if (ch == '\n') Num_of_Line++;  //한줄씩 띄어질경우 줄수 증가한다.

		if (feof(f) != 0) // 파일을 끝까지 읽었는지 체크합니다. 
			break;
	}
	fclose(f);
	v(semid);
	//printf("Num_Of_Line : %d\n",Num_of_Line);
	return Num_of_Line;
}

void load_keyvalue(int line,HeapElement *e,int semid)
{
	int i = 0;
	FILE *f;
	p(semid);
	if((f=fopen("KeyValue.txt","rb"))==NULL)
	{	
		printf("error\n");
		exit(1);
	}
	
	while(!feof(f)){
		if(i==line)
		   break;
		fscanf(f,"%d %d",&e[i].value,&e[i].key);
		i++;
	}

	
	fclose(f);
	v(semid);
}
void load_all_list(DNSTable* nt,int line,int semid)
{
	int i = 0;
	FILE *f;
	p(semid);
	if((f=fopen("addr.txt","rb"))==NULL)
	{	
		printf("error\n");
		exit(1);
	}
	
	while(!feof(f)){
		if(i==line)
		   break;
		fscanf(f,"%17s%12s%19s%3d", nt[i].name, nt[i].alias, nt[i].ip, &nt[i].hit);
		i++;
	}

	
	fclose(f);
	v(semid);
}
void Heaping(int line,DNSTable* dns,cache* ch,int semid)
{
	HeapElement h[line];
	HeapType Heap;

	init_pq(&Heap);


	load_keyvalue(line,h,semid);
	int i;
	int key =0;
	
	printf("h[0].key =%d\n",h[0].key);
	for(i= 0; i<line; i++)
	{
		key = minvalue(&Heap).key;
		insert(&Heap,h[i]);
	}
	i =0;
	int index;
	
	for(int i =0; i<PQ_SIZE; i++)
	{
		index =minvalue(&Heap).value;
		strcpy(ch[i].name,dns[index].name);
		strcpy(ch[i].alias,dns[index].alias);
		strcpy(ch[i].ip,dns[index].ip);
		ch[i].hit = dns[index].hit;
		remove_pq(&Heap);
	}

}
struct node * createNode(char* key, char *name, int hit,char *alias) {
    struct node *newnode;
    newnode = (struct node *) malloc(sizeof(struct node));
    strcpy(newnode->key,key);
    newnode->hit = hit;
    strcpy(newnode->name, name);
    strcpy(newnode->alias,alias);
    newnode->next = NULL;
    return newnode;
}
void insertToHash(char* key, char *name, int hit,char* alias,struct hash* hashTable,int line) {
    int hashIndex = atoi(key) % line;
    struct node *newnode = createNode(key, name, hit,alias);

    /* head of list for the bucket with index "hashIndex" */

    if (!hashTable[hashIndex].head) {
        hashTable[hashIndex].head = newnode;
        hashTable[hashIndex].count = 1;
        return;

    }
    /* adding new node to the list */

    newnode->next = (hashTable[hashIndex].head);

    /*

     * update the head of the list and no of

     * nodes in the current bucket

     */
    hashTable[hashIndex].head = newnode;
    hashTable[hashIndex].count++;

    return;

}

 
void deleteFromHash(char* key,struct hash* hashTable,int line) {

    /* find the bucket using hash index */
    int hashIndex = atoi(key) % line, flag = 0;

    struct node *temp, *myNode;

    /* get the list head from current bucket */

    myNode = hashTable[hashIndex].head;

    if (!myNode) {
        printf("Given data is not present in hash Table!!\n");

        return;
    }

    temp = myNode;

    while (myNode != NULL) {
        /* delete the node with given key */

        if (strcmp(myNode->key,key)==0) {

            flag = 1;

            if (myNode == hashTable[hashIndex].head)
                hashTable[hashIndex].head = myNode->next;

            else
                temp->next = myNode->next;


            hashTable[hashIndex].count--;

            free(myNode);
            break;
        }

        temp = myNode;

        myNode = myNode->next;

    }

    if (flag)

        printf("Data deleted successfully from Hash Table\n");

    else

        printf("Given data is not present in hash Table!!!!\n");

    return;

}

void searchInHash(char* key,struct hash* hashTable,int line,int clnt,cache *ch) {

    printf("top10\n");
    char buf[BUF_SIZE];
    char quit = 'q';
    int exit = 0;
    for(int i =0; i<PQ_SIZE; i++)
    {
	if(strcmp(key,ch[i].ip)==0 || strcmp(key,ch[i].name)==0){
		ch[i].hit++;
		sprintf(buf,"%17s%12s%19s%3d", ch[i].name, ch[i].alias, ch[i].ip, ch[i].hit);
		write(clnt,buf,strlen(buf));
		write(clnt,&quit,1);
		return;
	}
    }

		
    int hashIndex = atoi(key) % line, flag = 0;
    char noSearch[100] = "Search element not in DNS table ";

    struct node *myNode;

    myNode = hashTable[hashIndex].head;

    if (!myNode) {

        //write(clnt,noSearch,strlen(noSearch));
	//printf("hash strlen : %ld\n",strlen(noSearch));
    }
    while (myNode != NULL) {

	if (strcmp(myNode->key,key)==0) {

		    sprintf(buf,"%15s%12s%15s%3d", myNode->key, myNode->name, myNode->alias, myNode->hit);
		    write(clnt,buf,51);
		
		    flag = 1;

		    break;

		}

		myNode = myNode->next;

    }
	//write(clnt,noSearch,strlen(noSearch));
	//printf("hash strlen : %ld\n",strlen(noSearch));
	//exit++;
    if (!flag)
        write(clnt,noSearch,51);
    write(clnt,&quit,1);

    return;

}

 
void display(struct hash* hashTable,int line) {

    struct node *myNode;

    int i;

    for (i = 0; i < line; i++) {

        if (hashTable[i].count == 0)

            continue;

        myNode = hashTable[i].head;

        if (!myNode)

            continue;

        printf("\nData at index %d in Hash Table:\n", i);

        printf("IP     Name          Hit       Alias\n");

        printf("------------------------------------------\n");

        while (myNode != NULL) {

            printf("%-12s", myNode->key);

            printf("%-15s", myNode->name);

            printf("%d", myNode->hit);
	    printf("%12s\n", myNode->alias);

            myNode = myNode->next;

        }

    }

    return;

}
void makeHashIPTable(int line,DNSTable* dns,struct hash * hashTable)
{

    for(int i =0;i<line;i++)
    {
	insertToHash(dns[i].ip,dns[i].name, dns[i].hit,dns[i].alias,hashTable,line);
    }


}
void makeHashDNSTable(int line,DNSTable* dns,struct hash * hashTable)
{

    for(int i =0;i<line;i++)
    {
	insertToHash(dns[i].name,dns[i].ip, dns[i].hit,dns[i].alias,hashTable,line);
    }


}
