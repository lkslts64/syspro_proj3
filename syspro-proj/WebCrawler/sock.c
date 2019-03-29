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
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include "types.h"
#include "defs.h"

extern char *hostip;
extern int commport,port,nthr;
extern volatile sig_atomic_t flag;
extern pthread_t *workers;


//use this func every time a thread wants to make a request to WebServer.
int Connect(void)
{
	char *tmp;	
	int sock,templen=1024,hres,herr;
	struct sockaddr_in server;
	struct sockaddr *serverptr = (struct sockaddr*)&server;
	struct hostent hostbuf,*hp;	



	tmp=malloc(templen*sizeof(char));
	

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		perror("error socket");
	/* Find server address */
	while ((hres = gethostbyname_r(hostip,&hostbuf,tmp,templen,&hp,&herr)) == ERANGE) {		//gethostbyname thread safe 
		templen=templen*2;
		char *tmp1=realloc(tmp,templen*sizeof(char));
		if(tmp1!=NULL)
			tmp=tmp1;
		else
			printf("ERROR\n");
	}
	if(hp==NULL){
		perror("Error at translation");
		exit(0);
	}
	server.sin_family = AF_INET;
	memcpy(&server.sin_addr, hp->h_addr, hp->h_length);
	server.sin_port = htons(port);
		
	if (connect(sock, serverptr, sizeof(server)) < 0)		//connect to server
		perror("error connect\n");
	
	free(tmp);
	return sock;
}




void Command(void)
{
	int sock2,clientlen,newsock,i=0;
	struct sockaddr_in client,comm;
	struct sockaddr *commptr=(struct sockaddr *)&comm;
	struct sockaddr *clientptr=(struct sockaddr *)&client;
	
	
		
	if((sock2=socket(AF_INET,SOCK_STREAM,0))<0)
		perror("socket");
	comm.sin_family = AF_INET; 
	comm.sin_addr.s_addr = htonl(INADDR_ANY);
	comm.sin_port = htons(commport); 
	if (bind(sock2, commptr, sizeof(comm)) < 0)
		perror("bind");
	/* Listen for connections */
	if (listen(sock2, 5) < 0) 
		perror("listen");
		
	while(!flag){
		checkThreads();
		if((newsock=accept(sock2,clientptr,&clientlen))>0)
			getCommand(newsock);		
	}
}

//just if thread are dead. There wont be any...
void checkThreads(void)
{
	int i,err;
	
	for(i=0;i<nthr;i++){
		if(!(err=pthread_tryjoin_np(workers[i],0))){
			printf("WE WILL CREATE\n");
			if((err=pthread_create(workers+i,0,ProduceConsume,(void *)i)))
				perror("error create");
		}
	}
		

}
