/**
 * Producer-consumer problem suing shared memory.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/time.h>

#define PERMS (S_IRUSR | S_IWUSR)
#define BUFSIZE 5

int main()
{
        typedef struct {
		int in;
		int out;
		int buffer[BUFSIZE];
	} Seg;
	Seg * shared_memory;
	int segment_id;
	int pid;
	int i, item = 123;
	int duration;
	struct timeval tv;
	
	/* allocate a shared memory segment */
	if ( (segment_id = shmget(IPC_PRIVATE, sizeof(Seg), PERMS)) == -1) {
                fprintf(stderr,"Unable to create shared memory segment\n");
                return 1;
	} 

	/* attach the shared memory segment at the specified address */
	if ( (shared_memory = (Seg *) shmat(segment_id, 0, 0)) == (Seg *) -1) {
		fprintf(stderr,"Unable to attach to segment %d\n",segment_id);
		return 2;
	}

	shared_memory->in = 0;
	shared_memory->out = 0;

	/* fork() a child process to act as Consumer, the parent acts as Producer.
	 * The parent process will wait() on child to remove shared memory.
	 */
	if ( (pid = fork()) == -1) {
                fprintf(stderr,"Couldn't fork off the child, pid =  %d\n", pid);
                return 3;
	}

	if (pid == 0) { /** child code: Consumer */
        	printf("Child: shared memory attached at address %p\n", shared_memory);

		gettimeofday(&tv, NULL); /* tv includes micro-seconds (usecs), time(0) returns seconds */
		srand(tv.tv_usec);       /* usecs generate different seeds for parent and child */ 
		for (i = 0; i < 10; i++) {
			duration = (rand())%20; /* generate a random number in [0,19] */
			sleep(duration);
			printf("Consumer  slept for %02d\n", duration) /* do nothing! */ ;
			if (shared_memory->in == shared_memory->out) printf("Consumer wait loop!\n");
			while (shared_memory->in == shared_memory->out);
			item = shared_memory->buffer[shared_memory->out];
			printf("Consumer consumed %d from buf[%d]\n", item, shared_memory->out);
			shared_memory->out = (shared_memory->out+1)%BUFSIZE;
		}

		printf("Child: exiting\n");
        	/* now detach the shared memory segment */
        	shmdt((void *)shared_memory); 
	}
	else {	/* parent code: Producer */
		printf("Parent: continues\n");

		gettimeofday(&tv, NULL);
		srand(tv.tv_usec);
		for (i = 0; i < 10; i++) {
			duration = rand()%20;
			sleep(duration);
			printf("Producer slept for %02d\n", duration);
			if (((shared_memory->in+1)%BUFSIZE) == shared_memory->out) printf("Producer wait loop!\n");
			while (((shared_memory->in+1)%BUFSIZE) == shared_memory->out) /* do nothing! */ ;
			shared_memory->buffer[shared_memory->in] = item++;
			printf("Producer produced %d in buf[%d]\n", shared_memory->buffer[shared_memory->in], shared_memory->in);
			shared_memory->in = (shared_memory->in+1)%BUFSIZE;
		}

		wait();
		printf("Parent: exiting\n");
        	/* now detach and remove the shared memory segment */
        	shmdt((void *)shared_memory); 
		shmctl(segment_id, IPC_RMID, NULL);
	}
        return 0;
}
