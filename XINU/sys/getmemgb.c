/*
 * getmemgb.c
 *
 *  Created on: Oct 26, 2012
 *      Author: Jinay Mehta
 */

#include <conf.h>
#include <kernel.h>
#include <proc.h>

WORD *getmemgb(unsigned nbytes)
{
	STATWORD ps;
	WORD *result;
	struct pheapmemlist *templist,*new_node;
	disable(ps);
	result = getmem(nbytes);
	disable(ps); /* To keep the function Interrupt Free*/
	if(result == SYSERR)
	{
		restore(ps);
		return(result);
	}


	new_node = getmem(sizeof(struct pheapmemlist));
	disable(ps); /* To keep the function Interrupt Free*/
	new_node->nbytes = nbytes;
	new_node->phmemaddrs = result;
	new_node->next = NULL;



	templist = (&proctab[currpid])->phmemlist;
	while(templist->next!=NULL)
		templist = templist->next;
	templist->next = new_node;

	restore(ps);
	return(result);
}

