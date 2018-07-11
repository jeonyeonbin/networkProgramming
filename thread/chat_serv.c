#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 100
#define MAX_CLNT 256

typedef struct ClntInfo{
	int clnt_sock;
	char ip_name[30];
	int menu;
}ClntInfo;
typedef struct Menu{		  // 서버와 클라이언트를 맞추기위한 메뉴 구조체
	int number;		  //클라이언트에서 넘어온 메뉴번호를 atoi 
	char clnt_num[BUF_SIZE];  //클라이언트에서 넘어오는 메뉴번호
}Menu;
void handle_clnt(ClntInfo clntInfo);
void send_msg(char * msg, int len);
void error_handling(char * msg);
void write_log(char *msg);
int clnt_cnt=0;
int select_menu(int clnt_sd);
int clnt_socks[MAX_CLNT];
void * menu_handling(void * arg);
void clearBufferRead() 		    //버퍼에잇는거지우기
{
	 while(getchar() !='\n');
}
pthread_mutex_t mutx;


int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id;
	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
  
	pthread_mutex_init(&mutx, NULL);
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET; 
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
	int breaking =0;
	while(1)
	{
		ClntInfo clntInfo;
		clnt_adr_sz=sizeof(clnt_adr);
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz);
		
		pthread_mutex_lock(&mutx);
		clntInfo.clnt_sock = clnt_sock;

		int menu = select_menu(clntInfo.clnt_sock);
		clntInfo.menu = menu;
		//각자의 아이피주소 넣기위한
		strcpy(clntInfo.ip_name,inet_ntoa(clnt_adr.sin_addr));
		clnt_socks[clnt_cnt++]=clnt_sock;
		pthread_mutex_unlock(&mutx);
		
		pthread_create(&t_id, NULL, menu_handling, (void*)&clntInfo);
		pthread_detach(t_id);
		write_log(inet_ntoa(clnt_adr.sin_addr));
		printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
	
	}
	close(serv_sock);
	return 0;
}
void write_log(char *msg)
{
	struct tm *t;
  	time_t timer;

  	timer = time(NULL);    // 현재 시각을 초 단위로 얻기
  	t = localtime(&timer); // 초 단위의 시간을 분리하여 구조체에 넣기


	char buf[BUF_SIZE];
	FILE *f;
	//pthread_mutex_lock(&mutx);
	if((f=fopen("Log.txt","a"))==NULL)
	{	
		printf("error\n");
		exit(1);
	}
	
	sprintf(buf,"%s 님이 %04d-%02d-%02d %02d:%02d:%02d에 입장하엿습니다.",msg,t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
	fprintf(f,"%s\n",buf);
	
	fclose(f);
	//pthread_mutex_unlock(&mutx);
}

void write_out_log(char *msg,int count)
{
	struct tm *t;
  	time_t timer;

  	timer = time(NULL);    // 현재 시각을 초 단위로 얻기
  	t = localtime(&timer); // 초 단위의 시간을 분리하여 구조체에 넣기


	char buf[BUF_SIZE];
	FILE *f;
	printf("%s 님이 나갔습니다\n",msg);
	if((f=fopen("Log.txt","a"))==NULL)
	{	
		printf("error\n");
		exit(1);
	}
	
	sprintf(buf,"%s 님이 %04d-%02d-%02d %02d:%02d:%02d에 %d번메세지를 보내고 퇴장하엿습니다.",msg,t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec,count);
	fprintf(f,"%s\n",buf);
	
	fclose(f);
}
	
void handle_clnt(ClntInfo clntInfo)
{
	//ClntInfo clntInfo;
	//clntInfo = *((ClntInfo*)arg);
	//int clnt_sock=*((int*)arg.clnt_sock);
	int clnt_sock = clntInfo.clnt_sock;
	int str_len=0, i;
	char msg[BUF_SIZE];
	int count = 0;
	
	printf("socket :%d\n",clntInfo.clnt_sock);
	while((str_len=read(clntInfo.clnt_sock, msg, sizeof(msg)))!=0){
		count++;	
		send_msg(msg, str_len);
		printf("각자의 카운트 : %d\n",count);
	}
	pthread_mutex_lock(&mutx);
	for(i=0; i<clnt_cnt; i++)   // remove disconnected client
	{
		if(clntInfo.clnt_sock==clnt_socks[i])
		{
			printf("socket :%d\n",clntInfo.clnt_sock);
			printf("socket IP :%s\n",clntInfo.ip_name);
			write_out_log(clntInfo.ip_name,count);
			while(i++<clnt_cnt-1)
				clnt_socks[i]=clnt_socks[i+1];
			break;
		}
	}
	pthread_mutex_unlock(&mutx);
	clnt_cnt--;
	close(clntInfo.clnt_sock);
	//return NULL;
}
void send_msg(char * msg, int len)   // send to all
{
	int i;
	pthread_mutex_lock(&mutx);
	for(i=0; i<clnt_cnt; i++)
		write(clnt_socks[i], msg, len);
	pthread_mutex_unlock(&mutx);
}
void error_handling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

void * menu_handling(void * arg)
{
	ClntInfo clntInfo;
	clntInfo = *((ClntInfo*)arg);
	int menu = clntInfo.menu;
	switch(menu){
			case 1: 
				handle_clnt(clntInfo);			
				//clearBufferRead();//입력버퍼 지우기
				break;
			case 2:	
				clearBufferRead();//입력버퍼 지우기
				break;
			default:
				break;
	}

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
//join => unsafe so use instead detach
