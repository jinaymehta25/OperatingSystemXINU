#include <conf.h>
#include <kernel.h>
#include <pa4.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>

extern int dskread(struct devsw *pdev, char *buffer, int block_no, int count);
extern int dskwrite(struct devsw *pdev, char *buffer, int block_no, int count);

extern int memncpy(void *dest, void *src, int num);
extern int sort_buffer(struct devsw* pdev, int block_no);
dsk_buffer_p prefetch(struct devsw *pdev, int block_no);
dsk_buffer_p buf_fetch_fifo(struct devsw *pdev, int block_no);
dsk_buffer_p buf_fetch_lru(struct devsw *pdev, int block_no);
dsk_buffer_p get_read_buffer(struct devsw* pdev, int block_no);
dsk_buffer_p at_cache_miss(struct devsw* pdev, int block_no);
int check_buffer_concurrency(struct devsw *pdev, int block_no);
int release_buffer(struct devsw *pdev, int block_no);
int is_locked(struct devsw *pdev, int block_no);

char buf[128];
/*
 * Part A 3/4. buf_fetch()
 * buf_fetch() does:
 *     if there is a buffer that already store the block data of block_no,
 *         then return the buffer.
 *     otherwise
 *         read the block of block_no, store it on a buffer, and return the buffer.
 * 
 * parameters:
 *     pdev:		device descriptor
 *     block_no: 	a block number to read
 *     policy: 		buffer replacement policy
 */
dsk_buffer_p buf_fetch(struct devsw *pdev, int block_no, int policy) {
	int i;
	dsk_buffer_p res;
	//testing();/*Test*/
	if(is_buf_closed) return SYSERR;
	switch(policy)
	{
	case POLICY_FIFO:
	check_buffer_concurrency(pdev, block_no);
	
	res = buf_fetch_fifo(pdev,block_no);
	
	if(res == CACHE_MISS)//&& PA4_PREFETCH_SIZE==0) 
	res = at_cache_miss(pdev,block_no);
	release_buffer(pdev,block_no);
	
	if(PA4_PREFETCH_SIZE>0)
	for(i = 1;i<= PA4_PREFETCH_SIZE ; i++)
	if(block_no + i < 1024)	
	{
			check_buffer_concurrency(pdev, block_no + i);
			if(CACHE_MISS == buf_fetch_fifo(pdev,block_no+i))
			at_cache_miss(pdev,block_no + i);
			release_buffer(pdev,block_no + i);
	}
	//	res = prefetch(pdev,block_no);
	

	return res;
	
	case POLICY_LRU:
	check_buffer_concurrency(pdev, block_no);
		
	res = buf_fetch_lru(pdev,block_no); 
	
	if(res == CACHE_MISS)// && PA4_PREFETCH_SIZE==0) 
		res = at_cache_miss(pdev,block_no);
	
	release_buffer(pdev,block_no);

	if(PA4_PREFETCH_SIZE>0)
	for(i = 1;i<= PA4_PREFETCH_SIZE ; i++)
	if(block_no + i < 1024)	
	{
		check_buffer_concurrency(pdev, block_no + i);
			if(CACHE_MISS == buf_fetch_fifo(pdev,block_no+i))
			at_cache_miss(pdev,block_no + i);
			release_buffer(pdev,block_no + i);	
	}
	//release_buffer(pdev,block_no);
	return res;
	}
}

dsk_buffer_p get_read_buffer(struct devsw* pdev, int block_no) {
	dsk_buffer_p tmp,prev;
	struct devsw* ev_pdev;
	int ev_block_no;
	if(buf_count > 0)
	{

	//kprintf("\n%s : get_read_buffer - 1 | buf_count = %d | bphead = %d | block_no = %d\n",proctab[currpid].pname,buf_count,bphead,block_no);
		tmp = buf_head;
		buf_head = buf_head->next;
		buf_count--;
		tmp->pdev = pdev;
		tmp->block_no = block_no;
		tmp->next = NULL;
		if(PA4_WRITE_POLICY == POLICY_DELAYED_WRITE)tmp->dirty = 1;
	}
	else
	{
	/*General policy for Buffer Eviction. Check for dirty bit  */

		if(bphead == NULL || bphead->next == NULL) return SYSERR;
		/* Eviction */
		/* Check if a any process is accessing head before eviction*/
		check_buffer_concurrency(bphead->pdev, bphead->block_no);
	/* Start Evicting */
		tmp = bphead;
		bphead = bphead->next;
		//check_buffer_concurrency(tmp->pdev, tmp->block_no);
		tmp->next = NULL;
		
		ev_pdev = tmp->pdev;		// Old pdev
		ev_block_no = tmp->block_no;	// Old block_no

//		memncpy(buf,tmp->data,128);
//		kprintf("\n%s : get_read_buffer - 2 | buf_count = %d | tmp->dirty = %d | tmp->block_no = %d | tmp->data = %d \n",proctab[currpid].pname,buf_count,tmp->dirty,tmp->block_no,buf[0]);
		if(PA4_WRITE_POLICY == POLICY_DELAYED_WRITE && tmp->dirty == 1)
		if(dskwrite(tmp->pdev,(char*)tmp->data,tmp->block_no,1)==SYSERR) return SYSERR;
		tmp->pdev = pdev;
		tmp->block_no = block_no;
		if(PA4_WRITE_POLICY == POLICY_DELAYED_WRITE)tmp->dirty = 1;
		release_buffer(ev_pdev, ev_block_no); /*Now lock on bphead is no longer reqd hence released*/
	}
//		kprintf("\n%s : get_read_buffer - 3 | buf_count = %d | tmp = %d | block_no = %d\n",proctab[currpid].pname,buf_count,tmp,tmp->block_no);
		if(SYSERR == dskread(pdev, (char*) tmp->data, block_no, 1)) return SYSERR;
//		memncpy(buf,tmp->data,128);
//		kprintf("\n%s : get_read_buffer - 4 | tmp = %d | block_no = %d | data = %d \n",proctab[currpid].pname,tmp,block_no,buf[0]);

		return tmp;
}

dsk_buffer_p buf_fetch_fifo(struct devsw *pdev, int block_no)
{
	
	dsk_buffer_p tmp;
//	check_buffer_concurrency(pdev, block_no);
		/* Cache Hit */
		tmp = bphead;
		while(tmp!=NULL)
		{
			if(tmp->block_no == block_no && tmp->pdev == pdev)
			{
//			kprintf("\n%s : buf_fetch_fifo - 1 | block_no = %d | data = %d\n",proctab[currpid].pname,block_no,tmp_buf->data);
//				release_buffer(pdev, block_no);	
				return tmp;
			}
			else {
				if(tmp->next==NULL)break;
				else tmp = tmp->next;
			}
		}
//		release_buffer(pdev, block_no);	
		return CACHE_MISS;
		/* Cache Miss */
/*
		check_buffer_concurrency(pdev, block_no);
		if(SYSERR == (tmp_buf = get_read_buffer(pdev,block_no))) return SYSERR;
//		kprintf("\n%s : buf_fetch_fifo - 2 | bphead = %d | bptail = %d | tmp_buf = %d\n",proctab[currpid].pname,bphead,bptail,tmp_buf);
		if(bphead == NULL)
			bphead = bptail = tmp_buf;
		else
			{		
			bptail->next = tmp_buf;
			bptail = bptail->next;
			}
//		kprintf("\n%s : buf_fetch_fifo - 3 | bphead = %d | bptail = %d",proctab[currpid].pname,bphead,bptail);
		release_buffer(pdev, block_no);
		return bptail;
*/
}


dsk_buffer_p buf_fetch_lru(struct devsw *pdev, int block_no)
{
		dsk_buffer_p tmp,tmp_buf;
//		check_buffer_concurrency(pdev, block_no);
		/* Cache Hit */
		tmp = bphead;
		while(tmp!=NULL)
		{
			if(tmp->block_no == block_no && tmp->pdev == pdev)
			{
				sort_buffer(pdev,block_no);
//				release_buffer(pdev, block_no);	
				return tmp;
			}
			else {
				if(tmp->next==NULL)break;
				else tmp = tmp->next;
			}
		}
//		release_buffer(pdev, block_no);	
		return CACHE_MISS;
		/* Cache Miss */
//		check_buffer_concurrency(pdev, block_no);
/*		if(SYSERR == (tmp_buf = get_read_buffer(pdev,block_no))) return SYSERR;
//		kprintf("\n%s : buf_fetch_lru - 1 | bphead = %d | bptail = %d | tmp_buf = %d\n",proctab[currpid].pname,bphead,bptail,tmp_buf);
		if(bphead == NULL)
			bphead = bptail = tmp_buf;
		else
			{		
			bptail->next = tmp_buf;
			bptail = bptail->next;
			}
		release_buffer(pdev, block_no);	
		return bptail;
*/
}


check_buffer_concurrency(struct devsw *pdev, int block_no)
{

	if(&devtab[DISK0] == pdev){
	//	kprintf("\n%s : check_buffer_concurrency - 1 | DISK0 | Block_no = %d\n",proctab[currpid].pname,block_no);
		wait(disk0_sem[block_no]);
	//	kprintf("\n%s : check_buffer_concurrency - 2 | DISK0 | Block_no = %d\n",proctab[currpid].pname,block_no);
	}
	else if(&devtab[DISK1] == pdev)	{
	//	kprintf("\n%s : check_buffer_concurrency - 1 | DISK1 | Block_no = %d\n",proctab[currpid].pname,block_no);
		wait(disk1_sem[block_no]);
	//	kprintf("\n%s : check_buffer_concurrency - 2 | DISK1 | Block_no = %d\n",proctab[currpid].pname,block_no);
	}
	else return SYSERR;
	return OK;
}

int release_buffer(struct devsw *pdev, int block_no)
{
	if(&devtab[DISK0] == pdev){
	//	kprintf("\n%s : release_buffer - 1 | DISK0 | Block_no = %d\n",proctab[currpid].pname,block_no);
		ksignal(disk0_sem[block_no]);
	}
	else if(&devtab[DISK1] == pdev)	{
	//	kprintf("\n%s : release_buffer - 1 | DISK1 | Block_no = %d\n",proctab[currpid].pname,block_no);
		ksignal(disk1_sem[block_no]);
	}
	else return SYSERR;
	return OK;
}


dsk_buffer_p at_cache_miss(struct devsw* pdev, int block_no)
{
		dsk_buffer_p tmp_buf;
	//	check_buffer_concurrency(pdev, block_no);
		if(SYSERR == (tmp_buf = get_read_buffer(pdev,block_no))) return SYSERR;
	//	kprintf("\n%s : at_cache_miss - 1 | bphead = %d | bptail = %d | tmp_buf = %d\n",proctab[currpid].pname,bphead,bptail,tmp_buf);
		if(bphead == NULL)
			bphead = bptail = tmp_buf;
		else
			{		
			bptail->next = tmp_buf;
			bptail = bptail->next;
			}
	//	kprintf("\n%s : at_cache_miss - 2 | bphead = %d | bptail = %d",proctab[currpid].pname,bphead,bptail);
	//	release_buffer(pdev, block_no);
		return bptail;
}



SYSCALL ksignal(int sem)
{
	STATWORD ps;    
	register struct	sentry	*sptr;

	disable(ps);
	if (isbadsem(sem) || (sptr= &semaph[sem])->sstate==SFREE) {
		restore(ps);
		return(SYSERR);
	}
	if ((sptr->semcnt++) < 0)
		ready(getfirst(sptr->sqhead), RESCHNO);

	restore(ps);
	return(OK);
}

dsk_buffer_p prefetch(struct devsw *pdev, int block_no)
{
	dsk_buffer_p tmp,ret;
	int i,size;
	char *mem;

	kprintf("\n%s : prefetch - 1 |block_no = %d\n",proctab[currpid].pname,block_no);
	if(block_no + PA4_PREFETCH_SIZE > 1023){
		size = block_no + PA4_PREFETCH_SIZE - 1023;
		size = PA4_PREFETCH_SIZE - size;
	}
	else
		size = PA4_PREFETCH_SIZE;
			
	/* Prefetching data */	
	mem = getmem(PA4_APP_BUFFER_SIZE*(PA4_PREFETCH_SIZE + 1));
	if(SYSERR == dskread(pdev, mem, block_no, PA4_PREFETCH_SIZE + 1)) return SYSERR;
		
	ret = get_write_buffer(pdev,block_no);
		memncpy( ret->data , mem ,  128);
	if(bphead == NULL)
		bphead = bptail = ret;
	else
	{
		bptail->next = tmp;
		bptail = bptail->next;
	}

	kprintf("\n%s : prefetch - 2 |block_no = %d\n",proctab[currpid].pname,block_no);
	for(i=1;i<=size;i++)
	if(buf_fetch_fifo(pdev,block_no + i) == CACHE_MISS && is_locked(pdev,block_no + i))
	{
		check_buffer_concurrency(pdev, block_no + i);
		kprintf("\n%s : prefetch - 2.1 |block_no = %d\n",proctab[currpid].pname,block_no+i);
		tmp = get_write_buffer(pdev,block_no+i);
		if(tmp == SYSERR) break;
		memncpy(tmp->data,mem + (i * 128), 128);
		kprintf("\n%s : prefetch - 2.2 |bptail = %d | bptil->next = %d | block_no = %d | tmp = %d\n",proctab[currpid].pname,bptail,bptail->next,block_no+i,tmp);
		bptail->next = tmp;
		kprintf("\n%s : prefetch - 2.3 |bptail = %d | bptil->next = %d | block_no = %d\n",proctab[currpid].pname,bptail,bptail->next,block_no+i);
		bptail = bptail->next;
		release_buffer(pdev, block_no + i);
		kprintf("\n%s : prefetch - 2.4 |block_no = %d\n",proctab[currpid].pname,block_no+i);
	}
	
	freemem(mem,PA4_APP_BUFFER_SIZE*(PA4_PREFETCH_SIZE + 1));
	kprintf("\n%s : prefetch - 3 |block_no = %d\n",proctab[currpid].pname,block_no);
	return ret;
}

int is_locked(struct devsw *pdev, int block_no)
{
if(&devtab[DISK0] == pdev)
	if(semaph[(disk0_sem[block_no])].semcnt < 1)
		return 0;

if(&devtab[DISK1] == pdev)
	if(semaph[(disk1_sem[block_no])].semcnt < 1)
		return 0;
	
return 1;
}