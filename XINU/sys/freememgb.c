/*
 * freememgb.c
 *
 *  Created on: Oct 26, 2012
 *      Author: Jinay Mehta
 */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
SYSCALL freememgb(struct mblock *block, unsigned size)
{
	STATWORD ps;
	int result = SYSERR;
	struct pheapmemlist *templist, *removed_node;
	disable(ps);

	
	if(size > 0){
		templist = (&proctab[currpid])->phmemlist;
		while(templist->next !=NULL)
		{
			
			if(templist->next->phmemaddrs == block && templist->next->nbytes == size)
			{
			
			 removed_node = templist->next;
			 templist->next = removed_node->next;
			 result = freemem(block,size);
			 	disable(ps); /* To keep the function Interrupt Free*/
			 if(result == SYSERR) break;
			 freemem(removed_node,sizeof(struct pheapmemlist));
				disable(ps); /* To keep the function Interrupt Free*/
			 break;
			}
			else templist = templist->next;
		}
			
		restore(ps);
		return result;
	}
	else{
	templist = (struct pheapmemlist*)block;
		while(templist->next!=NULL)
		{
			
			removed_node = templist->next;
			templist->next = removed_node->next;
			result = freemem(removed_node->phmemaddrs, removed_node->nbytes);
			disable(ps); /* To keep the function Interrupt Free*/
			if(result == SYSERR) break;
			freemem(removed_node,sizeof(struct pheapmemlist));
			disable(ps); /* To keep the function Interrupt Free*/
			

		}
	templist = (struct pheapmemlist*)block;
	freemem(templist,sizeof(struct pheapmemlist));
	disable(ps); /* To keep the function Interrupt Free*/
	}
	restore(ps);
	return result;
}
