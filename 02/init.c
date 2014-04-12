#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

int main()
{
	pid_t pid;
	switch(pid=fork()) {
	case -1:
		exit(1);
	case 0:
		execl("time_start", NULL, NULL);
	default:
		execl("/bin/bash", NULL, NULL);
	}
	return 0;
}
