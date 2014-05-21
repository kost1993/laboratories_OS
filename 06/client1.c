#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define MAX_LENGTH_NICKNAME 80
#define MSG_BUF_SIZE 1024
#define SRV_NAME "Server"
#define EXIT_COMMAND "!exit"

int sock;
char nickname[MAX_LENGTH_NICKNAME];

void *work(void *number);

int main(int argc, char *argv[])
{
	char msg[MSG_BUF_SIZE];
	struct sockaddr_in addr;
	pthread_t clnt_lstn;
	int read_bytes;
	int send_bytes;

	printf("Hello in custom Instant messenger.\n");
	printf("Please use next format of messages:\n");
	printf("Destination_nickname: Message\n");
	printf("Please, enter your nickname: ");
	scanf("%s", nickname);
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket");
		exit(1);
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons(3425);
	//addr.sin_addr.s_addr = htonl(0x7f000001);
	//addr.sin_addr.s_addr = htonl(inet_addr("127.0.0.1"));
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("connect");
		exit(2);
	}

	if (pthread_create(&clnt_lstn, NULL, work, NULL)) {
		printf("Error creat thread\n");
		exit(19);
	}
	send(sock, nickname, MAX_LENGTH_NICKNAME, 0);

	memset(msg, 0, MSG_BUF_SIZE);
	while (1) {
		printf("%s: ", nickname);
		fflush(stdout);
		read_bytes = read(0, msg, MSG_BUF_SIZE);
		send_bytes = send(sock, msg, MSG_BUF_SIZE, 0);
		if (strncmp(msg, EXIT_COMMAND, strlen(EXIT_COMMAND)) == 0) {
			break;
		}
		memset(msg, 0, read_bytes);
	}
	pthread_kill(clnt_lstn, 0);
	close(sock);
	return 0;
}

void *work(void *number)
{
	char msg[MSG_BUF_SIZE];
	int bytes_read;

	memset(msg, 0, MSG_BUF_SIZE);
	while (1) {
		bytes_read = recv(sock, msg, MSG_BUF_SIZE, 0);
		if (strncmp(msg, SRV_NAME, strlen(SRV_NAME)) == 0) {
			if (strncmp(msg + strlen(SRV_NAME) + 2,
				EXIT_COMMAND, strlen(EXIT_COMMAND)) == 0) {
				break;
			}
			printf("\n%s", msg);
			printf("\n%s: ", nickname);
			fflush(stdout);
		} else {
			printf("\n%s", msg);
			printf("%s: ", nickname);
			fflush(stdout);
		}
	}
	printf("\n");
	pthread_exit(NULL);
}

