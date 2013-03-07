All the configurations parameters are to be passed in PA4.h
The configuration parameters that can be modified in PA4.h is given below,
Use any combinations of the right hand sides to get various outputs. Few og these outputs are depicted in 
TestResults.pdf

PA4_BUFFER_REPLACEMENT	POLICY_FIFO / POLICY_LRU 
PA4_WRITE_POLICY	POLICY_WRITE_THROUGH / POLICY_DELAYED_WRITE
PA4_BUFFER_SIZE		2048 / 1024 / 512 / 256 / 128
PA4_PREFETCH_SIZE	4 / 2 / 0
PA4_DISK_SCHEDULE	DISK_SCHED_CLOOK / DISK_SCHED_SSTF / (Passing Nothing ensures that its FIFO)
PA4_APP_BUFFER_SIZE	128 / 1024