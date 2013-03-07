/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
	STATWORD ps;
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}


	/*To change runtime priority during Aging scheduling policy */
	if(getschedclass()==1 && pptr->pstate==PRREADY)
	{
		dequeue(pid);
		insert(pid,rdyhead,newprio);
	}

	pptr->pprio = newprio;
	restore(ps);
	return(newprio);
}
