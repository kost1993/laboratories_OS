#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h> /* read(), write(), close() */
#include <signal.h>
#include <fcntl.h> /* open(), O_RDONLY, O_WRONLY */

#define STRING_MAX 256
#define COMMAND_BASH_CLOSE "exit"
#define MESS_BASH_WAIT_COMMAND "bash_alt$ "
#define MESS_BASH_NL "\n"
#define MESS_BASH_COMMAND_MISS "command not found"
#define MAX_ARGS 10
#define MAX_NUMBER_PATH 32

typedef void (*sighandler_t)(int);
static char *argv_child[STRING_MAX];
static char *search_path[MAX_NUMBER_PATH];

int read_bytes;

void handle_signal(int signo)
{
	int write_bytes;
	write_bytes = write(1, MESS_BASH_NL, sizeof(MESS_BASH_NL));
	if(read_bytes == 1)
		write_bytes = write(1, MESS_BASH_WAIT_COMMAND,
				sizeof(MESS_BASH_WAIT_COMMAND));
	else
		write_bytes = write(1, MESS_BASH_NL, sizeof(MESS_BASH_NL));
	fflush(stdout);
}

void fill_argv(char *tmp_argv)
{
	char *foo = tmp_argv;
	int index = 0;
	char ret[STRING_MAX];
	memset(ret, 0, STRING_MAX);
	while(*foo != 0) {
		if(index == MAX_ARGS)
			break;
		if(*foo == ' ') {
			if(argv_child[index] == NULL)
				argv_child[index] = (char *) malloc(
					sizeof(char) * strlen(ret) + 1);
			else
				memset(argv_child[index], 0,
					strlen(argv_child[index]));
			strncpy(argv_child[index], ret, strlen(ret));
			strncat(argv_child[index], "\0", 1);
			memset(ret, 0, STRING_MAX);
			index++;
		} else
			strncat(ret, foo, 1);
		foo++;
	}
	argv_child[index] = (char *)malloc(sizeof(char) * strlen(ret) + 1);
	strncpy(argv_child[index], ret, strlen(ret));
	strncat(argv_child[index], "\0", 1);
}

void get_path_string(char **tmp_envp, char *bin_path)
{
	int index = 0;
	char *tmp;
	while(1) {
		tmp = strstr(tmp_envp[index], "PATH=/");
		if(tmp != NULL && !strstr(tmp_envp[index], "_PATH=/"))
			break;
		else
			index++;
	}	
        strncpy(bin_path, tmp, strlen(tmp));
}

void insert_path_str_to_search(char *path_str) 
{
	int index = 0;
	char *tmp = path_str;
	char ret[STRING_MAX];
	while(*tmp != '=')
		tmp++;
	tmp++;
	while(1) {
		if(*tmp == ':' || *tmp == '\0') {
			strncat(ret, "/", 1);
			search_path[index] = (char *) malloc(sizeof(char) *
				(strlen(ret) + 1));
			strncat(search_path[index], ret, strlen(ret));
			strncat(search_path[index], "\0", 1);
			index++;
			memset(ret, 0, sizeof(ret));
			if(*tmp == '\0')
				break;
		} else
			strncat(ret, tmp, 1);
		tmp++;
	}
}

int attach_path()
{
	char ret[STRING_MAX];
	int index;
	int fd;
	for(index = 0; search_path[index] != NULL; index++) {
		memset(ret, 0, sizeof(ret));
		strcpy(ret, search_path[index]);
		strncat(ret, argv_child[0], strlen(argv_child[0]));
		if((fd = open(ret, O_RDONLY)) > 0) {
			free(argv_child[0]);
			argv_child[0] = (char *)malloc(sizeof(char) * strlen(ret) + 1);
			strncpy(argv_child[0], ret, strlen(ret));
			strncat(argv_child[0], "\0", 1);
			close(fd);
			return 0;
		}
	}
	return 1;
}

int main(int argc, char *argv[], char *envp[])
{
	int write_bytes;
	char exec_file_name[STRING_MAX];
	char path_str[STRING_MAX];
	int flag_exit;
	int exec_status;
	int index;
	int fd;
	pid_t pid;
	signal(SIGINT, SIG_IGN);
	signal(SIGINT, handle_signal);
	read_bytes = STRING_MAX;
	fflush(stdout);
	get_path_string(envp, path_str);
	insert_path_str_to_search(path_str);
	do {
		memset(exec_file_name, 0, read_bytes);
		write_bytes = write(1, MESS_BASH_WAIT_COMMAND,
				sizeof(MESS_BASH_WAIT_COMMAND));
		read_bytes = 1;
		read_bytes = read(0, exec_file_name, STRING_MAX);
		if(read_bytes != 1)
		{
			exec_file_name[read_bytes-1] = 0;
			if(flag_exit = strcmp(exec_file_name, COMMAND_BASH_CLOSE))
				switch(pid = fork()) {
					case -1:
						perror("fork");
						exit(1);
					case 0:
						fill_argv(exec_file_name);
						if(argv_child[0][0] != '/') {
							if((fd = open(argv_child[0], O_RDONLY)) > 0) {
								close(fd);
							} else if(attach_path() != 0) {
								printf("0 %s: %s\n",
									argv_child[0],
									MESS_BASH_COMMAND_MISS);
								exit(1);
							}
						} else {
							if((fd = open(argv_child[0], O_RDONLY)) > 0) {
								close(fd);
							} else {
								printf("1 %s: %s\n",
									argv_child[0],
									MESS_BASH_COMMAND_MISS);
								exit(1);
							}
						}
						exec_status = execve(argv_child[0],
							argv_child, envp);
						if (exec_status < 0) {
							printf("2 %s: %s\n",
								argv_child[0],
								MESS_BASH_COMMAND_MISS);
							exit(1);
						}
						exit(0);
					default:
						wait(NULL);
				}
		}
	} while(flag_exit);
	for (index = 0; argv_child[index] != NULL; index++)
		free(argv_child[index]);
	for (index = 0; search_path[index] != NULL; index++)
		free(search_path[index]);
	write_bytes = write(1, MESS_BASH_NL, sizeof(MESS_BASH_NL));
	return 0;
}
