#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30
void keycontrol(int sig);
void error_handling(char *message);
void read_routine(int sock, char *buf);
void write_routine(int sock, char *buf);

int main(int argc, char *argv[])
{
	// Signal handling
	signal(SIGINT, keycontrol);
	signal(SIGTSTP, keycontrol);

	int sock;
	pid_t pid;
	char buf[BUF_SIZE];
	struct sockaddr_in serv_adr;
	if (argc != 3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock = socket(PF_INET, SOCK_STREAM, 0);  
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_adr.sin_port = htons(atoi(argv[2]));
	
	if (connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
		error_handling("connect() error!");

	printf("Welcome!\n");
	printf("What is your name?\n");
	printf("Name : ");
        fgets(buf, BUF_SIZE, stdin);
	buf[strlen(buf)-1] = '\0';
	write(sock, buf, BUF_SIZE);
	

	pid = fork();
	if (pid >  0) {
		write_routine(sock, buf);
	}
	else if (pid == 0) {
		read_routine(sock, buf);
	}
	else if (pid == -1) {
		printf("error");
	}

	close(sock);
	return 0;
}

void read_routine(int sock, char *buf)
{
	while (1)
	{
		memset(buf, '\0', BUF_SIZE);
		int str_len = read(sock, buf, BUF_SIZE);
		if(str_len == 0) {
                        break;
                }
                printf("%s\n", buf);
	}
}
void write_routine(int sock, char *buf)
{	
	while (1)
	{
		memset(buf, '\0', BUF_SIZE);
                fgets(buf, BUF_SIZE, stdin);
		buf[strlen(buf)-1] = '\0';

                if(write(sock, buf, BUF_SIZE) <= 0)
                {
                        break;
                }

		if(!strcmp("quit", buf) || !strcmp("Quit", buf)) {
			break;
		}
	}
}
void keycontrol(int sig)
{
	if (sig == SIGINT) {
		printf("Type \"quit\" to leave the chat\n");
	}
	else if (sig == SIGSTOP) {
		printf("Type \"quit\" to leave the chat\n");
	}
}
void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
