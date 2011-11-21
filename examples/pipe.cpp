#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

main()
{
	int pd[2];

	if (pipe(pd) == -1) {
		printf("pipe creation failed.\n");
		exit(1);
	}
	if (fork()) { // parent
		printf("Parent process\n");
		char x;
		scanf("%c", &x);
		if (write(pd[1], &x, 1) <= 0) {
			printf("write error\n");
			exit(1);
		}
		printf("Parent process done\n");
	} else { // child
		char get;
		printf("Child process\n");
		if (read(pd[0], &get, 1) <= 0) {
			printf("read error\n");
			exit(1);
		}
		printf("Child unblocked, get = %c\n", get);
	}
} // main
