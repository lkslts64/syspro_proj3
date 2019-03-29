

//queue
typedef struct {
	struct listnode *start;			//start of queue
	struct listnode *end;			//end of queue
	struct listnode *unique;		//list with all distinct URL's processed.
}pool_t;






struct listnode {
	char URL[2083];				//URL has its own limits. Its 2083 chars
	struct listnode *next;
};


struct metadata {
	int pages;
	long bytes;
}

