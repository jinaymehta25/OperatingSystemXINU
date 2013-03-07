#include <conf.h>
#include <kernel.h>
#include <pa4.h>
#include <proc.h>

dsk_buffer_p get_write_buffer(struct devsw* pdev, int block_no);
int sort_buffer(struct devsw* pdev, int block_no);
/*
 * Part A 4/4. buf_put()
 * buf_put() processes each write request.
 * If policy is POLICY_WRITE_THROUGH,
 *     then find a buffer that already stores the block_no block, stores new data to it
 *     and finally store the data into disk.
 * Otherwise (POLICY_DELAYED_WRITE),
 *     then you have to handle requests based on the policy.
 */

STATWORD ps;
char buf[128];
int buf_put(struct devsw *pdev, int block_no, char *buffer, int policy) {
	dsk_buffer_p tmp;
	if(is_buf_closed) return SYSERR;
	switch(policy)
	{
	case POLICY_WRITE_THROUGH:
	check_buffer_concurrency(pdev, block_no);
	tmp = bphead;
	while(tmp!=NULL){
		if(tmp->block_no == block_no && tmp->pdev == pdev)
		{
			if(SYSERR == memncpy(tmp->data,buffer,tmp->size)) return SYSERR;		
			sort_buffer(pdev,block_no);
			break;
		}
		else
			tmp=tmp->next;
	}
	release_buffer(pdev, block_no);
	if(dskwrite(pdev,buffer,block_no,1)==SYSERR) return SYSERR;
	return OK;
	break;

	case POLICY_DELAYED_WRITE:
	check_buffer_concurrency(pdev, block_no);
	/* Cache Hit*/
	tmp = bphead;
	while(tmp!=NULL){
		if(tmp->block_no == block_no && tmp->pdev == pdev)
		{
			if(SYSERR == memncpy(tmp->data,buffer,tmp->size)) return SYSERR;
			tmp->dirty = 1;
			sort_buffer(pdev,block_no);
			release_buffer(pdev, block_no);
			return OK;
		}
		else
			tmp=tmp->next;
		}
	/* Cache Miss */
	
	if((tmp = get_write_buffer(pdev,block_no))==SYSERR) return SYSERR;
	if(SYSERR == memncpy(tmp->data,buffer,tmp->size)) return SYSERR;
	if(bphead == NULL)
		bphead = bptail = tmp;
	else
	{
	bptail->next = tmp;
	bptail = bptail->next;
	}
	release_buffer(pdev, block_no);
	return OK;
	}
}

dsk_buffer_p get_write_buffer(struct devsw* pdev, int block_no) {
	dsk_buffer_p tmp; 
	struct devsw* ev_pdev;
	int ev_block_no;
	if(buf_count>0)
	{
		
		tmp = buf_head;
		buf_head = buf_head->next;
		buf_count--;
		tmp->next = NULL;
		tmp->pdev = pdev;
		tmp->block_no = block_no;
		if(PA4_WRITE_POLICY == POLICY_DELAYED_WRITE)
			tmp->dirty = 1;
	}
	else
	{
	/*General policy for Buffer Eviction. Check for dirty bit  */

		if(bphead == NULL || bphead->next == NULL) return SYSERR;
		/* Check if a any process is accessing head before eviction*/
		check_buffer_concurrency(bphead->pdev, bphead->block_no);
	
		tmp = bphead;
		bphead = bphead->next;
		tmp->next = NULL;
	
		ev_pdev = tmp->pdev;		// Old pdev
		ev_block_no = tmp->block_no;	// Old block_no

		if(PA4_WRITE_POLICY == POLICY_DELAYED_WRITE && tmp->dirty == 1)
			if(dskwrite(tmp->pdev,(char*)tmp->data,tmp->block_no,1)==SYSERR) return SYSERR;

		tmp->pdev = pdev;
		tmp->block_no = block_no;
		if(PA4_WRITE_POLICY == POLICY_DELAYED_WRITE)	tmp->dirty = 1;
		release_buffer(ev_pdev, ev_block_no); /*Now lock on bphead is no longer reqd hence released*/
	}
	return tmp;
}

int sort_buffer(struct devsw* pdev, int block_no){
	dsk_buffer_p tmp_buf,prev_buf,ret_buf;
	if(PA4_BUFFER_REPLACEMENT == POLICY_FIFO)
		return OK;
	if(bphead->next == NULL && bphead->pdev == pdev && bphead->block_no == block_no)
		return OK;
	else if(bphead->block_no == block_no && bphead->pdev == pdev)
	{
		dsk_buffer_p ret_buf;
		ret_buf = bphead;
		bphead = bphead->next;
		ret_buf->next = NULL;
		bptail->next = ret_buf;
		bptail = bptail->next;
		return OK;
	}
	else if(bptail->pdev == pdev && bptail->block_no == block_no)
		return OK;
	else
	{
		prev_buf = tmp_buf = bphead;
		while(tmp_buf->next!=NULL)
		{
			if(tmp_buf->next->block_no == block_no && tmp_buf->next->pdev == pdev)
			{
				ret_buf = tmp_buf->next;
				prev_buf->next =  tmp_buf->next->next;
				ret_buf->next = NULL;
				bptail->next = ret_buf;
				bptail = bptail->next;
				return OK;
			}
			else
			{
					prev_buf = tmp_buf->next;
					tmp_buf = tmp_buf->next;
			}
		}
		return OK;
	}
}
