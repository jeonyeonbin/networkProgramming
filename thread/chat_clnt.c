#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
	
#define BUF_SIZE 100
#define NAME_SIZE 20
#define FILE_SEND_PORT 9999
#define FILE_RECV_PORT 8888

#define OP_FAIL 0xa0
#define OP_SUCCESS 0xa1

#define OP_ROOM_CREATE 0x01
#define OP_ROOM_IN 0x02
#define OP_EXIT 0x03

#define OP_ROOM_KICK 0x04
#define OP_ROOM_CHAT 0x05
#define OP_ROOM_INFO 0x06
#define OP_ROOM_OUT 0x07

#define OP_ROOM_SEND 0x08
#define OP_ROOM_FILE 0x09
#define OP_ROOM_RECV 0x10
#define OP_FILE_OK   0x11
#define OP_FILE_BLOCK 0x12

typedef struct Menu{
	int number;
	char server_menu[BUF_SIZE];
}Menu;
typedef struct Chat{
	char room_name[BUF_SIZE]; //방 제목
	int max_people;            //최대 인원수
}Chat;
typedef struct SockInfo{
	int menu;                //메뉴 번호
	pthread_t thread;
	int sock;   		 //클라이언트 소켓
	char ip[BUF_SIZE];      // 서버측 ip
	char user_name[BUF_SIZE]; //user 이름
}SockInfo;
typedef struct FileInfo{
	int sock;
	char file_name[BUF_SIZE];
	FILE* fp;
}FileInfo;

int flag;
int file_send_port;
int file_recv_port;
int room_cnt;
char room_index[BUF_SIZE][BUF_SIZE];
void * send_file(void *arg);
void * send_msg(void * arg);
void * recv_msg(void * arg);
void error_handling(char * msg);
int menu_select(int sd);
void room_create(SockInfo sockInfo);
void write_data(int socket, const char *data);
char* read_data(int socket);
void * menu_handling(void * arg);
void room_insert(SockInfo sockInfo);
void * recv_file(void * arg);
void room_info_print(int sock);
int get_file_size(FILE *fp);
FileInfo file_sock_send(SockInfo sockInfo);
FileInfo file_sock_recieve(SockInfo sockInfo);
FILE* file_send();
void * recv_file(void *arg);
char name[NAME_SIZE]="[DEFAULT]";
char msg[BUF_SIZE];
void clearBufferRead() 		    //버퍼에잇는거지우기
{
	getchar();
}
int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	
	pthread_t clnt_thread;
	void * thread_return;
	SockInfo sockInfo;	

	if(argc!=4) {
		printf("Usage : %s <IP> <port> <name>\n", argv[0]);
		exit(1);
	 }
	
	sprintf(sockInfo.user_name, "[%s]", argv[3]);
	sock=socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	  
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");
	
	//회원가입및 로그인 해야댐	
	printf("name : %s\n",sockInfo.user_name);
	//처음에 닉네임 전송
	write_data(sock,sockInfo.user_name);
	
	//thread 생성을 하기위한 정보들 나열		
	sockInfo.sock = sock; //socket 정보
	strcpy(sockInfo.ip,argv[1]); //ip정보
	printf("Server Socket  :%s\n",sockInfo.ip);

	pthread_create(&clnt_thread,NULL,menu_handling,(void*)&sockInfo);
	pthread_join(clnt_thread,&thread_return);

	close(sock);  
	return 0;
}
	

void menu_print() // 메뉴보여주기 함수
{
	printf("============================================================\n");
	printf("1.대화방 개설\t2.대화방 입장\t3.종료\t\n");
	printf("============================================================\n");
	printf("\n");
	printf("====>원하시는 메뉴번호를 입력해주세요 :");
}
int menu_select(int sd)
{
	Menu menu;

	menu_print();// 메뉴 보여주기창
	scanf("%d",&menu.number);// 메뉴번호를 입력받는다.

	sprintf(menu.server_menu,"%d",menu.number);//길이 복사
	write_data(sd,menu.server_menu);
	return menu.number;
}
void write_data(int socket, const char *data)
{
    int data_len = 0;           // 보내야할 전체 데이터 크기
    
    data_len = strlen(data);
    
    write(socket, (void *)&data_len, sizeof(int));  // 전체 데이터 크기 전송
    
    write(socket, (void *)data, data_len);  // 데이터 전송
    
    return;
}

char* read_data(int socket)
{
    int data_len = 0;           // 받아야할 전체 데이터 크기
    int rev_cnt, rev_len = 0;   // 받은 데이터 크기, 전체 받은 데이터 크기
    char *buf = NULL;           // 데이터 저장 버퍼
    
    read(socket, (void *)&data_len, sizeof(int));    // 전체 데이터 크기 수신
    
    buf = (char *)malloc((data_len + 1) * sizeof(char));    // 버퍼 동적할당
    
    while(rev_len < data_len) {
        rev_cnt = read(socket, (void *)((char *)buf + rev_len), 1);
        
        if(rev_cnt == -1)
            error_handling("read() error!");
        
        rev_len += rev_cnt;
    }
    
    buf[data_len] = '\0';   // 문자열 끝 표시

    return buf;
}
void * menu_handling(void * arg)
{
	SockInfo sockInfo; // 소켓 정보 전송하기위한 변수
	sockInfo=*((SockInfo*)arg);
	
	int sock = sockInfo.sock;
	int count =0;
	int menu;
	char buf[2];
	while(1){
		menu = 0;
		//종료후에  나오기 
		// 밑에 if 문 해주지않고 buffer 지우고 나서하기
		if(count == 0)//버퍼 처음에는 없는데 두번째부터 버퍼처리해야댐!!
		{
			menu = menu_select(sock);
			count++;
		}
		else{
			getchar();
			menu = menu_select(sock);			
			printf("%d\n",menu);
		
		}
		switch(menu){
			case OP_ROOM_CREATE:
				room_create(sockInfo);//방생성
				printf("나왔습니다!\n");	
				break;
			case OP_ROOM_IN:	
				room_insert(sockInfo);//방입장
				printf("나왔습니다!\n");			
				//clearBufferRead();//입력버퍼 지우기
				break;
			case OP_EXIT:
				sprintf(buf,"%d",OP_EXIT);	
				printf("종료합니다!!\n");
				write_data(sock,buf);
				return NULL;
			default:
				printf("잘못된 입력입니다!!!\n다시입력해주세요\n");
				break;

		}
	}
	return NULL;
	
}

void room_create(SockInfo sockInfo)
{
	Chat chat;  	//클라이언트가 서버측에게 전송하기위한 것
	char send_max[4];
	char* clientMessage;

	int sock = sockInfo.sock;
	int success_;
	//방 제목 먼저 전달
	printf("방제목을 입력해주세요 : ");
	clearBufferRead();
	//system("clear");

	scanf("%[^\n]",chat.room_name);
	printf("방제목 : %s\n",chat.room_name);
	write_data(sock,chat.room_name);
	
	//최대인원수는 0< max < 256
	do{
		printf("최대인원수를 입력해주세오 : ");
		clearBufferRead();
		scanf("%d",&chat.max_people);
	}while(chat.max_people>256 || chat.max_people<0);
	
	getchar();
	//클라이언트가 서버에게 최대인원수 전달
	sprintf(send_max,"%d",chat.max_people);
	printf("최대 인원수  : %s\n",send_max); 
	write_data(sock,send_max);

	//클라이언트에게 석세스인지 아닌지	
	clientMessage = read_data(sock);
	
	success_ = atoi(clientMessage);
	
	if(success_ == OP_SUCCESS)
	{
		printf("%s 방이 성공적으로 생성되었습니다!!\n",chat.room_name);
		free(clientMessage);
		//TODO: 채팅방으로 들어가야댄다!! 쓰레드 두개 생성해야될거같다!
		pthread_t snd_thread, rcv_thread;
		void * thread_return;

		sockInfo.thread = rcv_thread;
		pthread_create(&snd_thread, NULL, send_msg, (void*)&sockInfo);
		sockInfo.thread = snd_thread;
		pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sockInfo);
		//pthread_join(snd_thread, &thread_return);
		pthread_detach(snd_thread);
		pthread_join(rcv_thread,&thread_return);
		printf("나갑니다!\n");

		return ;
	}
	else{
		free(clientMessage);
		printf("방 생성에 실패하였습니다. 다시 선택해주세요!!\n");	
	}
	return ;
}

void room_info_print(int sock)
{
	char* buf;
	char room_info[2];
	int i;
	//char room_name[BUF_SIZE][BUF_SIZE];
	sprintf(room_info,"%d",OP_ROOM_INFO); // room에 대한정보
	
	//방에대한 정보출력
	printf("=========================ROOM INFO=====================\n");
	printf("  방 번호      방제         현재인원수         최대인원수 \n");
	printf("=======================================================\n");

	i=0;
	while(1)
	{
		//제목
		buf = read_data(sock);
		if(strcmp(buf,room_info)==0)
			break;
		printf("  %d ",i+1);
		
		printf("%15s\t\t",buf);
		
		strcpy(room_index[i],buf);
		free(buf);
	
		//현재인원수		
		buf = read_data(sock);
		printf("%2s명 \t",buf);
		free(buf);

		//최대인원수
		buf = read_data(sock);
		printf("\t%3s명\n",buf);
		free(buf);
		i++;
	}
	if (i == 0)
		printf(" %  입장할수 있는 방이 없습니다. \n");
	printf("=========================================================\n");
	room_cnt = i;

}
void room_insert(SockInfo sockInfo)
{
	//방정보들 먼저 받기 => 들어갈 방번호 전송 => 들어갈수있는지 플래그 받기 => 쓰레드 대화시작
	char* buf;
	//char room_index[BUF_SIZE][BUF_SIZE];
	char room_name[BUF_SIZE];
	int success_;
	int sock = sockInfo.sock;
	int r_index;
	
	//system("clear");
	room_info_print(sock); //room에대한 정보 출력
	if(room_cnt > 0) {
		while(1) {
			printf("방 번호를 적어주세요 :");
			getchar();
			scanf("%d",&r_index);

			if(r_index > 0 && r_index < room_cnt)
				break;

			getchar();
		}
	}
	else
		return;
	//클라이언트가 서버에게 방이름 전송
	printf(" room name : %s\n",room_index[r_index-1]);
	fflush(stdout);
	write_data(sock,room_index[r_index-1]);
	buf = read_data(sock); //서버에게 성공인지 아닌지 받기위한코드
	success_ = atoi(buf);

	if(success_ == OP_SUCCESS)
	{
		//system("clear");
		printf("어서오세요 !! %s 방입니다\n",room_index[r_index-1]);
		free(buf);
		//TODO: 채팅방으로 들어가야댄다!! 쓰레드 두개 생성해야될거같다!
		pthread_t snd_thread, rcv_thread;
		void * thread_return;
		
		sockInfo.thread = rcv_thread;
		pthread_create(&snd_thread, NULL, send_msg, (void*)&sockInfo);
		sockInfo.thread = snd_thread;
		pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sockInfo);
		//pthread_join(snd_thread, &thread_return);
		pthread_detach(snd_thread);
		pthread_join(rcv_thread,&thread_return);
		printf("나갑니다!\n");	
		return ;
	
	}
	else{
		
		free(buf);
		printf("방 입장에 실패하였습니다!\n");	
	}

	return ;
	
}
// fp가 가리키는 파일의 크기 반환
int get_file_size(FILE *fp)
{
	int file_len;

	fseek(fp, 0, SEEK_END);		// 파일포인터를 파일의 끝으로 이동
	file_len = ftell(fp);		// 파일의 시작부터 파일포인터의 위치까지 거리
	rewind(fp);					// 파일포인터를 시작점으로

	return file_len;
}
void * send_msg(void *arg)   // send thread main
{
	SockInfo sockInfo = *((SockInfo*)arg);
	int sock= sockInfo.sock;
	char msg[BUF_SIZE] = "님이 입장하셨습니다.";
	char file_name[BUF_SIZE];
	char msg_header[2];
	char* flag_str;
	FileInfo fileInfo;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);

	sprintf(msg_header,"%d",OP_ROOM_CHAT);
	write_data(sock,msg_header);
	write_data(sock,msg);

	while(1) 
	{
		sleep(3);
		if(flag ==1){
			flag =0;
			return NULL;
		}
		scanf("%[^\n]",msg);
		getchar();
		if(strcmp(msg,"!file")==0)
		{
			sprintf(msg_header,"%d",OP_ROOM_SEND);
			write_data(sock,msg_header); //데이터를 보낸다는 메세지를보내고 
			
			pthread_t clnt_thread;
			fileInfo = file_sock_send(sockInfo);			


			pthread_create(&clnt_thread,NULL,send_file,(void*)&fileInfo);
			pthread_detach(clnt_thread);
			memset(msg,0,sizeof(msg));

		}
		else if(strcmp(msg,"!exit")==0) 
		{

			//system("clear");
			printf("exit을입력하였습니다!\n");
			sprintf(msg_header,"%d",OP_ROOM_OUT);
			printf("msg_header :%s\n",msg_header);					
			write_data(sock,msg_header);
			sprintf(msg_header,"%d",OP_EXIT);
			write_data(sock,msg_header);	
			printf("종료합니다\n");
			pthread_cancel(sockInfo.thread);
			exit(1);
		}
		else{
			sprintf(msg_header,"%d",OP_ROOM_CHAT);
			write_data(sock,msg_header);
			write_data(sock,msg);
		}
	}
	return NULL;
}

FileInfo file_sock_send(SockInfo sockInfo)
{

	char* buf;
	char* opCode;
	char file_name[BUF_SIZE];
	int file_sock;

	//int port;
	//buf = read_data(sockInfo.sock);
	//port = atoi(buf);
	//free(buf);
	//printf("send port : %d\n",port);

	struct sockaddr_in file_serv_addr;
	FileInfo fileInfo; 

	file_sock=socket(PF_INET, SOCK_STREAM, 0);

	memset(&file_serv_addr, 0, sizeof(file_serv_addr));
	file_serv_addr.sin_family=AF_INET;

	file_serv_addr.sin_addr.s_addr=inet_addr(sockInfo.ip);
	
	sleep(1);	
	file_serv_addr.sin_port = htons(file_send_port);
	if(connect(file_sock, (struct sockaddr*)&file_serv_addr, sizeof(file_serv_addr))==-1)
		error_handling("connect() error");


	//회원가입및 로그인 해야댐	
	printf("name : %s\n",sockInfo.user_name);
	//처음에 닉네임 전송
	write_data(file_sock,sockInfo.user_name);
	
	printf("파일명을 입력해주세요:");
	scanf("%[^\n]",file_name);
	getchar();
	//파일명 보내기
	write_data(file_sock,file_name);

	//thread 생성을 하기위한 정보들 나열		
	fileInfo.sock = file_sock;
	strcpy(fileInfo.file_name,file_name);

	return fileInfo;
	
}

FileInfo file_sock_recieve(SockInfo sockInfo)
{
	pthread_t clnt_thread;
	void * thread_return;

	char* buf;
	char* opCode;
	char file_name[BUF_SIZE];
	int file_sock;

	//int port;
	//buf = read_data(sockInfo.sock);
	//port = atoi(buf);
	//free(buf);
	//printf("recv port : %d\n",port);


	struct sockaddr_in file_serv_addr;
	FileInfo fileInfo; 

	file_sock=socket(PF_INET, SOCK_STREAM, 0);

	memset(&file_serv_addr, 0, sizeof(file_serv_addr));
	file_serv_addr.sin_family=AF_INET;

	file_serv_addr.sin_addr.s_addr=inet_addr(sockInfo.ip);
	
	sleep(1);	
	file_serv_addr.sin_port=htons(file_recv_port);
	if(connect(file_sock, (struct sockaddr*)&file_serv_addr, sizeof(file_serv_addr))==-1)
		error_handling("connect() error");
	
	
	//file에대한 파일명 먼저보내주는지?
	buf =read_data(file_sock);
	//thread 생성을 하기위한 정보들 나열		
	fileInfo.sock = file_sock;
	strcpy(fileInfo.file_name,buf);
	free(buf);

	return fileInfo;
	
}

void * recv_msg(void * arg)   // read thread main
{
	SockInfo sockInfo=*((SockInfo*)arg);
	int sock = sockInfo.sock;
	char* msg;
	int str_len;
	FileInfo fileInfo;

	char opCode[2];
	sprintf(opCode,"%d",OP_ROOM_RECV);
	
	char Flagin[2];
	sprintf(Flagin,"%d",OP_FILE_OK);

	char block[2];
	sprintf(block,"%d",OP_FILE_BLOCK);
	char kick[2];
	sprintf(kick,"%d",OP_ROOM_KICK);
	char out[2];
	sprintf(out,"%d",OP_ROOM_OUT);

	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);

	while(1)
	{
		msg=read_data(sock);
		if(strcmp(msg,opCode)==0){
		    free(msg);
		    msg = read_data(sock);
		    file_recv_port = atoi(msg);
		    free(msg);	    
		
       		    pthread_t clnt_thread;
		    fileInfo = file_sock_recieve(sockInfo);			


	            pthread_create(&clnt_thread,NULL,recv_file,(void*)&fileInfo);
		    pthread_detach(clnt_thread);

		}
		else if(strcmp(msg,Flagin)==0){
			free(msg);
			msg = read_data(sock);
			file_send_port = atoi(msg);
			
			free(msg);

		}
		else if(strcmp(msg,block)==0){

		}
		else if(strcmp(msg,out)==0){

			printf("나가기!\n");
			flag =1;
			return NULL;
		}
		else{
			printf("%s\n",msg);
			free(msg);
		}
	}
	return NULL;
}
void * send_file(void *arg)
{
	FileInfo fileInfo = *((FileInfo*)arg);
	
	printf(" %s 전송을 시작하겠습니다\n",fileInfo.file_name);
	int socket = fileInfo.sock;
	FILE *fp;
	int file_len;
	int read_cnt;
	char buf[BUF_SIZE];

	fp=fopen(fileInfo.file_name, "rb"); 
	if (fp == NULL) {
		printf("\"%s\" file open error!\n", fileInfo.file_name);
		exit(1);
	}

	file_len = get_file_size(fp);
	write(socket, (void *)&file_len, sizeof(int));	// 파일 사이즈 전송

	while(1)
	{
		read_cnt=fread((void*)buf, 1, BUF_SIZE, fp);
		if(read_cnt<BUF_SIZE)
		{
			write(socket, buf, read_cnt);
			break;
		}
		write(socket, buf, BUF_SIZE);
	}
	flag =0;
	fclose(fp);
	close(fileInfo.sock);
	return NULL;
}

void * recv_file(void *arg)
{	
	FileInfo fileInfo = *((FileInfo*)arg);
	int file_len;				// 파일의 전체 크기
	int socket = fileInfo.sock;
	int rev_cnt, rev_len = 0;	// 받은 데이터 크기, 전체 받은 데이터 크기
	char buf[BUF_SIZE];			// 버퍼
	FILE *fp;	

	fp=fopen(fileInfo.file_name, "wb");
	if (fp == NULL) {
		printf("\"%s\" file open error!\n",fileInfo.file_name);
		exit(1);
	}

	read(socket, (void *)&file_len, sizeof(int));

	while(rev_len < file_len) {
		rev_cnt = read(socket, buf, BUF_SIZE);

		if (rev_cnt == -1)
			error_handling("read() error!");

		rev_len += rev_cnt;

		fwrite((void*)buf, 1, rev_cnt, fp);

		//print_statbar(rev_len, file_len);
	}
	printf("\n");

	puts("Received file data");
	fclose(fp);
	close(fileInfo.sock);

	return NULL;


}



void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
