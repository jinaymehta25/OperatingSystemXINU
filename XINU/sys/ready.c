/* ready.c - ready */

#include <conf.h>
#include <kernel.h>
#include <lock.h>
#include <proc.h>
#include <q.h>

/*------------------------------------------------------------------------
 * ready  --  make a process eligible for CPU service
 *------------------------------------------------------------------------
 */
int ready(int pid, int resch)
{
	register struct	pentry	*pptr;

	if (isbadpid(pid))
		return(SYSERR);
	pptr = &proctab[pid];
	pptr->pstate = PRREADY;
	if(pptr->pinh==0)
	insert(pid,rdyhead,pptr->pprio);
	else
	insert(pid,rdyhead,pptr->pinh);
	if (resch)
		resched();
	return(OK);
}
