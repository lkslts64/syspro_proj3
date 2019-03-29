

//pool
 typedef struct listnode {
	int fd;
	struct listnode *next;
}pool_t ;


struct metadata {
	int pages;
	long bytes;
}

