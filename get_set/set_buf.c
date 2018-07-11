#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void error_handling(char *message);

int main(int argc, char *argv[])
{
	int sock;
	int snd_buf; 
	int rcv_buf;
	int ttl_buf;
	
	int state;
	socklen_t len;

/*기존의 방식 사용 했을때의 값*/
	sock=socket(PF_INET, SOCK_STREAM, 0);	
	len=sizeof(snd_buf);
	state=getsockopt(sock, IPPROTO_IP, IP_TTL, (void*)&snd_buf, &len);
	if(state)
		error_handling("getsockopt() error");
	
	len=sizeof(rcv_buf);
	state=getsockopt(sock, IPPROTO_IP, IP_TTL, (void*)&rcv_buf, &len);
	if(state)
		error_handling("getsockopt() error");
	
	printf("TTL buffer size: %d \n", snd_buf);
	

/* 그다음 버퍼를 3072로 값을 햇을때의 값 */
//	sock=socket(PF_INET, SOCK_STREAM, 0);
	snd_buf=128, rcv_buf=128;
	
	state=setsockopt(sock, IPPROTO_IP, IP_TTL, (void*)&rcv_buf, sizeof(rcv_buf));
	if(state)
		error_handling("setsockopt() error!");
	
	state=setsockopt(sock, IPPROTO_IP, IP_TTL, (void*)&snd_buf, sizeof(snd_buf));
	if(state)
		error_handling("setsockopt() error!");
	
	len=sizeof(snd_buf);
	state=getsockopt(sock, IPPROTO_IP, IP_TTL, (void*)&snd_buf, &len);
	if(state)
		error_handling("getsockopt() error!");
	
	len=sizeof(rcv_buf);
	state=getsockopt(sock, IPPROTO_IP, IP_TTL, (void*)&rcv_buf, &len);
	if(state)
		error_handling("getsockopt() error!");
	
	printf("TTL buffer size: %d \n", snd_buf);
	return 0;
}
void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
/*
root@com:/home/swyoon/tcpip# gcc get_buf.c -o getbuf
root@com:/home/swyoon/tcpip# gcc set_buf.c -o setbuf
root@com:/home/swyoon/tcpip# ./setbuf
Input buffer size: 2000 
Output buffer size: 2048 
*/


