#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
 #include <sys/time.h>
#include "defs.h"
#include "trienode.h"

//char *input1;			//what user typed.



/*extern pid_t *pids;
extern int l,ln,*writefd,*readfd,w;
extern char **dirs,**buffer;
extern struct pollfd *fds,*fds2;
extern long maxbuffsize;*/






int navigateJE(int *readfd,int *writefd,char **buffer,int w,long buffsize,pid_t *pids,int l,char **dirs,char *input1)
{
	char input[1000];
	char *delim=" \n";
	char *token[13];		//max 10 words at search query
	time_t initTimer;
	struct timeval start;		//select time_t or struct timeval 
	int i,temp=0;
	
	for(i=0;i<13;i++)
		token[i]=NULL;
	memset(input,'\0',1000);
		strcpy(input,input1);
		token[0]=strtok(input,delim);
		i=0;
		while(token[i]!=NULL && i<13){				//vale tis lekseis se ena pinaka
			i++;
			token[i]=strtok(NULL,delim);
		}
		if(!strcmp(token[0],"SEARCH")){				//gia to /search
			if(token[1]){
				printf("ABOUT TO SEND QUERY\n");
				sendQeury(input1);
				printf("SENDED QUERY\n");
				ReceiveSearch(input1);
			}
			else
				printf("wrong input.\n");
		}
		else if(!strcmp(token[0],"/exit")){			//gia to /exit
			sendQeury(input1);
			ReceiveExit();
			free(input1);
			return 0;
		}
		else{
			printf("wrong input.Try again\n");
		}
		
		
}

int navigateWorker(int readfd,int writefd,int count,char *buffer,char **fnames,int maxline,long buffsize,long wc,char **map,int logfd,int sumlines,int sumwords)
{
	char *delim=" \n";
	char *token[13],input[1000];		//max 10 words at search query
	int n,i,qcount=0;
	
	for(i=0;i<13;i++)
		token[i]=NULL;
	memset(input,'\0',1000);
	while(64){	
		
		if((n=read(readfd,input,1000))<0)
			perror("read error");
		i=0;
		token[0]=strtok(input,delim);
		while(token[i]!=NULL && i<13){				//vale tis lekseis se ena pinaka
			i++;
			token[i]=strtok(NULL,delim);
		}
		if(!strcmp(token[0],"/mincount")){
			MinMaxCount(token[1],"min",count,wc,fnames,buffer,logfd,&qcount);
			if(write(writefd,buffer,1000)<0)
				perror("write error mincount");
			
		}
		else if(!strcmp(token[0],"/maxcount")){
			MinMaxCount(token[1],"max",count,wc,fnames,buffer,logfd,&qcount);
			if(write(writefd,buffer,1000)<0)
				perror("write error maxcount ");
			
		}
		else if(!strcmp(token[0],"/wc")){
			sprintf(buffer,"%lu %d %d\n",wc,sumwords,sumlines);
			logInit(buffer,"wc",logfd);
			if(write(writefd,buffer,1000)<0)
				perror("write error wc");
		}
		else if(!strcmp(token[0],"SEARCH")){
			search(token,i,count*maxline,buffsize,map,fnames,writefd,logfd,&qcount);
		
		
		}
		else if(!strcmp(token[0],"/exit")){
			sprintf(buffer,"%d",qcount);
			if(write(writefd,buffer,strlen(buffer))<0)
				perror("write error");
			return 0;
		}
		memset(buffer,'\0',1000);
		memset(input,'\0',1000);
	}	
}


