#include <stdio.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include "types.h"
#include "defs.h"


extern char *savedir,*hostip,*startURL;
extern int commport,port,nthr;
pthread_mutex_t mtx;
pthread_cond_t cond_nonempty;		//cond var for pool
pool_t *pool;
pthread_t *workers;			
extern volatile sig_atomic_t flag;
int waithr;								//a shared variable . when this variable becomes equal to ntrh (number_of_threads) then exit thread.

void initialize(void)
{
	pool=malloc(sizeof(pool_t));
	pool->start=NULL;
	pool->end=NULL;
	pool->unique=NULL;
	waithr=0;
}


void Crawler(void)
{
	int i,err,status;
	initialize();
	pthread_mutex_init(&mtx,0);
	pthread_cond_init(&cond_nonempty,0);
	
	
	
	
	
	
	if((workers=malloc(nthr*sizeof(pthread_t)))==NULL){
		perror("malloc");
		exit(1);
	}
	
	place(startURL);
	
	for(i=0;i<nthr;i++)					//start threads
		if(err=pthread_create(workers+i,0,ProduceConsume,(void *)i))
			perror("pthread_create error ");
	
	
	
	
	Command();			//function for commands -if this function return then we have a SHUTDOWN
	pthread_cond_broadcast(&cond_nonempty);		//send signal to workers so they unblock to exit
	
	
	for(i=0;i<nthr;i++)
		if((err=pthread_join(workers[i],NULL)))		//join threads 
			printf("i am fucked\n");
	
	
	//free the unique list. The queue will be empty so no need to free
	
	pthread_mutex_destroy(&mtx);
	pthread_cond_destroy(&cond_nonempty);
	
	struct listnode *p=pool->unique;
	struct listnode *q;
	while(p!=NULL){
		q=p;
		p=p->next;
		free(q);
	}
	
	free(pool);
	free(workers);
}





//first see if there are elements available -> obtain-> connect() -> sendRequest() -> getanswer -> findlinks() ->place() to queue -> close(socket) -> go to start.
void *ProduceConsume(void *argp)
{	
	char *url,*response;
	//char *URL table;
	//int i;
	int sock;
	while(!flag){
		//sock=Connect();
		if((url=obtain())==NULL){
			break;
		}
		sock=Connect();							//connect only if thread obtain something from queue
		if((response=sendRequest(url,sock))!=NULL)				//send and getanswer
			findLinks(response);
		close(sock);						//close socket and reconnect for another connection - protocol
		free(url);
		free(response);
	}
	pthread_exit(0);
}
char * obtain(void)
{
	char *url;
	pthread_mutex_lock(&mtx);
	while(pool->start==NULL){
		waithr++;
		pthread_cond_wait(&cond_nonempty,&mtx);
		waithr--;
		if(flag){
			pthread_mutex_unlock(&mtx);
			return 0;
		}
	}
	url=removePool();
	pthread_mutex_unlock(&mtx);
	return url;
}
void place(char *url)
{
	pthread_mutex_lock(&mtx);
	InsertPool(url);
	pthread_mutex_unlock(&mtx);
	pthread_cond_broadcast(&cond_nonempty);
}
//insert at end of list.
int InsertPool(char *url)
{
	struct listnode *new=malloc(sizeof(struct listnode));
	
	
	struct listnode *p=pool->end;
	
	memset(new->URL,'\0',2083);
	strcpy(new->URL,url);
	
	if(isUnique(new)<0){
		free(new);
		return 0;
	}
		
	
	new->next=NULL;
	if(p!=NULL){
		while(p->next!=NULL)
			p=p->next;
		p->next=new;
		pool->end=new;
	}
	else{
		pool->start=new;
		pool->end=new;
	}
	return 0;	
	
}
		
//unique is a list with all unique URL's . (helping struct)
int isUnique(struct listnode *new1)
{
	
	
	
	struct listnode *u=pool->unique;
	while(u!=NULL){
		if(!strcmp(u->URL,new1->URL))
			return -1;
		u=u->next;
	}
	struct listnode *new=malloc(sizeof(struct listnode));
	memset(new->URL,'\0',2083);
	strcpy(new->URL,new1->URL);
	new->next=NULL;
	
	if(pool->unique==NULL)
		pool->unique=new;
	else{
		new->next=pool->unique;
		pool->unique=new;
	}
	return 0;
	
	
	
	
}	
//just for debugging.
void printPool(void)
{
	struct listnode *p=pool->start;
	
	printf("PRINTING POOL QUEUE\n");
	if(p==NULL)
		printf("POOL IS EMPTY!!!!\n");
	while(p!=NULL){
		printf("%s\n",p->URL);
		p=p->next;
	}
	
	p=pool->unique;
	printf("PRINTING POOL UNIQUE\n");
	while(p!=NULL){
		printf("%s\n",p->URL);
		p=p->next;
	}	
}
//remove first element of queue.
char * removePool(void)
{
	char *url=malloc(2083*sizeof(char));;
	memset(url,'\0',2083);
	strcpy(url,pool->start->URL);
	struct listnode *p=pool->start;
	if(pool->start->next!=NULL){
		pool->start=pool->start->next;
	}
	else{
		pool->start=NULL;
		pool->end=NULL;
	}
	free(p);
	return url;
}
