#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUFSIZE 8
#define HEXMAX 256

long long int pow_i_i_alt(int x, int y)
{
	if (y == 0)
		return 1;
	long long int rezult = 1;
	int i;
	for (i = 1; i <= y; i++)
		rezult *= x;
	return rezult;
}

unsigned long long int bytestoint(char *s)
{
	int l = 2 * sizeof(s);
	/*
	printf("%d\n",l);
	*/
	unsigned long long int buf = 0;
	int pos = 0;
	unsigned long long int rezult = 0;
	unsigned char utemp = 0;
	while (l != 0) {
		utemp = s[l - 1];
		buf = utemp * pow_i_i_alt(256, pos);
		rezult += buf;
		/*
		printf("%x\t",utemp);
		printf("%lld\t",buf);
		printf("%lld\n",rezult);
		*/
		l--;
		pos++;
	}
	return rezult;
}

char *inttobytes(unsigned long long int inp_number)
{
	char s[BUFSIZE];
	char letter = 0;
	unsigned long long int number = inp_number;
	int buf = 0;
	int pos = 0;
	memset(s, 0, sizeof(s));
	do {
		buf = number % HEXMAX;
		/*if (buf == 0)
			number = 0;*/
		letter = buf;
		s[pos] = letter;
		pos++;
	} while ((number = number / HEXMAX) != 0);
	char s_inv[BUFSIZE];
	int i = 0;
	memset(s_inv, 0, sizeof(s_inv));
	pos--;
	do {
		s_inv[i] = s[pos];
		i++;
		pos--;
	} while (pos != -1);
	char *outs;
	outs = s_inv;
	return outs;
}

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
			char buf[BUFSIZE];
			int len_gamma_key;
			unsigned long long int gamma_key;
			if ((len_gamma_key = read(pipedes[0], buf, BUFSIZE)) != 0) {
				gamma_key = bytestoint(buf);
				/*printf("%llx\n", gamma_key);*/
			}
			pid2 = fork();
			if (pid2 > 0) {
				close(pipedes[1]);
				int read_bytes;
				int out_file_d;
				char buffer_xor[BUFSIZE];
				out_file_d = creat(argv[3], S_IWUSR | S_IRUSR);
				int pos=0;
				while ((read_bytes = read(pipedes[0], buf, len_gamma_key)) != 0) {
					unsigned long long int buf_char;
					buf_char = bytestoint(buf);
					printf("%4x\t%16llx\t", pos, buf_char);
					buf_char ^= gamma_key;
					printf("%16llx\n", buf_char);
					pos+=len_gamma_key;
					memset(buffer_xor, 0, BUFSIZE);
					strcpy(buffer_xor, inttobytes(buf_char));
					write(out_file_d, buffer_xor, read_bytes);
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
