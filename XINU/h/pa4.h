#include <disk.h>
#include <buf.h>

#ifndef _PA4
#define _PA4

#define PA4_BUFFER_REPLACEMENT	POLICY_FIFO
#define PA4_WRITE_POLICY		POLICY_WRITE_THROUGH

#define PA4_BUFFER_SIZE		2048
#define PA4_PREFETCH_SIZE		4

#define PA4_DISK_SCHEDULE		 DISK_SCHED_CLOOK

#define PA4_APP_BUFFER_SIZE		128

int disk0_sem[1024];
int disk1_sem[1024];

#endif
