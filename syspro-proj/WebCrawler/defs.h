
;struct listnode;
struct sockaddr;
struct sockaddr_in;
//crawler.c
char * removePool(void);
int InsertPool(char *);
void place(char *url);
char * obtain(void);
void initialize(void);
void Crawler(void);
int isUnique(struct listnode *new);
void *ProduceConsume(void *argp);
void printPool(void);


//sock.c
int Connect(void);
void Command(void);
void checkThreads(void);


//request.c
char * sendRequest(char *url,int sock);
void findLinks(char *text);
int strToInt(char *text);

//command.c
void getCommand(int fd);

struct filenode;
struct trienode;
struct pollfd;
//namedpipes.c
void create_npipe(pid_t pid,pid_t deadpid);
void create_npipesJE(int *pids,int w);
int openFifoReadWorker(char *basepath);
int openFifoWriteWorker(char *basepath);
int openFifoReadJE(char *basepath,unsigned long pid);
int openFifoWriteJE(char *basepath,unsigned long pid);
//workers.c
int Worker(int ln,int bool);
int noDigits(int i);
//jobExecutor.c
int jobExecutor(void);
void findDir(char *buff,int n,char *temp);
void storeDir(char **buff,char *temp,int ln);
int dirStats(char **buff,int ln);
char ** findDirs(FILE *fp,int *linecount);
void getDirFromPid(int l,int w,int pidindex,int *indexes);
void assignDirs(int *indexes,int l,char **dirs,char *buff);
//createworkers.c
pid_t * createWorkers(int w,int ln);
void createWorker(pid_t pid,int l,int w,char **dirs,pid_t *pids,int *writefd,int *readfd,int bool,char *input);
int findPids(pid_t pid,int w,pid_t *pids);
//file.c
char **loadFiles(char **buff,int l,int *counter,long *msize,char ***fnames,long *wc);
int seperateFiles(char **map,char **fnames,int count,int msize,int *sumlines,int *sumwords);
//trienode.c
void trieInit(void);
void trieInsert(char *key,int line,int offset,int index);
struct postingNode *getPostingNode(int line,int offset,int index);
int postingInsert(struct trienode *p,int flag2,int line,int offset,int index);
int freePosting(struct postingNode *post);
struct postingNode *trieSearch(char *key);
int maxcountQuery(struct postingNode *post,int count,int *max,char **fnames,char *buff,int logfd);
int mincountQuery(struct postingNode *post,int count,int *min,int wc,char **fnames,char *buff,int logfd);
int searchQuery(struct filenode *arr,struct postingNode *post,int arrsize,int logfd,char **fnames,char *buff );
void dealloc(void);
void freeTrie(struct trienode *p);
//navigator.c
int navigateJE(int *readfd,int *writefd,char **buff,int w,long buffsize,pid_t *pids,int l,char **dirs,char *input1);
int navigateWorker(int readfd,int writefd,int count,char *buffer,char **fnames,int maxline,long buffsize,long wc,char **map,int logfd,int sumlines,int sumwords);
//queriesWorker.c
void search(char **querys,int noqrs,int arrsize,long buffsize,char **map,char **fnames,int writefd,int logfd,int *qcount);
void MinMaxCount(char *keyword,char *how,int count,int wc,char **fnames,char *buffer,int logfd,int *qcount);
void storeLine(char *buff,char *map,int index);
//queriesJE.c
int getMaxBuffsize(int *readfd,char **buffsize,int w,int *writefd);
void sendQeury(char *input);
void ReceiveMaxcount(int *readfd,int w,long buffsize,int *writefd);
void ReceiveMincount(int *readfd,int w,long buffsize,int *writefd);
void ReceiveWc(int *readfd,int w,int *writefd);
void ReceiveSearch(char *input);
int polling(int bool,char *input);
int polling2(int bool,char *input);
void ReceiveExit(void);
//log.c
void logInit(char *keyword,char *queryType,int logfd);
void createLog(int w,pid_t *pids);



