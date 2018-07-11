#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

void read_childproc(int sig)
{
	int status;
	pid_t id=waitpid(-1, &status, WNOHANG);
	if(WIFEXITED(status))
	{
		printf("Removed proc id: %d \n", id);
		printf("Child send: %d \n", WEXITSTATUS(status));
	}
}

int main(int argc, char *argv[])
{
	pid_t pid;
	struct sigaction act;
	act.sa_handler=read_childproc;
	sigemptyset(&act.sa_mask);
	act.sa_flags=0;
	sigaction(SIGCHLD, &act, 0);
	
	int count = 0;

	pid=fork();
	if(pid==0)
	{
		puts("Hi! I'm First child process");
		sleep(10);
		return 12;
	}
	else
	{
		count++;
		printf("Child %d proc id: %d \n",count, pid);
		pid=fork();
		if(pid==0)
		{
			puts("Hi! I'm Second child process");
			sleep(10);
			exit(24);
		}
		else
		{
			count++;
			int i;
			printf("Child %d proc id: %d \n",count, pid);
			for(i=0; i<5; i++)
			{
				puts("wait...");
				sleep(5);
			}
		}
	}
	return 0;
}

/*
root@my_linux:/home/swyoon/tcpip# gcc remove_zombie.c -o zombie
root@my_linux:/home/swyoon/tcpip# ./zombie
Hi! I'm child process
Child proc id: 9529 
Hi! I'm child process
Child proc id: 9530 
wait...
wait...
Removed proc id: 9530 
Child send: 24 
wait...
Removed proc id: 9529 
Child send: 12 
wait...
wait...

*/
