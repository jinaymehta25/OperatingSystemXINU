#include <kernel.h>
#include <pa4.h>


/*
 * Part A 2/4. buf_close()
 * buf_close() deallocates all the resources that buffer manager has used.
 */
void buf_close(void) {
	int i;
	dsk_buffer_p tmp,prev;
	STATWORD ps;
	disable(ps);
	
	is_buf_closed = 1;
	while(buf_head!=NULL)
	{
		tmp = buf_head;
		buf_head = buf_head->next;
		//tmp->next = NULL;
		freemem(tmp->data, sizeof(PA4_APP_BUFFER_SIZE));
		freemem(tmp, sizeof(dsk_buffer));	
	}

	tmp = bphead;
	while(tmp!=NULL)
	{
		check_buffer_concurrency(tmp->pdev, tmp->block_no);
		if(PA4_WRITE_POLICY == POLICY_DELAYED_WRITE && tmp->dirty == 1)
			dskwrite(tmp->pdev,(char*)tmp->data,tmp->block_no,1);
		release_buffer(tmp->pdev, tmp->block_no);
		
		freemem(tmp->data, sizeof(PA4_APP_BUFFER_SIZE));
		prev = tmp;
		tmp = tmp->next;
		freemem(prev, sizeof(dsk_buffer));
	
	}
	
	for(i=0;i<1024;i++)
	{
	sdelete(disk0_sem[i]);
	sdelete(disk1_sem[i]);
	}

	buf_head = bphead = bptail = NULL;
	buf_count = -1;
	restore(ps);
}
