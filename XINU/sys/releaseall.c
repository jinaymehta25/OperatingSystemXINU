#include <conf.h>
#include <kernel.h>
#include <lock.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>


/* releaseall.c
 * Simultaneous release of multiple locks
 *  */
 static unsigned long	*ebp;

 SYSCALL release(int lck)
{
	STATWORD ps;    
	register struct	lentry	*lptr;
  int tpid,tprio;
	disable(ps);

	 /* Check and release*/
	 lptr = &slockarr[lck];
	 lptr->lckprio = 0;
   if ((0 == lptr->lock++) && nonqueueempty(lptr->lwhead)){
	  	  
		  if((&proctab[(q[(lptr->lwtail)].qprev)])->plwtype == READER)
			{ 	  
			  tpid = getlast(lptr->lwtail);
			  lptr->lock = 0;
			  proctab[tpid].plock = 0;
			  lptr->lstate = LOCKED_READ;
			  ready(tpid, RESCHNO);
			  lptr->nreaders = 1;
			  addacl(lck,tpid);
			  addpacl(lck,tpid);

			  while(nonqueueempty(lptr->lwhead) && (&proctab[(q[(lptr->lwtail)].qprev)])->plwtype == READER)
			  {
				   tpid = getlast(lptr->lwtail);
				   proctab[tpid].plock = 0;
				   lptr->nreaders++;
				   ready(tpid,RESCHNO);
				   addacl(lck,tpid);
				   addpacl(lck,tpid);
			  }
			 
			 if(nonqueueempty(lptr->lwhead))
				{
				 lptr->lckprio = getmaxpriority(lptr->lwhead,lptr->lwtail);
					incrpriorites(lck,lptr->lckprio);
				}
				resched();		   
			}
		else
			{	
			  tpid = getlast(lptr->lwtail);
			  addacl(lck,tpid);
			  addpacl(lck,tpid);
			  if(nonqueueempty(lptr->lwhead))
				{ 
					lptr->lckprio = getmaxpriority(lptr->lwhead,lptr->lwtail);
					incrpriorites(lck,lptr->lckprio);
				}	
			  proctab[tpid].plock = 0;
			  lptr->lock = 0;
			  lptr->lstate = LOCKED_WRITE;
			  ready(tpid, RESCHYES);
			}
  }
  else
	{
	  lptr->lstate = LUSED;
	}

  restore(ps);
  return(OK);
}


 int releaseall(int numlocks, ...)
 {  
    STATWORD ps;
    int i,j,res=1;
    int arg_ldes[numlocks];
    struct	lentry	*lptr;
    unsigned long *fp;
    
    disable(ps);
    
    /* Reading stack to get variable arguments */
    asm("movl %ebp,ebp");
		fp = ebp;
    fp = fp+3;  /* parmeters */	
    i=0;
	while(i<numlocks)
    {
      arg_ldes[i] = *fp;
      i++;
      fp++;		
    }
    
    /*checking all locks*/
	for(i=0;i<numlocks;i++)
	if (isbadlck(arg_ldes[i]) || (lptr= &slockarr[arg_ldes[i]])->lstate==LFREE  || proctab[currpid].pwaitret==DELETED) {
		restore(ps);
		return(SYSERR);
	}
	
	/* Releasing all the locks*/
    for(i=0;i<numlocks;i++)
    {
    
	lptr =  &slockarr[(arg_ldes[i])];

	/* Remove lock entry from pacl and pid from lock's acl*/
		rmpacl(arg_ldes[i],currpid);
		rmacl(arg_ldes[i],currpid);
		
		if(proctab[currpid].plock==-1)
		proctab[currpid].pinh = 0;

	if(lptr->lstate == LOCKED_READ && lptr->nreaders > 1)
      { 
	  lptr->nreaders--;
	  ready(currpid, RESCHYES);
      } 
    else
      {
	   res = release(arg_ldes[i]);
     }
    }    
   restore(ps);
   return res;
 }