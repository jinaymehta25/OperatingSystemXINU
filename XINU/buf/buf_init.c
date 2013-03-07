#include <kernel.h>
#include <pa4.h>

/* 
 * Part A 1/4. buf_init()
 * buf_init() is responsible for initializing buffer cache management.
 */
void buf_init(void) {
	int i;
	dsk_buffer_p tmp;
	STATWORD ps;
	disable(ps);
	buf_count = PA4_BUFFER_SIZE;
	tmp = buf_head = getmem(sizeof(dsk_buffer));
	tmp->dirty = 0;
	//tmp->lock = screate(1);
	tmp->size = PA4_APP_BUFFER_SIZE;
	tmp->data = getmem(PA4_APP_BUFFER_SIZE);
	for(i=1;i<PA4_BUFFER_SIZE;i++)
	{
		tmp->next = getmem(sizeof(dsk_buffer));
		tmp->next->size = PA4_APP_BUFFER_SIZE;
		tmp->next->data = getmem(PA4_APP_BUFFER_SIZE);
		tmp->next->dirty = 0;
//		tmp->next->lock = screate(1);
		tmp = tmp->next;
	}

	tmp->next = NULL;
	//meta_lock = screate(1);
	
	for(i=0;i<1024;i++)
	{
	disk0_sem[i] = screate(1);
	disk1_sem[i] = screate(1);
	}

	is_buf_closed = 0;
	restore(ps);
}
