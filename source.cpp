#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> /* open(), O_RDONLY, O_WRONLY */
#include <unistd.h> /* read(), write(), close() */

#define BUF_SIZE 1024
#define DEFAULT_MODE S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
const size_t FILESIZESTRING_MAX = 16;

// format of archive
// <length of name of file><name of file><length of size of file><size of file><data of file>
// <length of name of file> = 1 byte
// <name of file> = <length of name of file> characters
// <length of size of file> = 1 byte
// <size of file> = <length of size of file> characters

char *inttobytes (int number);
int bytestoint (char *s);
long int pow_int (int x, int y);

int main(int argc,char *argv[]) {
	DIR *dfd;
	struct dirent *dp;
	char filename[NAME_MAX];
	char absfilename[NAME_MAX];
	char foldername[NAME_MAX];
	char tempchar[NAME_MAX];
	char buffer[BUF_SIZE];
	char bytesname;
	char *stroka;
	ssize_t read_bytes;
	ssize_t data_read_bytes;
	ssize_t data_need_bytes;
	ssize_t written_bytes;
	int inp_file_d;
	int out_file_d;
	struct stat statfile; 
	if (argc!=4) {
		printf("Arguments required.\narchivator -glue/-unglue ");
		printf("input_folder/input_file output_file/output_folder\n");
	} else {
		if (strcmp(argv[1],"-glue")==0) {
			printf("Glue operation is started.\n");
			strcpy(foldername, argv[2]);
			strcpy(filename, argv[3]);
			dfd=opendir(foldername);
			out_file_d=open(filename, O_RDWR|O_CREAT, S_IWUSR | S_IRUSR);
			while((dp=readdir(dfd))!=NULL) {
				if(strcmp(dp->d_name,".")!=0 && 
				strcmp(dp->d_name,"..")!=0) {
					memset(absfilename,0,sizeof(absfilename));
					sprintf(absfilename,"./%s/%s", foldername, dp->d_name);
					printf("Open file: %s...\n",absfilename);
					inp_file_d=open(absfilename, O_RDONLY);
					fstat(inp_file_d, &statfile);
					//printf(" %s %lld", dp->d_name, (long long)statfile.st_size);
					memset(tempchar,0,sizeof(tempchar));
					sprintf(tempchar,"%s", dp->d_name);
					//printf(" %s", tempchar);
					bytesname=strlen(tempchar);
					//printf("%d\n",bytesname);
					written_bytes=write(out_file_d, &bytesname, 1);
					written_bytes=write(out_file_d, tempchar, bytesname);
					memset(tempchar,0,sizeof(tempchar));
					sprintf(tempchar,"%lld", (long long)statfile.st_size);
					//printf(" %s", tempchar);
					stroka=inttobytes((long long)statfile.st_size);
					//printf("%s\n%d\n",stroka,bytestoint(stroka));
					bytesname=strlen(stroka);
					written_bytes=write(out_file_d, &bytesname, 1);
					written_bytes=write(out_file_d, stroka, bytesname);
					data_read_bytes=(size_t)0;
					do {
						read_bytes=read(inp_file_d, buffer,BUF_SIZE);
						//printf("%s\n%lld\n",buffer,(long long)read_bytes);
						data_read_bytes+=read_bytes;
						written_bytes=write(out_file_d, buffer, read_bytes);
					} while(data_read_bytes!=statfile.st_size);
					close(inp_file_d);
					printf("File %s successfully added to archieve.\n",absfilename);
				}
			}
			close(out_file_d);
			closedir(dfd);
			printf("\nClue operation is completed.\n");
		} else if (strcmp(argv[1],"-unglue")==0) {
			printf("Unglue operation is started.\n");
			strcpy(filename, argv[2]);
			strcpy(foldername, argv[3]);
			//dfd=opendir(foldername);
			mkdir(foldername,DEFAULT_MODE);
			inp_file_d=open(filename, O_RDONLY);
			memset(tempchar,0,sizeof(tempchar));
			do {
				read_bytes=read(inp_file_d, tempchar,1);
				if (read_bytes==(size_t)0) break;
				bytesname=tempchar[0];
				memset(tempchar,0,sizeof(tempchar));
				read_bytes=read(inp_file_d, tempchar,bytesname);
				memset(absfilename,0,sizeof(absfilename));
				sprintf(absfilename,"./%s/%s", foldername, tempchar);
				out_file_d=open(absfilename, O_RDWR|O_CREAT, S_IWUSR | S_IRUSR);
				printf("Unclue file %s.\n", absfilename);
				read_bytes=read(inp_file_d, tempchar,1);
				bytesname=tempchar[0];
				memset(tempchar,0,sizeof(tempchar));
				read_bytes=read(inp_file_d, tempchar,bytesname);
				//printf("%s.\n", tempchar);
				long long tempbytes=0;
				//sscanf(tempchar,"%lld",&tempbytes);
				tempbytes=bytestoint(tempchar);
				data_need_bytes=(size_t) tempbytes;
				printf("%lld bytes.\n", (long long)data_need_bytes);
				//printf("debug\n");
				do {
					if (BUF_SIZE<=data_need_bytes) {
						read_bytes=read(inp_file_d, buffer,BUF_SIZE);
					} else {
						read_bytes=read(inp_file_d, buffer,data_need_bytes);
					}
					data_need_bytes-=read_bytes;
					//printf("%s\n%lld\n",buffer,(long long)read_bytes);
					written_bytes=write(out_file_d, buffer, read_bytes);
				} while(data_need_bytes!=(size_t)0);
				close(out_file_d);
				printf("File %s unclued.\n", absfilename);
			} while(1);
			close(inp_file_d);
			//closedir(dfd);
			printf("Unclue operation is completed.\n");
		} else {
			printf ("Wrong second argument.\n");
		}
	}
	return 0;
}

char *inttobytes (int inp_number) {
	char s[256];
	char letter=0;
	int number=inp_number;
	int buf=0;
	int pos=0;
	memset(s,0,sizeof(s));
	do {
		buf=number%256;
		if (buf==0) number=0;
		letter=buf;
		s[pos]=letter;
		pos++;
	} while((number=number/256)!=0);
	char s_inv[256];
	int i=0;
	memset(s_inv,0,sizeof(s_inv));
	pos--;
	do {
		s_inv[i]=s[pos];
		i++;
		pos--;
	} while(pos!=-1);	
	char *outs;
	outs=s_inv;
	return outs;
}

int bytestoint (char *s) {
	int l=strlen(s);
	int buf=0;
	int pos=0;
	int rezult=0;
	unsigned char utemp=0;
	while(l!=0) {
		utemp=s[l-1];
		buf=utemp*pow_int(256, pos);
		rezult+=buf;
		l--;
		pos++;
	}
	return rezult;
}

long int pow_int(int x,int y) {
	long int rezult=1;
	while(y!=0) {
		rezult*=x;
		y--;
	}
	return rezult;
}
