#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>


#define NAME_MAX 256
#define COMMAND_BASH_CLOSE "exit"
#define MESS_BASH_WAIT_COMMAND "bash_alt$ "
#define MESS_BASH_COMMAND_MISS "command not found"
#define MAX_ARGS 31

static char *argv_child[100];

void fill_argv(char *tmp_argv)
{
	char *foo = tmp_argv;
	int index = 0;
	char ret[100];
	memset(ret, 0, 100);
	while(*foo != 0) {
		if(index == 10)
			break;
		if(*foo == ' ') {
			if(argv_child[index] == NULL)
				argv_child[index] = (char *)malloc(sizeof(char) *
					strlen(ret) + 1);
			else {
				memset(argv_child[index], 0, strlen(argv_child[index]));
			}
			strncpy(argv_child[index], ret, strlen(ret));
			strncat(argv_child[index], "\0", 1);
			bzero(ret, 100);
			index++;
		} else {
			strncat(ret, foo, 1);
		}
		foo++;
	}
	argv_child[index] = (char *)malloc(sizeof(char) * strlen(ret) + 1);
	strncpy(argv_child[index], ret, strlen(ret));
	strncat(argv_child[index], "\0", 1);
}

int main(int argc, int *argv[])
{
	pid_t pid;
	int read_bytes;
	int write_bytes;
	char exec_file_name[NAME_MAX];
	int flag_exit;
	int exec_status;
	int index;
	read_bytes = NAME_MAX;
	do {
		memset(exec_file_name, 0, read_bytes);
		write_bytes = write(1, MESS_BASH_WAIT_COMMAND,
			sizeof(MESS_BASH_WAIT_COMMAND));
		read_bytes = read(0, exec_file_name, NAME_MAX);
		exec_file_name[read_bytes-1] = 0;
		if(flag_exit = strcmp(exec_file_name, COMMAND_BASH_CLOSE))
			switch(pid = fork()) {
				case -1:
					perror("fork");
					exit(1);
				case 0:
					fill_argv(exec_file_name);
					exec_status = execv(argv_child[0], argv_child);
					if (exec_status < 0) {
						printf("%s: %s\n",
							exec_file_name,
							MESS_BASH_COMMAND_MISS);
						exit(1);
					}
					exit(0);
				default:
					wait();
			}
	} while(flag_exit);
	for (index = 0; argv_child[index] != NULL; index++)
		free(argv_child[index]);
	printf("\n");
	return 0;
}
