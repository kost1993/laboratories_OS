#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> /* open(), O_RDONLY, O_WRONLY */
#include <unistd.h> /* read(), write(), close() */

#define BUF_SIZE 64
const size_t FILESIZESTRING_MAX = 16;

int main(int argc,char *argv[]) {
	DIR *dfd;
	struct dirent *dp;
	char filename[NAME_MAX];
	char absfilename[NAME_MAX];
	char foldername[NAME_MAX];
	char tempchar[NAME_MAX];
	char buffer[BUF_SIZE];
	ssize_t read_bytes;
	ssize_t written_bytes;
	int inp_file_d;
	int out_file_d;
	struct stat statfile; 
	if (argc!=4) {
		printf("Arguments requared.\narchivator -glue/-unglue ");
		printf("input_folder/input_file output_file/output_folder\n");
	}
	else {
		if (strcmp(argv[1],"-glue")==0) {
			printf("Glue operation is started.\n");
			strcpy(foldername, argv[2]);
			strcpy(filename, argv[3]);
			dfd=opendir(foldername);
			out_file_d=creat(filename, S_IWUSR | S_IRUSR);
			while((dp=readdir(dfd))!=NULL) {
				if(strcmp(dp->d_name,".")!=0 && 
				strcmp(dp->d_name,"..")!=0) {
					sprintf(absfilename,"./%s/%s", foldername, dp->d_name);
					inp_file_d=open(absfilename, O_RDONLY);
					fstat(inp_file_d, &statfile);
					//printf(" %s %lld", dp->d_name, (long long)statfile.st_size);
					written_bytes=write(out_file_d, dp->d_name, NAME_MAX);
					sprintf(tempchar,"%lld", (long long)statfile.st_size);
					printf(" %s", tempchar);
					written_bytes=write(out_file_d, tempchar, FILESIZESTRING_MAX);
					close(inp_file_d);
					written_bytes=write(out_file_d, "\n", (size_t)1);
				}
			}
			close(out_file_d);
			closedir(dfd);
			printf("\nClue operation is completed.\n");
		} else if (strcmp(argv[1],"-unglue")==0) {
			printf("Unglue operation.\n");
		} else {
			printf ("Wrong second argument.\n");
		}
	}
	return 0;
}

char *putspace[NAME_MAX] (char *inputchar[],size_t needsize, int flag)
{
	char *tempchar[NAME_MAX];
	size_t count;
	switch (flag) {
		case 0: {
			count=needsize-strlen(inputchar);
			sprintf(tempchar, "%s", inputchar);
		}
		case 1: {
		}
		default: {
			printf("Error.");
		}
	}
	return tempchar;
}
