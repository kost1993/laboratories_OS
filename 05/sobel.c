#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFSIZE 4096
#define LEN_STR_NORM 80
#define LEN_STR_RGB 5

int main(int argc, char *argv[])
{
	if (argc != 3)
		return 1;

	int gy[3][3] = {{-1, -2, -1},
			{0, 0, 0},
			{1, 2, 1}};
	int gx[3][3] = {{-1, 0, 1},
			{-2, 0, 2},
			{-1, 0, 1}};
	int in_fd, out_fd;
	in_fd = open(argv[1], O_RDONLY);
	if (in_fd == -1) {
		printf("Error open input file\n");
		return 1;
	}
	out_fd = creat(argv[2], S_IWUSR | S_IRUSR);
	if (out_fd == -1) {
		printf("Error creat output file\n");
		return 1;
	}
	FILE *in_stream;
	in_stream = fdopen(in_fd, "r");
	if (in_stream == NULL) {
		printf("Error open input stream file\n");
		return 1;
	}
	char buf[BUFSIZE];
	if (fgets(buf, 4, in_stream) == NULL) {
		printf("Error read input file\n");
		return 1;
	}
	char mode_image[2];
	strncpy(mode_image, buf, 2);
	memset(buf, 0, BUFSIZE);
	sprintf(buf, "P2\n");
	write(out_fd, buf, strlen(buf));
	if (fgets(buf, LEN_STR_NORM, in_stream) == NULL) {
		printf("Error read input file\n");
		return 1;
	}
	if (buf[0] == '#') {
		memset(buf, 0, BUFSIZE);
		if (fgets(buf, LEN_STR_NORM, in_stream) == NULL) {
			printf("Error read input file\n");
			return 1;
		}
	}

	int width;
	int height;
	int count;
	count = sscanf(buf, "%d %d", &width, &height);
	if (count != 2) {
		printf("Cant get information about resolution\n");
		return 1;
	}
	/*printf("%d %d\n", width, height);*/
	memset(buf, 0, BUFSIZE);
	sprintf(buf, "%d %d\n", width - 2, height - 2);
	write(out_fd, buf, strlen(buf));
	int brightness;
	memset(buf, 0, BUFSIZE);
	if (fgets(buf, LEN_STR_RGB, in_stream) == NULL)  {
		printf("Cant get information about brightness\n");
		return 1;
	}
	count = sscanf(buf, "%d", &brightness);
	write(out_fd, buf, strlen(buf));

	int i, j;

        int limit = 128 * 128;

	int **allPixR = (int **)malloc(width * sizeof(int *));
	int **allPixG = (int **)malloc(width * sizeof(int *));
	int **allPixB = (int **)malloc(width * sizeof(int *));
	for(i = 0; i < width; i++) {
		allPixR[i] = (int *)malloc(height * sizeof(int));
		allPixG[i] = (int *)malloc(height * sizeof(int));
		allPixB[i] = (int *)malloc(height * sizeof(int));
	}

	int **outPix = (int **)malloc((width - 2) * sizeof(int *));
	for(i = 0; i < width - 2; i++)
		outPix[i] = (int *)malloc((height - 2) * sizeof(int));

	int tempcolor;
	for (j = 0; j < height; j++) {
		for (i = 0; i < width; i++) {
			if (fgets(buf, LEN_STR_RGB, in_stream) == NULL)  {
				printf("Cant get information about Rpixel\n");
				return 1;
			}
			count = sscanf(buf, "%d", &tempcolor);
			if (count != 1) {
				printf("Cant recognize information about Rpixel\n");
				return 1;
			}
			allPixR[i][j] = tempcolor;
			if (fgets(buf, LEN_STR_RGB, in_stream) == NULL)  {
				printf("Cant get information about Gpixel\n");
				return 1;
			}
			count = sscanf(buf, "%d", &tempcolor);
			if (count != 1) {
				printf("Cant recognize information about Gpixel\n");
				return 1;
			}
			allPixG[i][j] = tempcolor;
			if (fgets(buf, LEN_STR_RGB, in_stream) == NULL)  {
				printf("Cant get information about Bpixel\n");
				return 1;
			}
			count = sscanf(buf, "%d", &tempcolor);
			if (count != 1) {
				printf("Cant recognize information about Bpixel\n");
				return 1;
			}
			allPixB[i][j] = tempcolor;
		}
	}
	fclose(in_stream);
	close(in_fd);
	
	int new_rx, new_ry;
	int new_gx, new_gy;
	int new_bx, new_by;
	int rc, gc, bc;
	int wi, hw;
	for (j = 1; j < height - 1; j++) {
		for (i = 1; i < width - 1; i++) {

			new_rx = 0; new_ry = 0;
			new_gx = 0; new_gy = 0;
			new_bx = 0; new_by = 0;
			rc = 0; gc = 0; bc = 0;

			for (wi = -1; wi < 2; wi++) {
				for (hw = -1; hw < 2; hw++) {
					rc = allPixR[i + hw][j + wi];
					new_rx += gx[wi + 1][hw + 1] * rc;
					new_ry += gy[wi + 1][hw + 1] * rc;

					gc = allPixG[i + hw][j + wi];
					new_gx += gx[wi + 1][hw + 1] * gc;
					new_gy += gy[wi + 1][hw + 1] * gc;

					bc = allPixB[i + hw][j + wi];
					new_bx += gx[wi + 1][hw + 1] * bc;
					new_by += gy[wi + 1][hw + 1] * bc;
				}
			}
			if (new_rx * new_rx + new_ry * new_ry > limit ||
			new_gx * new_gx + new_gy * new_gy > limit ||
			new_bx * new_bx + new_by * new_by > limit)
				outPix[i - 1][j - 1] = 255;
			else
				outPix[i - 1][j - 1] = 0;
		}
	}
	
	for (j = 0; j < height - 2; j++) {
		for (i = 0; i < width - 2; i++) {
			memset(buf, 0, BUFSIZE);
			sprintf(buf, "%d\n", outPix[i][j]);
			write(out_fd, buf, strlen(buf));
		}
	}
	return 0;
}
