#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define BUF_SIZE 30
int main(int argc,char *argv[])
{
	long number;
	float allsize; //file 전체사이즈
	float status;	//현재상태
	char msg[BUF_SIZE] = "1500";
	msg[strlen(msg)] = 0;
	
	number =atol(msg);
	printf("%ld\n",number);

	//allsize = number/100.0;
	//status = 30.0/(number/100.0);
	
	//while(status!=100.00){
	//	status +=status;
	//	printf("%lf\n",status);
	//}
	//전체바이트수 나누기 현재바이트수

	return 0;
}
