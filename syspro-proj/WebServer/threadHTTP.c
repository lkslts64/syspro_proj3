//the name of this file is very misleading...
#define _GNU_SOURCE 
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
#include "types.h"
#include "defs.h"


extern int nthr,serverport,commport;
extern char *rootdir;
pthread_mutex_t mtx;
pthread_cond_t cond_nonempty;
pthread_cond_t cond_nonfull;		//cond var for pool
pool_t *pool;			//singly linked list
pthread_t *workers;			
extern volatile sig_atomic_t flag;



void initialize(pool_t **pool,int nthr)
{
	(*pool)=NULL;
}


void *Server(void *argp)
{
	
	int serverport=(int)argp,i,err,status;
	
	
	initialize(&pool,nthr);
	pthread_mutex_init(&mtx,0);
	pthread_cond_init(&cond_nonempty,0);
	pthread_cond_init(&cond_nonfull,0);

	if((workers=malloc(nthr*sizeof(pthread_t)))==NULL){
		perror("malloc");
		exit(1);
	}
	
	for(i=0;i<nthr;i++)
		if(err=pthread_create(workers+i,0,consumer,(void *)i))
			perror("pthread_create error ");
	
	createSockets();		//if this functions returns means we have a shutdown command
	
	
	
	
	pthread_cond_broadcast(&cond_nonempty);		//send signal to threads so they unblock to exit
	
	
	
	
	
	for(i=0;i<nthr;i++)
		if((err=pthread_join(workers[i],(void **)status)))
			printf("i am fucked\n");
	
	
	pthread_mutex_destroy(&mtx);
	pthread_cond_destroy(&cond_nonempty);
	free(pool);
	free(workers);
	
}

void *consumer(void *argp)
{
	
	
	
	int fd;
	int id=(int)argp;
	pid_t tgid,tid;
	while(!flag){
		if((fd=obtain(&pool))<0){
			break;
		}
		getRequest(fd);
		close(fd);
		/*if(id==1){						//i think this doesnt works...
			printf("WE WILL KILL\n");
			tid = syscall(SYS_gettid);
			tid = syscall(SYS_tgkill, getpid(), tid,SIGINT);
		}*/
	}
	pthread_exit(0);	
} 



int obtain(pool_t **pool)
{
	int fd=0;
	pthread_mutex_lock(&mtx);
	while((*pool)==NULL){
		pthread_cond_wait(&cond_nonempty,&mtx);
		if(flag){									//if SHUTDOWN is sent then we should check the flag here...
			pthread_mutex_unlock(&mtx);
			return -1;
		}
	}
	fd=removePool(pool);
	pthread_mutex_unlock(&mtx);
	return fd;
}


void *producer(int fd)
{
	place(&pool,fd);							//just place the fd and 
	pthread_cond_broadcast(&cond_nonempty);		//inform ALL the consumers.
}

void *place(pool_t **pool,int fd)
{
	pthread_mutex_lock(&mtx);
	InsertPool(pool,fd);
	pthread_mutex_unlock(&mtx);
}



//just a linked list - NOT queue ( i 'll make the queue at crawler.
void InsertPool(pool_t **pool,int fd)
{
	
	pool_t *new=malloc(sizeof(pool_t));
	 pool_t *p=(*pool);
	
	new->fd=fd;
	new->next=NULL;
	
	if(p!=NULL){
		while(p->next!=NULL)
			p=p->next;
		p->next=new;
	}
	else
		(*pool)=new;
			
	
}
		
//just a linked list - NOT queue ( i 'll make the queue at crawler.	
int removePool(pool_t **pool)
{
	int fd=(*pool)->fd;
	pool_t *p=(*pool);
	(*pool)=(*pool)->next;
	if(p!=NULL)
		free(p);
	return fd;
}
