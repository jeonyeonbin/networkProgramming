#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	long length;
	char buffer[100];
	FILE *fp;
	fp=fopen("file_server.c", "rb");
	fseek(fp,0,SEEK_END);
	length = ftell(fp);
	rewind(fp);

	sprintf(buffer,"%ld",length);
	printf("buffer %s",buffer);	
	return 0;

}
