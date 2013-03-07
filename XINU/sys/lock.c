#include <conf.h>
#include <kernel.h>
#include <lock.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>

 
 
int getmaxpriority(int head,int tail)
{
	int max = 0;
	int next;

	next = q[head].qnext;
	while (next !=tail)
    {
		if(max < (&proctab[next])->pprio)
			max = (&proctab[next])->pprio;
		if(max < (&proctab[next])->pinh)
			max = (&proctab[next])->pinh;

	next = q[next].qnext;
	}
	return max;
}

void addacl(int ldes,int pid)
{
	int i;
	for(i=0;i<NPROC;i++)
	{	
		if((&slockarr[ldes])->acl[i]==-1)
		{
			(&slockarr[ldes])->acl[i]=pid;
			break;
		}
	}
}

void rmacl(int ldes,int pid)
{
	int i;
	for(i=0;i<NPROC;i++)
		if((&slockarr[ldes])->acl[i]==pid)
		{
			(&slockarr[ldes])->acl[i] = -1;
			break;
		}
}

void addpacl(int ldes,int pid)
{
	int i;
	for(i=0;i<NLOCKS;i++)
	{
		if((&proctab[pid])->pacl[i]==-1)
		{
			(&proctab[pid])->pacl[i]=ldes;
			break;
		}
	}
}

void rmpacl(int ldes,int pid)
{
	int i;
	for(i=0;i<NLOCKS;i++)
	{
		if((&proctab[pid])->pacl[i]==ldes)
		{
			(&proctab[pid])->pacl[i]=-1;
			break;
		}
	}
}

int incrpriorites(int lck,int prio)
{
	int i;
	int pid;
	struct lentry *lptr;
	struct pentry *pptr;

	lptr = &slockarr[lck];
	lptr->lckprio = prio;

		for(i=0; i< NPROC; i++)
		if((-1 != (pid = lptr->acl[i])) && ((&proctab[pid])->pprio < prio) && ((&proctab[pid])->pinh < prio))
		{
			pptr = &proctab[pid];
			pptr->pinh = prio;
			if(pptr->plock!=0 && pptr->pwaitret == OK)
			{	
			incrpriorites(pptr->plock,prio);
			}
			
		}
	return OK;
}


/*
 * swinsert() : Modified insert function to insert as per priority
 */

int swinsert(int proc, int head, int key)
{
	STATWORD ps;
	disable(ps);
	int	next;			/* runs through list		*/
	int	prev;

	next = q[head].qnext;
	while (q[next].qkey < key)	/* tail has maxint as key	*/
    next = q[next].qnext;
  
  
  while((q[next].qkey == key) && ((&proctab[proc])->plwtype == WRITER) && ((&proctab[next])->plwtype == READER))
     next = q[next].qnext; 
	
  q[proc].qnext = next;
	q[proc].qprev = prev = q[next].qprev;
	q[proc].qkey  = key;
	q[prev].qnext = proc;
	q[next].qprev = proc;
	restore(ps);
	return(OK);
}

int nonqueueempty(int head)
{
   
	int next;
	next = q[head].qnext; 
   
   if(q[next].qkey == MAXINT)
    return 0;
    else return 1;
}

/* swait()
 * modified wait version for locks()
 * */ 

 int lwait(int lck,int priority)
 {
  STATWORD ps;
	struct	lentry	*lptr;
	struct	pentry	*pptr;
  
  int flag = 0;	/* Indicates when to enter a process in wait queue */
  
	disable(ps);
	if (isbadlck(lck) || (lptr= &slockarr[lck])->lstate==LFREE) {
		restore(ps);
		return(SYSERR);
	}
  
  
  if(((pptr = &proctab[currpid])->plwtype == READER) && (lptr->lock == 1))
	  {
		lptr->lock = 0;
		lptr->nreaders = 1;
		lptr->lstate = LOCKED_READ;
		addacl(lck,currpid);
		addpacl(lck,currpid);
		restore(ps);
		return(OK);
	  }

  if(lptr->lock == 1 && pptr->plwtype == WRITER)
	 {
		lptr->lock = 0;
		lptr->lstate = LOCKED_WRITE;
		addacl(lck,currpid);
		addpacl(lck,currpid);
		restore(ps);
		return(OK);
	 }
  
  if((pptr->plwtype == READER) && (lptr->lstate == LOCKED_READ))
  {
    if(nonqueueempty(lptr->lwhead) && (lastkey(lptr->lwtail)>= priority))
    {
      flag = 1;
    }
    else
    {
    flag = 0;
    lptr->nreaders++;
	addacl(lck,currpid);
	addpacl(lck,currpid);
	restore(ps);
	return(OK);
	}
  }
  else flag = 1;
	 

  
  if (flag == 1) 
	  {
		swinsert(currpid,lptr->lwhead,priority);  /* Priority based insert in wait q*/
		lptr->lckprio = getmaxpriority(lptr->lwhead,lptr->lwtail);
		pptr->pstate = PRWAIT;
		pptr->plock = lck;
		pptr->pwaitret = OK;
		incrpriorites(lck,lptr->lckprio);
		resched();
		restore(ps);
		return pptr->pwaitret;
	}

 }


/* lock.c
 * Acquisition of lock for read or write
 *  */
 //extern int ncount = NLCOUNT;
 
 
 int lock(int ldes1,int type, int priority)
 {
    STATWORD ps;
	int res =1;
    struct	lentry	*lptr;
    struct	pentry	*pptr;
    disable(ps);
	
    /* Lock is invalid or deleted*/
    if(isbadlck(ldes1) || (pptr = &proctab[currpid])->pwaitret == DELETED  || (lptr=&slockarr[ldes1])->lstate == LFREE)
    {
      restore(ps);
      return SYSERR;
    }
    
    if(type == READ)
      pptr->plwtype = READER;
    else  
      pptr->plwtype = WRITER;
 
	res = lwait(ldes1,priority);
    restore(ps);
	return (res);    
  }