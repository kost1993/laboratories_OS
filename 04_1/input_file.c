#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h> /* open(), O_RDONLY, O_WRONLY */

#define BUFSIZE 0xffff
#define FIFO_NAME "./source_pipe"

int main(int argc, char *argv[])
{
	if (argc > 1) {
		int inp_fd, out_fd;
		mkfifo(FIFO_NAME, 0600); 
		out_fd = open(FIFO_NAME, O_WRONLY);
		if (out_fd == -1) {
			printf("Error open output file\n");
			return 1;
		}
		inp_fd = open(argv[1], O_RDONLY);
		if (inp_fd == -1) {
			printf("Error open input file\n");
			return 1;
		}
		char buf[BUFSIZE];
		int read_bytes, write_bytes;
		while ((read_bytes = read(inp_fd, buf, BUFSIZE)) != 0) {
			write_bytes = write(out_fd, buf, read_bytes);
		}
		close(out_fd);
		close(inp_fd);
		unlink(FIFO_NAME);
		return 0;
	} else {
		printf("Usage of program:\n");
		printf("input_file source_filename\n");
		return 1;
	}
}
