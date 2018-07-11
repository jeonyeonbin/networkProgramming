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
	int len;
	char buf[BUF_SIZE];
	FILE *fp1,*fp2;
	
	int fd2 = open("cpy.txt",O_WRONLY|O_CREAT|O_TRUNC);
	int fd1 = open("news.txt",O_RDONLY);
	
	fp1 = fdopen(fd1,"r");	
	fp2 = fdopen(fd2,"w");
		
	
	while(fgets(buf,BUF_SIZE,fp1) !=NULL){
		fputs(buf,fp2);	
	}

	fclose(fp1);
	fclose(fp2);


	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
