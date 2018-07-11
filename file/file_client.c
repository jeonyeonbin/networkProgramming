#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30
//server 에서 가져올 파일구조체
typedef struct File{
	char file_name[BUF_SIZE];  //파일명
	char file_size[BUF_SIZE];  //파일사이즈
	char receive_name[BUF_SIZE];
	int length;  //파일사이즈를 계산하기위해 int로 변환
}File;
typedef struct Menu{
	int number;
	char server_menu[BUF_SIZE];
}Menu;

void clearBufferRead(); //입력버퍼 지우기
void file_select(int *sd,FILE *fp);
void path_read(int *sd);
int menu_select(int sd);
void menu_print();
void error_handling(char *message);

int main(int argc, char *argv[])
{
	int sd; //소켓통신을 위한 변수
	FILE *fp;  //파일 오픈을위핸 변수
	int menu_number;
	int len;
	char buf[BUF_SIZE]; //파일에대한 데이터를 한줄씩 받아낼수잇는 변수
	//double status; //현재상태
	//int read_cnt; //읽은횟수 퍼센티지로 나타내기위한 함수
	char dir[BUF_SIZE];
	

	struct sockaddr_in serv_adr;
	if(argc!=3) {
		printf("Usage: %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sd=socket(PF_INET, SOCK_STREAM, 0);   
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));

	connect(sd, (struct sockaddr*)&serv_adr, sizeof(serv_adr));
	for(;;){
		
		int menu = menu_select(sd);
		switch(menu){
			case 1: 
				clearBufferRead();//입력버퍼 지우기
				file_select(&sd,fp);//파일선택함수
				printf("\n");
				write(sd, "Thank you", 10);//마지막에 Thankyou라는 문자를보내 잘받앗다는 메세지전송
				break;
			case 2:	
				clearBufferRead();//입력버퍼 지우기
				printf("\n");
				path_read(&sd);//경로읽기 함수
				break;
			case 3:
				printf("====> 바꾸실 디렉토리명을입력하시오");
				clearBufferRead();//입력버퍼지우기
				scanf("%s",dir);// 경로명을 입력받는다.
				write(sd,dir,strlen(dir));//경로명을 서버에게 보낸다.
				read(sd,buf,BUF_SIZE);//서버에게 바뀌었다는 문자를 받는다.
				printf("%s \n",buf);
			default:break;
		}
	}
	
	
	close(sd);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
void menu_print() // 메뉴보여주기 함수
{
	printf("============================================================\n");
	printf("1.File Download\t2.Show Path\t3.Change Directory\t\n");
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
	
	write(sd,menu.server_menu,1);//파일메뉴전송	
	menu.server_menu[0]= '\0'; //서버에게보내는메뉴 문자열형태 초기화
	return menu.number;
}
void file_select(int *sd,FILE *fp)
{
	char buf[BUF_SIZE]; //파일에대한 데이터를 한줄씩 받아낼수잇는 변수
	int read_cnt; //읽은횟수 퍼센티지로 나타내기위한 함수
	double status; //현재상태
	File file;
	//////////파일명 입력하기///////	
	printf("받으실 파일명을 입력해주세요");
	memset(file.file_name,0,BUF_SIZE);// 버퍼지우기
	scanf("%s",file.file_name);
		
	printf("새로운 파일명을입력해주세요");
	memset(file.receive_name,0,BUF_SIZE);//버퍼지우기

	scanf("%s",file.receive_name);

	
	fp=fopen(file.receive_name, "w"); //데이터를 쓰기위함
	//원하는 파일명먼저보내기
	write((*sd),file.file_name,strlen(file.file_name)+1);
	
	//file size 먼저 read
	read((*sd),file.file_size,BUF_SIZE);
	
	//file.file_size[BUF_SIZE] =0;  //파일사이즈의 마지막 버퍼에 값을 널?
	file.length =atoi(file.file_size);  //파일사이즈의 길이를 다시 Long으로 바꿔야 값을 진행
	
	printf("length :%d\n",file.length);
	//현재 상태를 나타내줌
	int bufsize = BUF_SIZE;  
	status = (double)bufsize/(file.length/100.0);
	//file에 있는 내용 read	
	//fflush(stdout);

	// 값을 먼저계속읽는다. 허나 end 4개의 버퍼가왓을시 종료시켜야한다.
        // 그와관련된 코드는 밑에적혀있다.
	while((read_cnt=read((*sd), buf, BUF_SIZE))!=0)
	{
		//memset(buf,0,BUF_SIZE);
		buf[BUF_SIZE]='\0';
		//버퍼에 읽다가 버퍼가 4개짜리가 들어오면 종료시킨다.
		if(read_cnt ==4){
			printf("%s",buf);
			clearBufferRead();
			break;
		}
		//에러 발생시 종료
		if(read_cnt==-1){
		    error_handling("Read Error!!");
		}
		//파일에 읽은데이터들을 다써낸다
		fwrite((void*)buf, 1, read_cnt, fp);
		//현재상태 계속출력
		status = status+((double)read_cnt/(file.length/100.0));
				
		if(status>100.0) //버퍼에잇는값이 조금도올경우가잇다 데이터에는 헤더가잇으므로 그헤더의값이오는거같다?
			status =100.0;
		printf("\r %.2lf",status);
	}
	
	printf("\n");
	puts("Received file data");
	fclose(fp);
	return ;

}
void path_read(int *sd)
{
	int read_cnt;
	char buf[BUF_SIZE];
	
	while((read_cnt=read((*sd), buf, BUF_SIZE))!=0)
	{
		buf[read_cnt] = '\0';// 버퍼클리어
		
		if(read_cnt ==4) //위와같이 end 버퍼 4개의 버퍼가오게되면 종료
			return ;
		printf("%s",buf);//리스트들을계속 출력한다.
	}	
	printf("\n");
	
	return ;

}
void clearBufferRead()  //입력버퍼 초기화
{
	 while(getchar() !='\n');
}
