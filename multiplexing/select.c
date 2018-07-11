#include<stdio.h>
#include<unistd.h>
#include<sys/time.h>
#include<sys/select.h>
#define BUF_SIZE 30

int main(int argc,char *argv[])
{
	fd_set reads, temps;
	int result, str_len;
	char buf[BUF_SIZE];
	struct timeval timeout;
	
	FD_ZERO(&reads);
	FD_SET(0,&reads);
	

	while(1){
		temps=reads;
		timeout.tv_sec=2;
		result = select(1,&temps,0,0,&timeout);
		
		if(result== -1)
		{
			puts("select() error!");
	     		break;
	  	}
	  	else if(result ==0)
	  	{
	    		puts("time-out!!\n");
	  	}
	  
	  	else{
			if(FD_ISSET(0,&temps)){
				str_len =read(0,buf,BUF_SIZE);
				if(buf[0] == 'q' || buf[0] =='Q'){
					
					printf("byebye\n");
					return 0;
				}
				buf[str_len] = 0;
				printf("message from console : %s\n",buf);		
			}
	  	}	
	}

	 return 0;
}
