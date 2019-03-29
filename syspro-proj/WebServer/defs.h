
;
struct sockaddr;
//sock.c
int createSockets(void);
int decideAccept(struct sockaddr *clientptr,int *clientlen,int *sockfds,int *fdtoret);
void checkThreads(void);


//threadHTTP.c
void initialize(pool_t **pool,int nthr);
void *Server(void *argp);
void *consumer(void *ptr);
int obtain(pool_t **pool);
void *producer(int fd);
void *place(pool_t **pool,int fd);
int removePool(pool_t **pool);
void InsertPool(pool_t **pool,int fd);

//request.c
int getRequest(int fd);
char *createResponse(int filefd,int size);

//command.c
void getCommand(int fd);

