/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <math.h>

#define AGINGSCHED 1
#define LINUXSCHED 2

unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);

void increment_priorty();


/*-----------------------------------------------------------------------
 * setschedclass  --  Set the value of the schedclass
 *------------------------------------------------------------------------
 */
void setschedclass (int sched_class)
{
	if(sched_class==AGINGSCHED)
	schedclass = sched_class;

	if(sched_class==LINUXSCHED)
	schedclass = sched_class;


}
/*-----------------------------------------------------------------------
 * getschedclass  --  gets the value of the schedclass
 *------------------------------------------------------------------------
 */
int getschedclass()
{
	return schedclass;
}

/*-----------------------------------------------------------------------
 * setrolloverthreshold  --  sets the value of the threshold
 *------------------------------------------------------------------------
 */
void setrolloverthreshold(int threshold)
{
	quatnumThreshold = threshold;
}

/*------------------------------------------------------------------------
 * aging_incrementpriority()  --  increment priority of all processes in ready queue
 *------------------------------------------------------------------------
 */
int aging_incrementpriority(int head,int tail)
{
	int	st;			/* runs through list		*/
	int	end;

	st = q[head].qnext;
	end = q[tail].qnext;
	while (st!=end)
	{
		q[st].qkey = min(99, ceil(q[st].qkey * 1.2));
		st = q[st].qnext;

	}

		return(OK);
}



/*-----------------------------------------------------------------------
 * create_new_epoch  --  Initializes values for new epoch
 *------------------------------------------------------------------------
 */
int check_epoch()
{

		int i;
		int flag = 0;

		for(i=1;i<NPROC;i++)
		{
			struct pentry	*proc = &proctab[i];

			if((proc->pstate==PRREADY || proc->pstate==PRCURR)&& proc->goodness!=0)
			{
				flag=1;
				break;
			}
		}


		if(flag==0)
		{



			for(i=NPROC-1;i>0;i--)  /* To maintain the order of creation of process */
			{
				struct pentry	*proc = &proctab[i];

				if(proc->pstate!=PRFREE)
				{
				if(proc->pstate==PRREADY)
				dequeue(i);
				proc->counter = proc->pprio;
				proc->goodness=proc->pprio+proc->counter;	//Goodness calculation
				insert(i,rdyhead,proc->goodness);
				}
			}


		}

		return OK;

}

/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int resched()
{
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */

	if(schedclass==0) /*Default Scheduling*/
	{
		/* no switch needed if current process priority higher than next*/

		if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
		   (lastkey(rdytail)<optr->pprio)) {
			return(OK);
		}

		/* force context switch */

				if (optr->pstate == PRCURR) {
					optr->pstate = PRREADY;
					insert(currpid,rdyhead,optr->pprio);
				}

		nptr = &proctab[ (currpid = getlast(rdytail)) ];
		nptr->pstate = PRCURR;		/* mark it currently running	*/

			/* remove highest priority process at end of ready list */


					#ifdef	RTCLOCK
						preempt = QUANTUM;		/* reset preemption counter	*/
					#endif

					ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

							/* The OLD process returns here when resumed. */

							return OK;



	}

	if(schedclass==AGINGSCHED)
	{



		aging_incrementpriority(rdyhead,rdytail);

		/* no switch needed if current process priority higher than next*/

		if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
				   (lastkey(rdytail)<optr->pprio)) {
					return(OK);
				}

				/* force context switch */

						if (optr->pstate == PRCURR) {
							optr->pstate = PRREADY;
							insert(currpid,rdyhead,optr->pprio);

						}


			nptr = &proctab[ (currpid = getlast(rdytail)) ];
			nptr->pstate = PRCURR;		/* mark it currently running	*/

				/* remove highest priority process at end of ready list */


						#ifdef	RTCLOCK
							preempt = QUANTUM;		/* reset preemption counter	*/
						#endif

						ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

								/* The OLD process returns here when resumed. */

						return OK;


	}

	if(schedclass==LINUXSCHED)
	{



		optr=&proctab[currpid];
		optr->counter=preempt;

		if(preempt==0)
		optr->goodness = 0;
		else
		optr->goodness = preempt + optr->pprio;


		/* no switch needed if current process' goodness is higher than next*/

		if ( ( optr->pstate == PRCURR)  &&(optr->goodness!=0)) {
					return(OK);
		}

								/* force context switch */

								if (optr->pstate == PRCURR) {
									optr->pstate = PRREADY;
									insert(currpid,rdyhead,optr->goodness);

								}

				check_epoch();	/* Start a new epoch? */

				nptr = &proctab[ (currpid = getlast(rdytail)) ];
				nptr->pstate = PRCURR;		/* mark it currently running	*/

				if (nptr->counter >=quatnumThreshold)
				   quant = floor(nptr->counter/2) + nptr->pprio;
				else
      				  quant = nptr->pprio;

		/* remove highest priority process at end of ready list */


							#ifdef	RTCLOCK
								preempt = quant;		/* reset preemption counter	*/
							#endif

							ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

									/* The OLD process returns here when resumed. */

							return OK;
	}


		return OK;


}
