#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <fcntl.h> /* open(), O_RDONLY, O_WRONLY */
#include <unistd.h> /* read(), write(), close() */
#include <pthread.h>
#include <sys/time.h> /* gettimeofday() */
#include "sobel.h"

#define BUFSIZE 4096
#define LEN_STR_NORM 80
#define OUTPUT_FORMAT 5

#define SAFE_MALLOC(v, t, n) \
do { \
	v = (t *)malloc((n)*sizeof(t)); \
	if (v == NULL) \
		return EXIT_FAILURE; \
} while (0)

struct RGB {
	unsigned char R;
	unsigned char G;
	unsigned char B;
};

unsigned char **outPix;
struct RGB **allPixRGB;
int width;
int height;
int brightness;
int limit;
pthread_t *id;
int *massive_of_indexes_thread;
int *position_thread_start;
int *position_thread_end;

int gy[3][3] = {{-1, -2, -1},
		{0, 0, 0},
		{1, 2, 1} };
int gx[3][3] = {{-1, 0, 1},
		{-2, 0, 2},
		{-1, 0, 1} };

int main(int argc, char *argv[])
{
	struct timeval start_time, end_time;
	double time_elapsed;
	int ALLOWED_NUM_THREADS;
	int index_thread;
	int count;
	if (argc != 4) {
		printf("Usage:\nsobel input_file.ppm output_file.pgm count_threads\n");
		return EXIT_FAILURE;
	}
	count = sscanf(argv[3], "%d", &ALLOWED_NUM_THREADS);
	if (count != 1) {
		printf("Cant recognize count of threads\n");
		return EXIT_FAILURE;
	}
	SAFE_MALLOC(id, pthread_t, ALLOWED_NUM_THREADS);
	SAFE_MALLOC(massive_of_indexes_thread, int, ALLOWED_NUM_THREADS);
	SAFE_MALLOC(position_thread_start, int, ALLOWED_NUM_THREADS);
	SAFE_MALLOC(position_thread_end, int, ALLOWED_NUM_THREADS);
	int in_fd, out_fd;
	int offset_start = 0;
	in_fd = open(argv[1], O_RDONLY);
	if (in_fd == -1) {
		printf("Error open input file\n");
		return EXIT_FAILURE;
	}
	out_fd = creat(argv[2], S_IWUSR | S_IRUSR);
	if (out_fd == -1) {
		printf("Error creat output file\n");
		return EXIT_FAILURE;
	}
	FILE *in_stream;
	in_stream = fdopen(in_fd, "r");
	if (in_stream == NULL) {
		printf("Error open input stream file\n");
		return EXIT_FAILURE;
	}
	char buf[BUFSIZE];
	if (fgets(buf, 4, in_stream) == NULL) {
		printf("Error read input file\n");
		return EXIT_FAILURE;;
	}
	offset_start += strlen(buf);
	char mode_image[2];
	strncpy(mode_image, buf, 2);
	if (strncmp(mode_image, "P6", 2) != 0) {
		printf("Please use images in format ppm version p6\n");
		return EXIT_FAILURE;;
	}
	memset(buf, 0, BUFSIZE);
	sprintf(buf, "P%d\n", OUTPUT_FORMAT);
	write(out_fd, buf, strlen(buf));
	if (fgets(buf, LEN_STR_NORM, in_stream) == NULL) {
		printf("Error read input file\n");
		return EXIT_FAILURE;
	}
	offset_start += strlen(buf);
	while (buf[0] == '#') {
		memset(buf, 0, BUFSIZE);
		if (fgets(buf, LEN_STR_NORM, in_stream) == NULL) {
			printf("Error read input file\n");
			return EXIT_FAILURE;
		}
		offset_start += strlen(buf);
	}

	count = sscanf(buf, "%d %d", &width, &height);
	if (count != 2) {
		printf("Cant get information about resolution\n");
		return EXIT_FAILURE;
	}
	memset(buf, 0, BUFSIZE);
	sprintf(buf, "%d %d\n", width - 2, height - 2);
	write(out_fd, buf, strlen(buf));
	memset(buf, 0, BUFSIZE);
	if (fgets(buf, LEN_STR_NORM, in_stream) == NULL) {
		printf("Cant get information about brightness\n");
		return EXIT_FAILURE;
	}
	offset_start += strlen(buf);
	count = sscanf(buf, "%d", &brightness);
	write(out_fd, buf, strlen(buf));

	int i, j;
	limit = (brightness / 2) * (brightness / 2);
	int n_kk;
	n_kk = 3;
	int size_pic;
	size_pic = width * height * n_kk;
	unsigned char *allPix;
	SAFE_MALLOC(allPix, unsigned char, size_pic);

	SAFE_MALLOC(allPixRGB, struct RGB *, width);
	for (i = 0; i < width; i++)
		SAFE_MALLOC(allPixRGB[i], struct RGB, height);

	SAFE_MALLOC(outPix, unsigned char *, (height - 2));
	for (j = 0; j < height - 2; j++)
		SAFE_MALLOC(outPix[j], unsigned char, (width - 2));
	int offset_check;
	offset_check = lseek(in_fd, offset_start, 0);
	int read_bytes;
	read_bytes = read(in_fd, allPix, size_pic * sizeof(unsigned char));
	if (read_bytes == -1) {
		printf("Error to read matrix of pixels\n");
		return EXIT_FAILURE;
	}
	fclose(in_stream);
	int index;
	index = 0;
	for (j = 0; j < height; j++) {
		for (i = 0; i < width; i++) {
			allPixRGB[i][j].R = allPix[index + 0];
			allPixRGB[i][j].G = allPix[index + 1];
			allPixRGB[i][j].B = allPix[index + 2];
			index += 3;
		}
	}

	int temp_height;
	temp_height = height - 2;
	position_thread_start[0] = 1;
	for (index_thread = 0; index_thread < ALLOWED_NUM_THREADS; index_thread++) {
		if (index_thread == 0)
			position_thread_start[index_thread] = 1;
		else
			position_thread_start[index_thread] =
				position_thread_end[index_thread - 1];
		if (index_thread == ALLOWED_NUM_THREADS - 1)
			position_thread_end[index_thread] = height - 1;
		else {
			position_thread_end[index_thread] = ceil(temp_height / (double)(ALLOWED_NUM_THREADS - index_thread));
			temp_height -= position_thread_end[index_thread];
			position_thread_end[index_thread] += position_thread_start[index_thread];
		}
	}

	index_thread = 0;
	gettimeofday(&start_time, NULL);
	do {
		massive_of_indexes_thread[index_thread] = index_thread;
		if (pthread_create(&id[index_thread],
			NULL, work,
			&massive_of_indexes_thread[index_thread])) {
			printf("Error creat thread\n");
			exit(19);
		}
		index_thread++;
	} while (index_thread < ALLOWED_NUM_THREADS);
	index_thread--;
	/*Closing threads*/
	do {
		pthread_join(id[index_thread], NULL);
		index_thread--;
	} while (index_thread >= 0);
	gettimeofday(&end_time, NULL);
	/*sec to ms*/
	time_elapsed = (end_time.tv_sec - start_time.tv_sec) * 1000.0;
	/*us to ms*/
	time_elapsed += (end_time.tv_usec - start_time.tv_usec) / 1000.0;
	printf("Time elapsed %g ms\n", time_elapsed);
	for (j = 0; j < height - 2; j++)
		write(out_fd, outPix[j], (width - 2) * sizeof(char));
	close(out_fd);
	for (j = 0; j < height - 2; j++)
		free(outPix[j]);
	free(outPix);
	free(allPix);
	for (i = 0; i < width; i++)
		free(allPixRGB[i]);
	free(allPixRGB);
	free_mem();
	return 0;
}

void free_mem()
{
	free(id);
	free(massive_of_indexes_thread);
	free(position_thread_start);
	free(position_thread_end);
}

void *work(void *number)
{
	int local_index = *(int *) number;
	int new_rx, new_ry;
	int new_gx, new_gy;
	int new_bx, new_by;
	int rc, gc, bc;
	int wi, hw;
	int i, j;
	for (j = position_thread_start[local_index];
	     j < position_thread_end[local_index]; j++) {
		for (i = 1; i < width - 1; i++) {

			new_rx = 0; new_ry = 0;
			new_gx = 0; new_gy = 0;
			new_bx = 0; new_by = 0;
			rc = 0; gc = 0; bc = 0;

			for (wi = -1; wi < 2; wi++) {
				for (hw = -1; hw < 2; hw++) {
					rc = allPixRGB[i + hw][j + wi].R;
					new_rx += gx[wi + 1][hw + 1] * rc;
					new_ry += gy[wi + 1][hw + 1] * rc;

					gc = allPixRGB[i + hw][j + wi].R;
					new_gx += gx[wi + 1][hw + 1] * gc;
					new_gy += gy[wi + 1][hw + 1] * gc;

					bc = allPixRGB[i + hw][j + wi].B;
					new_bx += gx[wi + 1][hw + 1] * bc;
					new_by += gy[wi + 1][hw + 1] * bc;
				}
			}
			rc = sqrt(new_rx * new_rx + new_ry * new_ry);
			gc = sqrt(new_gx * new_gx + new_gy * new_gy);
			bc = sqrt(new_bx * new_bx + new_by * new_by);
			/*outPix[j - 1][i - 1] = 0.299 * rc + 0.587 * gc + 0.114 * bc;*/
			outPix[j - 1][i - 1] = 0.2126 * rc + 0.7152 * gc + 0.0722 * bc;
			if (outPix[j - 1][i - 1] > brightness)
				outPix[j - 1][i - 1] = brightness;
		}
	}
	pthread_exit(NULL);
}
