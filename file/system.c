#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#define BUF_SIZE 30
//-rw-rw-r-- 1 jyb5858 jyb5858 63524864 10월 21 20:29 data.d
typedef struct Path{
	char dir[15];
	int num;
	char server_name[15];
	char server_subname[15];
	long byte;
	char month[6];
	int day;
	char time[7];
	char file_name[20];
}Path;
//196	242	3	881250949
typedef struct User{
	int number;
	int kind;
	int job;
	long movie;

}User;

int main(int argc,char* argv[])
{
	FILE *fp;
	char buf[BUF_SIZE];
	int read_cnt;
	Path path[15];
	User user[10000];
	chdir("dataFile");
	int re =system("ls -al > imsi");
	int i =system("sed 1d imsi> imsi2");
	fp = fopen("imsi2","r");
	
	
	while(!feof(fp)) {
    		if(fgets(buf, BUF_SIZE, fp) == NULL) break;//한 줄씩 읽다가 마지막에 읽은 내용이 없으면 루프를 빠져나옴
		printf("%s",buf);
	}






	//while(!feof(fp))
	//{	
	//	if(fgets(bufStr, MAX, fp) == NULL) break;    //한 줄씩 읽다가 마지막에 읽은 내용이 없으면 루프를 빠져나옴
	//	fscanf(fp,"%s %d %s %s %ld %s %d %s %s",path[i].dir,&path[i].num,path[i].server_name,path[i].server_subname,&path[i].byte,path[i].month,&path[i].day,path[i].time,path[i].file_name);
	//	printf("%d %ld %s %d %s %s\n",path[i].num,path[i].byte,path[i].month,path[i].day,path[i].time,path[i].file_name);
	//}
	
	fclose(fp);
	return 0;
}
