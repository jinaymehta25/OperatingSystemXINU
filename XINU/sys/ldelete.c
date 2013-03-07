#include <conf.h>
#include <kernel.h>
#include <lock.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>

/* ldelete.c
 * Deletes a lock with its lock descriptor
 *  */

 
 int ldelete(int lockdescriptor)
{
	STATWORD ps;    
	int j;
	int	pid;
	struct	lentry	*lptr;

	disable(ps);
	if (isbadlck(lockdescriptor) || slockarr[lockdescriptor].lstate==LFREE) {
		restore(ps);
		return(SYSERR);
	}
	lptr = &slockarr[lockdescriptor];
	/* Releasing all locking parameters */
	lptr->lstate = LFREE;
	lptr->nreaders = 0;
	lptr->lckprio = 0; /* priority of lock holding process    */
	for(j=0;j<NPROC;j++)
	lptr->acl[j]=-1;
	
	if (nonempty(lptr->lwhead)) {
		while( (pid=getfirst(lptr->lwhead)) != EMPTY)
		  {
		    proctab[pid].pwaitret = DELETED;
			proctab[pid].plock = -1;
			ready(pid,RESCHNO);
		  }
		
	resched();
	}
	
	restore(ps);
	return(OK);
}
