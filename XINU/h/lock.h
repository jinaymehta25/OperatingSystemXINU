
/* Lock constants*/
#define DELETED  -6
#define READ     1
#define WRITE    2
#define READER 3
#define WRITER 4
#define LFREE 5
#define LUSED 6
#define LOCKED_READ 7
#define LOCKED_WRITE 8
#define NLOCKS 50

/*Lock Structure*/
struct lentry
{
	int lstate;
	int lock;	/* coresponds to mutex semaphore*/
	int lwhead;
	int lwtail;
	int acl[NPROC];
	int lckprio; /* Priority of process holding lock*/
	int nreaders;

};

#define	isbadlck(l)	(l<0 || l>=NLOCKS)

extern	struct	lentry slockarr[];
extern	int	nextlck;