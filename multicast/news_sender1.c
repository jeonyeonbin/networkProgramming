#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>

#define TTL 64
#define BUF_SIZE 30
void error_handling(char *message);

int main(int argc, char *argv[])
{
	int fd1,fd2;
	int len;
	char buf[BUF_SIZE];
	
	fd1 = open("news.txt",O_RDONLY);
	fd2 = open("cpy.txt",O_WRONLY|O_CREAT|O_TRUNC);
	
	while((len = read(fd1,buf,sizeof(buf)))>0)
		write(fd2,buf,len);

	close(fd1);
	close(fd2);


	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
