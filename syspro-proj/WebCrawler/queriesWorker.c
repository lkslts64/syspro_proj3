#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "defs.h"
#include "trienode.h"

void search(char **queries,int noqrs,int arrsize,long buffsize,char **map,char **fnames,int writefd,int logfd,int *qcount)	//noqrs=number_of_querys , arrsize=maxline*count ,msize=megisto megethos arxeiou
{
	int i,dig;
	char *temp;
	struct postingNode *p;
	struct filenode *arr;
	
	char *buffer=malloc((arrsize*(512+4)+500)*sizeof(char));			//for log file
	memset(buffer,'\0',arrsize*(512+4)+500);
	arr=malloc(arrsize*sizeof(struct filenode));
	for(i=0;i<arrsize;i++){
		arr[i].line=-1;
		arr[i].index=-1;
		arr[i].offset=-1;
	}
	for(i=1;i<noqrs;i++){
		
		if((p=trieSearch(queries[i]))>0){
			(*qcount)++;
			logInit(queries[i],"search",logfd);
			searchQuery(arr,p,arrsize,logfd,fnames,buffer);
			memset(buffer,'\0',arrsize*(512+4)+500);
		}
	}
	char *buff=malloc(buffsize*sizeof(char));
	memset(buff,'\0',buffsize);
	
	if(arr[0].line==-1){		//if word is not found just pass newline (my protocol)
		buff[0]='\n';
		buff[1]='\n';
		if(write(writefd,buff,strlen(buff))<0)
			perror("write error searchvoid");
	}
	else{
		for(i=0;i<arrsize;i++){
			if(arr[i].line!=-1){
				sprintf(buff,"%d %s\n",arr[i].line,fnames[arr[i].index]);		//metadata
				storeLine(buff,map[arr[i].index]+arr[i].offset,strlen(buff));		//data
				if(i==(arrsize-1) || arr[i+1].line==-1)	{							//my protocol 
					buff[strlen(buff)]='\n';
				}									//store a newline (\n) if worker is done with passing files
				if(write(writefd,buff,strlen(buff))<0){
					perror("write error search");
				}
				memset(buff,'\0',buffsize);
			}	
		}
	}						
	free(arr);
	free(buff);
	free(buffer);
}

void storeLine(char *buff,char *map,int index)
{
	int i=0;
	
	while(map[i]!='\n'){
		buff[index]=map[i];
		i++;
		index++;
	}
	buff[index]='\n';
}




void MinMaxCount(char *keyword,char *how,int count,int wc,char **fnames,char *buffer,int logfd,int *qcount)
{
	struct postingNode *p;
	int value=0,findex;
	
	char *buff=malloc((count*(512+4)+500)*sizeof(char));		//for log file
	memset(buff,'\0',count*(512+4)+500);
	if((p=trieSearch(keyword))<=0)
		printf("Word %s not found.\n",keyword);
	else{
		(*qcount)++;
		if(!strcmp(how,"min")){
			logInit(keyword,"mincount",logfd);
			findex=mincountQuery(p,count,&value,wc,fnames,buff,logfd);
			memset(buff,'\0',count*(512+4)+500);
		}
		else{
			logInit(keyword,"maxcount",logfd);
			findex=maxcountQuery(p,count,&value,fnames,buff,logfd);
			memset(buff,'\0',count*(512+4)+500);
		}
	}
		
	sprintf(buffer,"%d %s",value,fnames[findex]);		
	free(buff);						//for log file
}



