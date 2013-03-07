#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <q.h>
#include <stdio.h>

LOCAL int newlck();

/* lcreate.c
 * Creates a lock, initializes it and then returns lock descriptor
 *  */
 
 int lcreate()
 {
	STATWORD ps;    
	int	ldes;

	disable(ps);
	if ((ldes=newlck())==SYSERR ) {
		restore(ps);
		return(SYSERR);
	}
	slockarr[ldes].lock = 1;
	/* lwhead and lwtail were initialized at system startup */
	restore(ps);
	return(ldes);
 }



/*------------------------------------------------------------------------
 * newsem  --  allocate an unused semaphore and return its index
 *------------------------------------------------------------------------
 */
LOCAL int newlck()
{
	int	ldes;
	int	i;

	for (i=0 ; i<NLOCKS ; i++) {
		ldes=nextlck--;
		if (nextlck < 0)
			nextlck = NLOCKS-1;
		if (slockarr[ldes].lstate==LFREE) {
			slockarr[ldes].lstate = LUSED;
			//kprintf("\n Jin Ldes : %d\n",ldes);
			return(ldes);
		}
	}
	return(SYSERR);
}
