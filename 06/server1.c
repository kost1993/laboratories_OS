#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define MAX_CLIENTS 10
#define MAX_LENGTH_NICKNAME 80
#define CNCT_PORT 3425
#define MSG_BUF_SIZE 1024
#define SRV_NAME "Server"
#define EXIT_COMMAND "!exit"
#define CLOSE_SRV_COMMAND "!close"

char msg1[] = "Hello there!\n";
char msg2[] = "Bye bye!\n";

char nicknames[MAX_CLIENTS][MAX_LENGTH_NICKNAME];
int socks[MAX_CLIENTS];
int listener;
int index_thrd;
pthread_t id[MAX_CLIENTS];

void *work(void *number);
void *work_serv_lstn(void *number);

int main(int argc, char *argv[])
{
	pthread_t serv_lstn;
	pthread_t serv_cmd;

	if (pthread_create(&serv_lstn, NULL, work_serv_lstn, NULL)) {
		printf("Error creat thread lstn\n");
		exit(19);
	}

	char msg[MSG_BUF_SIZE];
	int read_bytes;
	memset(msg, 0, MSG_BUF_SIZE);
	while (1) {
		printf("%s: ",SRV_NAME);
		fflush(stdout);
		read_bytes = read(0, msg, MSG_BUF_SIZE);
		if (strncmp(msg, CLOSE_SRV_COMMAND, strlen(CLOSE_SRV_COMMAND)) == 0) {
			printf("%s: Closing server...\n", SRV_NAME);
			pthread_kill(serv_lstn, 0);
			break;
		}
		memset(msg, 0, read_bytes);
	}

	index_thrd--;
	while (index_thrd >= 0) {
		pthread_kill(id[index_thrd], 0);
		close(socks[index_thrd]);
		index_thrd--;
	}

	close(listener);
	return 0;
}

void *work(void *number)
{
	int lcl_index = *(int *) number;
	char msg[MSG_BUF_SIZE];
	int bytes_read;
	int send_bytes;
	int chk_size;
	int src_index;

	memset(msg, 0, MSG_BUF_SIZE);
	bytes_read = recv(socks[lcl_index], nicknames[lcl_index],
		MAX_LENGTH_NICKNAME, 0);
	printf("\n%s: connection established", nicknames[lcl_index]);
	printf("\n%s: ",SRV_NAME);
	fflush(stdout);
	sprintf(msg, "Server: Your nickname: %s", nicknames[lcl_index]);
	send(socks[lcl_index], msg, MSG_BUF_SIZE, 0);
	while (1) {
		bytes_read = recv(socks[lcl_index], msg, MSG_BUF_SIZE, 0);
		if (strncmp(msg, EXIT_COMMAND, strlen(EXIT_COMMAND)) == 0) {
			memset(msg, 0, MSG_BUF_SIZE);
			sprintf(msg, "%s: %s", SRV_NAME, EXIT_COMMAND);
			send_bytes = send(socks[lcl_index], msg, MSG_BUF_SIZE, 0);
			break;
		} else {
			src_index = 0;
			while (src_index < MAX_CLIENTS) {
				if (src_index == lcl_index) {
					src_index++;
				} else {
					if (strncmp(msg, nicknames[src_index], strlen(nicknames[src_index])) != 0) {
						src_index++;
					} else {
						break;
					}
				}
			}
			if (src_index != MAX_CLIENTS) {
				printf("\n%s -> %s: %s", nicknames[lcl_index],
					nicknames[src_index], msg + strlen(nicknames[src_index]) + 1);
				send_bytes = send(socks[src_index],
					msg + strlen(nicknames[src_index]) + 1,
					MSG_BUF_SIZE, 0);
			} else {
				printf("\n%s: %s", nicknames[lcl_index], msg);
			}
			printf("\n%s: ",SRV_NAME);
			fflush(stdout);
		}
	}
	close(socks[lcl_index]);
	printf("\n%s: connection closed", nicknames[lcl_index]);
	printf("\n%s: ",SRV_NAME);
	fflush(stdout);
	pthread_exit(NULL);
}

void *work_serv_lstn(void *number)
{
	struct sockaddr_in addr;
	int self_index[MAX_CLIENTS];

	listener = socket(AF_INET, SOCK_STREAM, 0);
	if(listener < 0) {
		perror("socket");
		exit(1);
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons(CNCT_PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		exit(2);
	}

	index_thrd = 0;
	printf("\n%s: ",SRV_NAME);
	printf("Start listen...");
	printf("\n%s: ",SRV_NAME);
	fflush(stdout);
	listen(listener, 1);
	while (1) {
		socks[index_thrd] = accept(listener, NULL, NULL);
		self_index[index_thrd] = index_thrd;
		if (pthread_create(&id[index_thrd], NULL, work, &self_index[index_thrd])) {
			printf("Error creat thread\n");
			exit(19);
		}
		index_thrd++;
	}
}
