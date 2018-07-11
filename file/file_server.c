#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30
typedef struct Menu{		  // 서버와 클라이언트를 맞추기위한 메뉴 구조체
	int number;		  //클라이언트에서 넘어온 메뉴번호를 atoi 
	char clnt_num[BUF_SIZE];  //클라이언트에서 넘어오는 메뉴번호
}Menu;
typedef struct File{		  //클라이언트와 서버간 데이터전송을위한 파일 구조체
	char filesize[BUF_SIZE];    // 파일사이즈
	char file_name[BUF_SIZE];   // 파일명
}File;
void clearBufferRead() 		    //버퍼에잇는거지우기
{
	 while(getchar() !='\n');
}
void error_handling(char *message);
int select_menu(int clnt_sd);		    //메뉴선택하는함수
void file_select(int *clnt_sd,FILE *path);  //파일선택하는 함수
void path_send(int *clnt_sd,FILE *path);    //현재경로보여주는함수
int main(int argc, char *argv[])
{
	int serv_sd, clnt_sd;
	FILE * fp; //파일전송하기위한 
	FILE *path;
	File file; //file 구조체
	Menu menu;  
	char buf[BUF_SIZE];
	int read_cnt;
	long length;
	struct sockaddr_in serv_adr, clnt_adr;
	socklen_t clnt_adr_sz;
	
	if(argc!=2) {
		printf("Usage: %s <port>\n", argv[0]);
		exit(1);
	}
	
		serv_sd=socket(PF_INET, SOCK_STREAM, 0);   
	
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family=AF_INET;
		serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
		serv_adr.sin_port=htons(atoi(argv[1]));
	
		bind(serv_sd, (struct sockaddr*)&serv_adr, sizeof(serv_adr));
		listen(serv_sd, 5);
	

		clnt_adr_sz=sizeof(clnt_adr);    
		clnt_sd=accept(serv_sd, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
/********************************************************************************************************/
/**********************************************************************************************************/
	for(;;){
		int select = select_menu(clnt_sd);
		switch(select){
			case 0:
				break;
			case 1:
				file_select(&clnt_sd,fp);//파일선택함수
				read(clnt_sd,buf,BUF_SIZE);//클라이언트에게 Thank you 라는 문자를받는다
				printf("Message from client: %s\n", buf);
				printf("\n");	
				break;
			case 2:
				path_send(&clnt_sd,path);//경로전송하는함수
				break;	
			case 3:				//클라이언트가원하는 디렉토리를 변하게끔도와줌
				buf[BUF_SIZE]='\0';//먼저 온 값을 초기화한다.
				read(clnt_sd,buf,BUF_SIZE); //폴더로 경로를바꿀 메세지를 먼저읽는다.
				chdir(buf); 		    //그경로로 변경
				printf("Change : %s\n",buf);
				write(clnt_sd,"Change",7); // 클라이언트에게 Change라는 값을보내폴더변경이 완료됫음을보낸다.
				//buf[BUF_SIZE]= '\0';
			default:break;
		}
		if(select ==0){
			error_handling("Client Error!\n");			
			break;
		}
	}	
	
	
	
	close(clnt_sd); close(serv_sd);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
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

void file_select(int *clnt_sd,FILE *fp)
{
	File file;
	long length;
	int read_cnt;
	char buf[BUF_SIZE];
	//file명을 먼저읽어온다.//////////////////////
	printf("read\n");
	read((*clnt_sd),file.file_name,BUF_SIZE); //파일명을읽어온다.
	printf("fileName:%s\n",file.file_name);	
	
	fp=fopen(file.file_name, "r");//읽은 파일명을 읽어 fopen시킨다.
	fseek(fp,0,SEEK_END);//끝까지 먼저돌려놔야 파일의 길이를 알아올수있다.
	length = ftell(fp); // 파일의 길이를 알아올수잇다.
	rewind(fp);  //file의 길이를 알려고 맨끝까지 포인터이동햇으니 다시 돌려놔야댄다.
	sprintf(file.filesize,"%ld",length);//길이 복사
	
	//file buffersize를 출력하기
	printf("buffer size : %s\n",file.filesize);
	
	//파일사이즈 마지막값은 널
	write((*clnt_sd),file.filesize,strlen(file.filesize)+1); //filesize를 보내준다.
	file.filesize[strlen(file.filesize)-1] = '\0';// 파일사이즈에대한값 다시초기화
		
	while(1)
	{
		//memset(buf,0x00,BUF_SIZE);
		read_cnt=fread((void*)buf, 1, BUF_SIZE, fp);// 버퍼만큼계속읽는다.
		buf[BUF_SIZE] = '\0';
		if(read_cnt!=BUF_SIZE)
		{
			//write((*clnt_sd), buf, read_cnt);
			break;
		}
		//printf("%s",buf);
		write((*clnt_sd), buf, BUF_SIZE);// 버퍼에서읽은값을 클라이언트에게전송
		buf[BUF_SIZE] = '\0';//버퍼값은 초기화
	}
	write((*clnt_sd),"end",4);//클라이언트에게 read데이터의 끝임을 알려준다
	printf("end\n");
	fclose(fp);//파일종료
	return ;
}
void path_send(int *clnt_sd,FILE *path)
{
	
	char buf[BUF_SIZE];
	int read_cnt;

	int re =system("ls -al > imsi");  //폴더의 모든데이터들을 보여줌
	printf("파일경로 먼저만들기 :%d\n",re);
	int i =system("sed 1d imsi> imsi2");  //첫행지우기 첫행=> 총합 12345 이런식의 파일크기? 써잇음
	printf("파일 임시파일 첫행지우기!%d\n",i);
	path = fopen("imsi2","r");

	//파일이 빈곳까지 다읽는다. 
	while(!feof(path)) {
		buf[BUF_SIZE] ='\0';  //먼저 보낼 버퍼초기
    		if(fgets(buf, BUF_SIZE, path) == NULL){  //마지막내용에 두줄이 읽어짐 허나 마지막두줄은 같은내용 겹친것이므로 마지막 줄 짤라서 보냄 
				printf("null");
				break;//한 줄씩 읽다가 마지막에 읽은 내용이 없으면 루프를 빠져나옴
		}
		printf("%s",buf);
		write((*clnt_sd),buf,BUF_SIZE);// 클라이언트에게 버퍼에잇는내용 바로 쓰기
	}
	write((*clnt_sd),"end",4);  //클라이언트에게 종료임을알려 클라이언트는 읽고나서 종료
	
	printf("end\n");
	fclose(path);//파일클로즈
	
	return ;

}
