#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h> /* open(), O_RDONLY, O_WRONLY */

#define BUFSIZE 8

int main(int argc, char *argv[])
{
	if (argc > 1) {
		int inp_fd;
		inp_fd = open(argv[1], O_RDONLY);
		if (inp_fd == -1) {
			printf("Error open input file\n");
			return 1;
		}
		char buf[BUFSIZE];
		int len;
		if ((len = read(inp_fd, buf, BUFSIZE)) != 0)
			write(1, buf, len);
	}
	return 0;
}
