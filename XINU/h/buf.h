#ifndef _BUFFER_CACHE
#define _BUFFER_CACHE

#define BUFFER_STATE_FULL		-1
#define BUFFER_STATE_HEAP_FULL		-2
#define BUFFER_STATE_WRONG_PARAM	-3
#define BUFFER_STATE_EMPTY		-4
#define BUFFER_STATE_OK			1

#define INVALID_BLOCK			0

#define POLICY_FIFO			1
#define POLICY_LRU			2
#define POLICY_WRITE_THROUGH		10
#define POLICY_DELAYED_WRITE		11
#define CACHE_MISS					15

struct buf {
	struct buf	*next;
	void		*pdev;
	int		block_no;
	void		*data;
	int		size;
	int     dirty;
	int		lock;
};

typedef struct buf	dsk_buffer;
typedef struct buf	*dsk_buffer_p;

dsk_buffer_p	buf_head;
dsk_buffer_p	bphead;
dsk_buffer_p	bptail;

int		buf_count;
int		meta_lock;
int     is_buf_closed;
#endif
