
#include <conf.h>
#include <kernel.h>
#include <lock.h>
#include <proc.h>
#include <sem.h>
#include <q.h>
#include <stdio.h>

/* linit.c
 * This program initializes the locks
 */

 int nextlck;
 struct	lentry slockarr[NLOCKS];
 void linit()
 {
    int i,j;
	struct lentry *lptr;
    nextlck = NLOCKS-1; 
	for (i=0 ; i<NLOCKS ; i++) {	/* initialize locks */
		(lptr = &slockarr[i])->lstate = LFREE;
		lptr->nreaders = 0;
		lptr->lock = 1;
		lptr->lckprio = 0; /* priority of lock holding process    */
		for(j=0;j<NPROC;j++)
		lptr->acl[j]=-1;
		lptr->lwtail = 1 + (lptr->lwhead = newqueue());
	}
 }
   

