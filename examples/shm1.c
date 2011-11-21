/**
 * Simple shared memory program. Shared memory is just an integer.
 * Child process reads in a value, parent process displays it.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>

#define PERMS (S_IRUSR | S_IWUSR)

int main()
{
        int * shared_memory;
	int segment_id;
	int pid;
	
	/* allocate a shared memory segment */
	if ( (segment_id = shmget(IPC_PRIVATE, sizeof(int), PERMS)) == -1) {
                fprintf(stderr,"Unable to create shared memory segment\n");
                return 1;
	} 

	/* attach the shared memory segment at the specified address */
	if ( (shared_memory = (int *) shmat(segment_id, 0, 0)) == (int *) -1) {
		fprintf(stderr,"Unable to attach to segment %d\n",segment_id);
		return 2;
	}

	/* fork a child process and have the child process read a value into 
	 * the shared memory segment. The parent process will inquire on this
	 * shared value when it returns from wait().
	 * Thus, the call to wait() provides the synchronization.
	 */
	if ( (pid = fork()) == -1) {
                fprintf(stderr,"Couldn't fork off the child, pid =  %d\n", pid);
                return 3;
	}

	if (pid == 0) { /** child code */
        	printf("CHILD: shared memory attached at address %p\n", shared_memory);
		scanf("%d", shared_memory);

        	/* now detach the shared memory segment */
        	shmdt((void *)shared_memory); 
	}
	else {	/* parent code */
		wait();
		printf("PARENT: %d\n", *shared_memory);

        	/* now detach and remove the shared memory segment */
        	shmdt((void *)shared_memory); 
		shmctl(segment_id, IPC_RMID, NULL);
	}
        return 0;
}
