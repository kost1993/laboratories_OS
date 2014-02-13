#include <stdio.h>
#include <dirent.h>
#include <string.h>

#define NAME_MAX 256

int main(int argc,char *argv[]) {
	DIR *dfd;
	struct dirent *dp;
	char filename[NAME_MAX];

	if (argc<2)
		strcpy(filename,".");
	else
		strcpy(filename,argv[1]);

	printf("%s\n\n", filename);
	dfd=opendir(filename);

	while((dp=readdir(dfd))!=NULL)
		printf("%s\n",dp->d_name);

	closedir(dfd);
	return 0;
}
