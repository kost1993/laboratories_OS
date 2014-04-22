#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUFSIZE 8

/* pipes_lab gamma_file inp_file out_file*/

int main(int argc, char *argv[])
{
	int pipedes[2];
	pid_t pid, pid2;
	char *newenviron[] = { NULL };
	pipe(pipedes);
	pid = fork();
	if (argc == 4) {
		if (pid > 0) {
			int len_gamma_key;
			unsigned long long int gamma_key;
			if ((len_gamma_key = read(pipedes[0], &gamma_key, BUFSIZE)) == 0) {
				printf("Error getting gamma key\n");
				return 1;
			}
			pid2 = fork();
			if (pid2 > 0) {
				close(pipedes[1]);
				int read_bytes;
				int out_file_d;
				out_file_d = creat(argv[3], S_IWUSR | S_IRUSR);
				unsigned long long int buf_llu;
				while ((read_bytes = read(pipedes[0], &buf_llu, len_gamma_key)) != 0) {
					buf_llu ^= gamma_key;
					write(out_file_d, &buf_llu, read_bytes);
				}
				close(pipedes[0]);
				close(out_file_d);			
			} else {
				close(pipedes[0]);
				dup2(pipedes[1], 1);
				char *newargv[] = { "input_file", argv[2], NULL };
				execve(newargv[0], newargv, newenviron);
			}
		} else {
			close(pipedes[0]);
			dup2(pipedes[1], 1);
			char *newargv[] = { "input_gamma", argv[1], NULL };
			execve(newargv[0], newargv, newenviron);
		}
	}
	return 0;
}
