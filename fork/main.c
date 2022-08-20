#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
FILE *fp;
int to_close;

void * waiter()
{
	pthread_detach(pthread_self());
	int status;
	wait(&status);
	if(WIFEXITED(status))
                printf("My child done its job and return %d\n", WEXITSTATUS(status));
	return 0;
}

void handler()
{
        puts("Signal from parent has come");
	fclose(fp);
	if(close(to_close) == -1)
		puts("Named pipe wasnt closed");
	else
		puts("All descriptors closed");
        exit(0);
}

void parent(int id, int spipe)
{
	puts("I am parent");
        puts("I am waiting for my child");
	pthread_t tid;
	int status;
	char string[100]={0};

	char * pfifo = "/tmp/pfifo";
	mkfifo(pfifo, 0777);
	int fifo_d=open(pfifo,O_RDONLY);
	while(1)
	{
		printf("Message: ");
		scanf("%s",string);
		if(strcmp(string,"end") == 0)
		{
			kill(id,SIGTERM);
			if(close(fifo_d) == -1)
				puts("Named pipe wasnt closed");
			break;
		}
		write(spipe,string,strlen(string));
		read(fifo_d,string,17);
		printf("Child send: %s\n",string);
	}
	wait(&status);
        if(WIFEXITED(status))
        {
                printf("My child done its job and return %d\n", WEXITSTATUS(status));
                if(unlink(pfifo) == -1)
                	puts("Delete named pipe error");
                else
                	puts("Named pipe was deleted");
        }
}

void child(int rpipe)
{
	signal(SIGTERM,handler);
	char string[100]={0};

	char * pfifo = "/tmp/pfifo";
        mkfifo(pfifo, 0777);
	int fifo_d=open(pfifo,O_WRONLY);

	fp = fopen("cache","a");
	to_close = fifo_d;
	while(1)
	{
		read(rpipe, string, 100);
		write(fifo_d, "Message recieved", 17);
		fprintf(fp,"Message: %s\n",string);
	}
}


int main(int argc, char *argv[])
{
	int id;
	int pipes[2];
	pipe(pipes);

	puts("Let's see");
	id = fork();
	if(id != 0)
	{
		close(pipes[0]);
		parent(id,pipes[1]);
	}
	else
	{
		close(pipes[1]);
		child(pipes[0]);
	}
	puts("End of work");
	return 0;
}
