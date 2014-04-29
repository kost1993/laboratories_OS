#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> /* open(), O_RDONLY, O_WRONLY */

#define BUFSIZE 8
#define GAMMA_NAME "./gamma_pipe"
#define SOURCE_NAME "./source_pipe"
/* pipes_lab gamma source output*/

int main(int argc, char *argv[])
{
	int pipedes[2];
	pid_t pid[2];
	char *newenviron[] = { NULL };
	if (argc == 2) {
		int in_fd[2], out_fd;
		in_fd[0] = open(GAMMA_NAME, O_RDONLY);
		if (in_fd[0] == -1) {
			printf("Error open gamma pipe\n");
			return 1;
		}
		int len_gamma_key;
		unsigned long long int gamma_key;
		if ((len_gamma_key = read(in_fd[0], &gamma_key, BUFSIZE)) == 0) {
			printf("Error getting gamma key\n");
			return 1;
		}
		close(in_fd[0]);
		unlink(GAMMA_NAME);
		in_fd[1] = open(SOURCE_NAME, O_RDONLY);
		if (in_fd[1] == -1) {
			printf("Error open source pipe\n");
			return 1;
		}
		out_fd = creat(argv[1], S_IWUSR | S_IRUSR);
		int read_bytes;
		unsigned long long int buf_llu;
		while ((read_bytes = read(in_fd[1], &buf_llu, len_gamma_key)) != 0) {
			buf_llu ^= gamma_key;
			write(out_fd, &buf_llu, read_bytes);
		}
		close(in_fd[1]);
		unlink(SOURCE_NAME);
		close(out_fd);
	} else {
		printf("Usage of program:\n");
		printf("pipes_lab output_filename\n");
		return 1;
	}
}
