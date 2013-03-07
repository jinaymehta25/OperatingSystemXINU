#include <kernel.h>
#include <pa4.h>


int shiftblocks(disk_desc *ptr, int block_no);
void before_sched(disk_desc *ptr)
{
request_desc_p target, prev;
		for(target = ptr -> request_head, prev = NULL;target -> next;prev = target, target = target -> next);
	//	kprintf("\nJinay : dskschedule as per FIFO - block_no = %d\n",target->block_no);
}

void printdisklink(disk_desc *ptr) {
	struct req_desc *disktemp;
	disktemp = ptr->request_head;
	kprintf("\nblock no->");
	kprintf("\nJinay : dskschedule - 0 | ptr = %d | ptr->request_head = %d | tmp->head_sector = %d | tmp->block_no = %d\n",ptr,ptr->request_head,ptr->head_sector,ptr->request_head->block_no);
	while (disktemp != NULL) {
		kprintf("-%d", disktemp->block_no);
		disktemp = disktemp->next;
	}
	kprintf("-ends\n");
}
/*
 * Part B 1/1. dskschedule()
 * This function should be implemented to support each disk scheduling policy.
 * Parameters:
 *     ptr:	a descriptor of each device information
 *     option:	a disk scheduling policy to be used
 */

void dskschedule(disk_desc *ptr, int option) {
	request_desc_p tmp;
	int dskhead, min,tmp_min,tmp_blk,nxt,flag;
	
	
	if(ptr == NULL || ptr->request_head == NULL || ptr->request_head->next == NULL || option == DISK_SCHED_FIFO)
	return;

		
		dskhead = ptr->head_sector;
//		before_sched(ptr);
//		printdisklink(ptr);
		switch(option)
		{
			case DISK_SCHED_SSTF:
//			kprintf("\nJinay : dskschedule - 1\n");
			tmp = ptr->request_head;
			min = ptr->logical_blocks * 10;
			while(tmp)
			{
			if(tmp->block_no > dskhead)
				tmp_min = tmp->block_no - dskhead;
			else 
				tmp_min =  dskhead - tmp->block_no;

			if(tmp_min < min) {
				min = tmp_min;
				tmp_blk = tmp->block_no;
				}
			tmp = tmp->next;
			}
			shiftblocks(ptr,tmp_blk);
//			kprintf("\nJinay : dskschedule - 2 | block_no = %d\n",tmp_blk);
			return;
			break;

			
			case DISK_SCHED_CLOOK:
//			kprintf("\nJinay : dskschedule - 3\n");
			tmp = ptr->request_head;
			flag = 1;
			nxt = min = ptr->logical_blocks * 10;
			
			while(tmp)
			{
			if(tmp->block_no < min)		
				min = tmp->block_no;
			if(tmp->block_no >= dskhead &&  tmp->block_no < nxt) 
				{
				nxt = tmp->block_no;
				flag = 0;
				}
			tmp = tmp->next;
			}
			if(flag)
			{	
				shiftblocks(ptr,min);
//				kprintf("\nJinay : dskschedule - 4 | block_no = %d\n",min);
			}else{
				shiftblocks(ptr,nxt);
//			kprintf("\nJinay : dskschedule - 4 | block_no = %d\n",nxt);
			}
			return;
			break;
		}

}

int shiftblocks(disk_desc *ptr, int block_no)
{
	request_desc_p target, prev, tmp_blk;
	int count = 0;
		/* In case of multiple requests take the 2nd one */

	target = ptr->request_head;
		while(target->next) 
		{
			if(target->block_no == block_no)
				count++;
			target = target->next;
		}

		if(target->block_no == block_no)
		return OK;
		
		if(count == 1){
//				kprintf("\nJinay : dskschedule - 1 | count = 1| block_no = %d\n",block_no);
		if((ptr->request_head)->block_no == block_no)
		{
			tmp_blk = ptr->request_head;
			ptr->request_head = tmp_blk->next;
			tmp_blk->next = NULL;
			
			target = ptr->request_head; 
			while(target->next) 
			target = target->next;
			target->next = tmp_blk;
			return OK;
		}	
		else
		{
			target = ptr -> request_head; 
			prev = NULL;
			
			while(target && target->next)
			{
				if(target->block_no == block_no)
				{
					tmp_blk = target;
					prev->next = target->next;
					tmp_blk->next = NULL;
					break;
				}
				else
				{
					prev = target;
					target = target -> next;
				}

			}
			target = ptr -> request_head;	
			while(target->next) 
			target = target->next;
			target->next = tmp_blk;
//kprintf("\nJinay : dskschedule - 2 | count = 1| block_no = %d\n",block_no);
				return OK;
		}
	}
		else if(count > 1)
		{
	//		kprintf("\nJinay : dskschedule - 3 | count = %d| block_no = %d\n",count,block_no);
			target = ptr -> request_head; 
			prev = NULL;
			//int flag = 0;
			while(target && target->next)
			{
				if(target->block_no == block_no)
				{
					if(count == 1)
					{	
					tmp_blk = target;
					prev->next = target->next;
					tmp_blk->next = NULL;
					break;
					}
					count--;
					prev = target;
					target = target -> next;
	
				}
				else
				{
					prev = target;
					target = target -> next;
				}

			}
			target = ptr -> request_head;	
			while(target->next) 
			target = target->next;
			target->next = tmp_blk;
//kprintf("\nJinay : dskschedule - 4 | count = 1| block_no = %d\n",block_no);
				return OK;
		}	
}