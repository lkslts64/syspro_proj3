#define _GNU_SOURCE 
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <poll.h>
#include <pthread.h>
#include "types.h"
#include "defs.h"


extern pthread_mutex_t mtx3;

extern int serverport,commport,nthr;
extern pthread_t *workers;
extern volatile sig_atomic_t flag;


int createSockets(void)
{
	int sock,sock2,clientlen,newsock,i=0,sockfds[2];
	struct sockaddr_in server,client,comm;
	struct sockaddr *serverptr=(struct sockaddr *)&server;
	struct sockaddr *commptr=(struct sockaddr *)&comm;
	struct sockaddr *clientptr=(struct sockaddr *)&client;
	
	
	if((sock=socket(AF_INET,SOCK_STREAM,0))<0)
		perror("socket");
	server.sin_family = AF_INET; 
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(serverport); 
	if (bind(sock, serverptr, sizeof(server)) < 0)
		perror("bind");
	/* Listen for connections */
	if (listen(sock, 5) < 0) 
		perror("listen");
		
	if((sock2=socket(AF_INET,SOCK_STREAM,0))<0)
		perror("socket");
	comm.sin_family = AF_INET; 
	comm.sin_addr.s_addr = htonl(INADDR_ANY);
	comm.sin_port = htons(commport); 
	if (bind(sock2, commptr, sizeof(comm)) < 0)
		perror("bind");
	/* Listen for connections */
	if (listen(sock2, 100) < 0) 
		perror("listen");
			
	sockfds[0]=sock;
	sockfds[1]=sock2;	
		
		
		
	int fdtoret=-1;			//fd to return decideAccept()	
		
	while(!flag){
		checkThreads();
		clientlen = sizeof(client);
		newsock=decideAccept(clientptr,&clientlen,sockfds,&fdtoret);
		if(newsock>0){
			if(fdtoret==sock)
				producer(newsock);
			else if(fdtoret==sock2)
				getCommand(newsock);			//call command function
		}
	}
}

void checkThreads(void)
{
	int i,err;
	
	for(i=0;i<nthr;i++){
		if(!(err=pthread_tryjoin_np(workers[i],0))){
			printf("WE WILL CREATE\n");
			if((err=pthread_create(workers+i,0,consumer,(void *)i)))
				perror("error create");
		}
	}
		

}




//find what port is ready (http or command port) and return
//the fd that is ready and  the fd that will be used for communicaton. 
int decideAccept(struct sockaddr *clientptr,int *clientlen,int *sockfds,int *fdtoret)
{
	struct pollfd fds[2];
	int i,fd=-1;
	
	
	for(i=0;i<2;i++){
		fds[i].fd=sockfds[i];
		fds[i].events=POLLIN;
	}
	
	int res=poll(fds,2,-1);
	if(res<=0)
		return -1;
	else{
		for(i=0;i<2;i++)
			if(fds[i].revents & POLLIN){
				fd=fds[i].fd;
				break;
			}
	}
	
	if(fd<0)
		return fd;
	else{
		(*fdtoret)=fd;
		return accept(fd,clientptr,clientlen);
	}
}

