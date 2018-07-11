#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 100
void error_handling(char *message);
void print_line(void);
void explain_game(void);
typedef struct Seed{
	int money;  
	int bat;
	int win;
	int drawn;
	int lose;
	double rate;
}Seed;
int main(int argc, char *argv[])
{
	int sock;
	char message[BUF_SIZE];
	int str_len;

	struct sockaddr_in serv_addr;

	if(argc!=3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock=socket(PF_INET, SOCK_STREAM, 0);   

	if(sock==-1)
		error_handling("socket() error");
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error!");
	
	
	Seed sd; //seed
	sd.money =100;
	sd.bat = 10;
	sd.win = 0;
	sd.drawn = 0;
	sd.lose = 0;
	sd.rate = 0.0;
	

	explain_game();

	printf("배팅금액을 먼저 넣어주세요(100~500, 100단위만!):");
	scanf("%d",&sd.money);
	sd.bat = sd.money/10;
	print_line();
	while(1){
	
		
		str_len = read(sock, message, BUF_SIZE - 1);    //서버에서주는 가위바위보 읽기
		message[str_len] = 0;
		fputs(message, stdout);
		fflush(stdout);

		memset(message,0,BUF_SIZE); //먼저버퍼 초기화
		str_len = read(0,message, BUF_SIZE); // 키보드에서 입력하는 값
		
		//printf("strlen: %d\n",str_len);
		//나가기 취소하게됨 
		if(message[0]=='q'){
			print_line();
			printf("quit!!!!!!\n");
			printf("Win : %d\n",sd.win);
			printf("Lose: %d\n",sd.lose);
			printf("Draw: %d\n",sd.drawn);
			sd.rate = (double)sd.win/(sd.win+sd.lose)*100;
			printf("수령할 금액 : %d만원\n",sd.money);
			printf("게임이 종료되었습니다 승률은 %.2lf percent 입니다\n",sd.rate);
			print_line();
			print_line();
			write(sock,"q",1);
			sleep(1);
			close(sock);
			break;
		}
		message[str_len] = 0;//초기화
		write(sock, message, 1);//서버에게 내 가위바위보의 값을 보내준다.
		memset(message,0,BUF_SIZE); //먼저버퍼 초기화
		str_len = read(sock, message, BUF_SIZE - 1);// 서버측에서 이긴지 진지에대한 값을 읽는다.

		//이겻을떄!!!		
		if(str_len ==11){
			sd.money +=sd.bat;
			sd.bat *=2;
			sd.win +=1;
			printf("win :%d\n",sd.win);
		}
		//졋을떄!!!!!
		else if(str_len ==12){
			sd.money-=sd.bat;			
			sd.lose +=1;
			sd.bat /=2;
			if(sd.bat<=10)
				sd.bat=10;
			printf("lose :%d\n",sd.lose);
		}

		//비겻을떄!!!!
		else if(str_len ==17){
			sd.drawn +=1;
			printf("draw : %d\n",sd.drawn);
		}
		if(sd.money<=0)
		{
			print_line();
			printf("quit!!!!!!\n");
			printf("Win : %d\n",sd.win);
			printf("Lose: %d\n",sd.lose);
			printf("Draw: %d\n",sd.drawn);
			sd.rate = (double)sd.win/(sd.win+sd.lose)*100;
			printf("Seed Money를 모두 사용하셨습니다!!!\n");
			printf("다음기회를 이용해주세요!!!!!!!\n");
			printf("게임이 종료되었습니다 승률은 %.2lf percent\n",sd.rate);
			write(sock,"q",1);
			sleep(1);
			close(sock);
			break;
		}

		
		printf("money : %d만원\n",sd.money);
		printf("Batting 금액: %d만원\n",sd.bat);
		message[str_len] = 0; //초기화


		fputs(message, stdout);
		printf("\n");
		print_line();
	}

        close(sock);
        return 0;
}
void explain_game(void)
{
	print_line();
	printf("바다이야기에 오신것을 환영합니다.\n");
	printf("\n");
	printf("가위바위보 게임으로 가위 : 0 바위 : 1 ,보 :2\n");
	printf("\n");
	printf("첫 배팅금액은 100만원부터시작~~\n");
	printf("\n");
	printf("가위바위보 게임에서 이기시게되면 배팅이 2배!!!!!\n");
	printf("\n");
	printf("첫배팅금액의 10percent가 배팅금액입니다!!\n");
	printf("\n");
	printf("지면 배팅은 0.5배로 반으로 줄어듭니다!\n");
	printf("\n");
	printf("비기게 되면 배팅은 그대로!!!\n");
	printf("\n");
	printf("게임을 현재상태에서 끝내고싶다  그러면 q를 눌러주세요!!!\n");
	printf("\n");
	printf("시작하겠습니다 많이따구가세요~~\n");
	print_line();
	sleep(2);

}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
void print_line(void)
{
	printf("==================================================\n");
}
