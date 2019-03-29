#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include "trienode.h"
#include "defs.h"
;
	struct trienode *root;

void trieInit(void)
{
	root=malloc(sizeof(struct trienode));
	root->endword=0;
	root->letter='0';
	root->next=NULL;
	root->child=NULL;
}
void trieInsert(char *key,int line,int offset,int index)
{
	int i,flag,flag2=0;	//flag2 for posting list and flag for allocating new trienode
	int length=strlen(key);
	struct trienode *p=root;	//parent pointer
	struct trienode *Childcurr=p->child;	//child pointer
	struct trienode *Childprev=Childcurr;
	for(i=0;i<length;i++){	
		flag=0;
		while(Childcurr!=NULL && (key[i]>=Childcurr->letter)){
			if(Childcurr->letter==key[i]){
				flag=1;
				break;
			}
			Childprev=Childcurr;		//we need this so we dont iterate again through the list (the list is sorted)
			Childcurr=Childcurr->next;
			
		}
		if(Childcurr!=NULL && flag)			//this if statement is about words like "pen" and "penalty"("penalty" is already in trie and we insert "pen")
			if(i==length-1 && (!(Childcurr->endword)))
				flag2=1;
		if((!flag)){
			flag2=1;		//an prostethei toulaxiston enas komvos , auto simainei pws h leksi einai kainouria
			struct trienode *tnode=malloc(sizeof(struct trienode));
			tnode->endword=0;
			tnode->letter=key[i];
			tnode->next=NULL;
			tnode->child=NULL;
			struct trienode *q=p->child;	//holds first child of parent ,like Childcurr before modified (Childcurr=Childcurr->next;). 
			if(q==NULL || (tnode->letter < q->letter)){			//xeirizomai diaforetika tin riza 
				tnode->next=q;
				Childcurr=p->child=tnode;
			}
			else{
				tnode->next=Childprev->next;
				Childcurr=Childprev->next=tnode;
			}
		}
		p=Childcurr;
		Childcurr=Childcurr->child;
		
	}
	p->endword=1;
	postingInsert(p,flag2,line,offset,index);
}
struct postingNode *getPostingNode(int line,int offset,int index)
{
	struct postingNode *pnode=malloc(sizeof(struct postingNode));
	pnode->line=line;
	pnode->offset=offset;
	pnode->index=index;
	pnode->no=1;
	pnode->next=NULL;
	return pnode;
}
int postingInsert(struct trienode *p,int flag2,int line,int offset,int index)
{
	
	struct postingNode *pnode;
	if(flag2){							//if no posting list is created then create postingFirst node and after the postingNode.
		struct postingNode *pnode=getPostingNode(line,offset,index);
		p->posting=pnode;
	}else{
		struct postingNode *post=p->posting;		//this wont be NULL surely.
		while(post!=NULL){
			if(post->index==index && post->line==line){		//no need to check offset ,we check only the line (offset fully depends on line). 
				post->no++;
				return 0;
			}
			post=post->next;
		}
		pnode=getPostingNode(line,offset,index);
		pnode->next=p->posting;
		p->posting=pnode;
		return 0;	
	}
}
int freePosting(struct postingNode *post)
{
	struct postingNode *temp;
	
	while(post!=NULL){
		temp=post;
		post=post->next;
		free(temp);
	}
	return 0;	
}
//search the trie for a keyword return 0 at failure or postinglist * when hit 
struct postingNode *trieSearch(char *key)
{
	int result=0;
	int i;
	int length=strlen(key);
	struct trienode *curr=root->child;
	struct trienode *prev;
	for(i=0;i<length;i++){
		while(curr!=NULL){
			if(curr->letter==key[i]){
				result=1;
				break;
			}
			curr=curr->next;
		}
		if(result==0){
			return 0;
		}
		prev=curr;
		curr=curr->child;
		result=0;
	}
	if(prev->endword)
		return (prev->posting);
	else
		return 0;
}
//search posting list for maxcount - call this func after search trie
int maxcountQuery(struct postingNode *post,int count,int *max,char **fnames,char *buff,int logfd)
{
	int *arr;	//array to store the max of every file
	int max1=0,i,temp;
	char *ret;
	
	arr=calloc(count,sizeof(int));
	
	while(post!=NULL){
		arr[post->index]+=post->no;
		post=post->next;
	}
	
	
	for(int i=0;i<count;i++)
		if(arr[i]>max1){
			max1=arr[i];
			temp=i;
		}
	sprintf(buff,"%s	(%d)\n",fnames[temp],max1);
	
	if(write(logfd,buff,strlen(buff))<0)
		perror("write error");
	free(arr);
	(*max)=max1;
	return temp;		//index at fnames (file that has most time the keyword)
}

//search posting list for mincount - call this func after search trie
//wc just to initialize min1
int mincountQuery(struct postingNode *post,int count,int *min,int wc,char **fnames,char *buff,int logfd)
{
	int *arr;	//array to store the max of every file
	int min1,i,temp;
	char *ret;
	
	min1=wc;
	arr=calloc(count,sizeof(int));
	
	while(post!=NULL){
		arr[post->index]+=post->no;
		post=post->next;
	}
	
	
	
	for(i=0;i<count;i++)
		if(arr[i]<min1 && arr[i]>=1){
			min1=arr[i];
			temp=i;
		}
	sprintf(buff,"%s	(%d)\n",fnames[temp],min1);
	
	if(write(logfd,buff,strlen(buff))<0)
		perror("write error");
	free(arr);
	(*min)=min1;
	return temp;		//index at fnames (file that has most times the keyword)
}
int searchQuery(struct filenode *arr,struct postingNode *post,int arrsize,int logfd,char **fnames,char *buff)
{
	int i;
	char *ret;		//for strstr
	
	while(post!=NULL){
		for(i=0;i<arrsize;i++){
			if(arr[i].line==-1)	{	//i must initialize arr with -1
				arr[i].line=post->line;
				arr[i].index=post->index;
				arr[i].offset=post->offset;
				break;
			}
			if((post->line==arr[i].line) && (post->index==arr[i].index))
				break;
		}
		if((ret=strstr(buff,fnames[post->index]))==NULL){
			strcat(buff,fnames[post->index]);
			strcat(buff," : ");
		}
		post=post->next;
	}
	buff[strlen(buff)]='\n';
	if(write(logfd,buff,strlen(buff))<0)
		perror("write error log");
	
}
void dealloc(void){
	freeTrie(root->child);
	free(root);
}
void freeTrie(struct trienode *p)
{
	if(p->child) freeTrie(p->child);
	if(p->next) freeTrie(p->next);
	freePosting(p->posting);
	free(p);
}
