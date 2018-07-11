#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 51
#define QUIT 0
#define DOMAIN_SEARCH 1
#define IP_SEARCH 2
#define CACHE_SEARCH 3
typedef struct Menu{
	int number;
	char server_menu[BUF_SIZE];
}Menu;
void error_handling(char *message);
void read_routine(int sock, char *buf);
void write_routine(int sock, char *buf);
void menu_print();
int menu_select(int sd);
void read_cache(int sock);
void printLine();
void printCacheLine();
void searchingIP(int sock);
void searchingDomain(int sock);
void printSearchLine(char* line); 
int main(int argc, char *argv[])
{
	int sock;
	pid_t pid;
	char buf[BUF_SIZE];
	struct sockaddr_in serv_adr;
	if(argc!=3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock=socket(PF_INET, SOCK_STREAM, 0);  
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));
	
	if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("connect() error!");

	int flag =0;
	while(1)
	{
		//while문 탈출하기위한 조건!!!
		if(flag!=0)
			break;
		int menu = menu_select(sock);
		switch(menu){
			case QUIT:
				flag++;
				break;
			case DOMAIN_SEARCH:
				printLine();
				searchingDomain(sock);
				printLine();
				break;

			case IP_SEARCH:
				printLine();
				searchingIP(sock);
				//scanf("%s",buf);
				//write(sock,buf,strlen(buf));
				//buf[strlen(buf)-1] ='\0';
				//read_cache(sock);
				printLine();
				break;
			case CACHE_SEARCH:
				printCacheLine();
				read_cache(sock);
				printLine();
				break;
			default:
				printf("잘못된입력입니다 다시입력해주세요!!\n");
				break;
				
		}		
	}
	close(sock);

	return 0;
}


void read_routine(int sock, char *buf)
{
	while(1)
	{
		int str_len=read(sock, buf, BUF_SIZE);
		if(str_len==1){
			printf("the end\n");	
			return ;
		}
		printf("str_len :%d\n",str_len);
		buf[str_len]=0;
		printf("%s\n",buf);
	}
}
void read_cache(int sock)
{
	char buf[BUF_SIZE];
	int str_len;
	while(1)
	{
		str_len=read(sock, buf, BUF_SIZE);
		if(str_len==1){
			break;
		}
		buf[str_len]=0;
		printf("%s\n",buf);
	}
	write(sock, "Thank you", 10);//마지막에 Thankyou라는 문자를보내 잘받앗다는 메세지전송
}
void menu_print() // 메뉴보여주기 함수
{
	printf("============================================================\n");
	printf("1.Domain Search\t2.IP Search\t3.top 10 Search\t0.QUIT\n");
	printf("============================================================\n");
	printf("\n");
	printf("====>원하시는 메뉴번호를 입력해주세요 :");
}
void printLine()
{
	printf("============================================================\n");
}
void printCacheLine()
{
	printf("===========================TOP10============================\n\n");
	printf("   Domain Name\t\tAlias\t\tIPAddr  hit\n");
	printf("------------------------------------------------------------\n");
}
void printSearchLine(char* line)
{
	printf("=========================%s의 검색결과===================\n",line);

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

//자신의 주소 하나보내기!!!
void write_routine(int sock, char *buf)
{
	while(1)
	{
		int menu = menu_select(sock);
		switch(menu){
			case 0:
				return;
			case 1:
				while(1){
					fgets(buf, BUF_SIZE, stdin);
					if(!strcmp(buf,"q\n") || !strcmp(buf,"Q\n"))
					{	
						shutdown(sock, SHUT_WR);
						return;
					}
					write(sock, buf, strlen(buf));
				}
				break;
			case 2:
				printf("wemkwemlfwekmf\n");
				break;
			case 3:
				write(sock, "Thank you", 10);//마지막에 Thankyou라는 문자를보내 잘받앗다는 메세지전송
				return;
				
		}		
	}
}
void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
void searchingIP(int sock)
{
	char buf[BUF_SIZE];
	scanf("%s",buf);
	printSearchLine(buf);
	write(sock,buf,strlen(buf));
	buf[strlen(buf)-1] ='\0';
	read_cache(sock);
}
void searchingDomain(int sock)
{
	char buf[BUF_SIZE];
	scanf("%s",buf);
	printSearchLine(buf);
	write(sock,buf,strlen(buf));
	buf[strlen(buf)-1] ='\0';
	read_cache(sock);
}
