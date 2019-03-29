

//single linked list
struct postingNode{
	int line;	//number of line found the word
	int offset;		//offset of line from start of the file
	int index;	//we can find the path of file through index .path is -->fname[index]
	int no;			//number of times the word is found at this line of file=path
	struct postingNode *next;
};
typedef struct trienode{
	char letter;
	struct trienode *child;
	struct trienode *next;
	struct postingNode *posting;
	int endword;
	} Trienode;


struct filenode{
	int line;
	int index;
	int offset;
};


