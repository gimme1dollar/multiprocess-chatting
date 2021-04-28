#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/mman.h>

#define CLN_SIZE 30
#define BUF_SIZE 30
void error_handling(char *message);
void read_childproc(int sig);

int main(int argc, char *argv[])
{
	// Argument check
	if (argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}


	// Signal handling
	struct sigaction act;
	int state;

	act.sa_handler = read_childproc;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	state = sigaction(SIGCHLD, &act, 0);


	// Socket instiantiation
	int serv_sock, clnt_sock;
	int *clnt_num, *clnt_curr, *clnt_sent;
	int str_len;
	char msg[BUF_SIZE];
	char* buf = malloc(sizeof(char)*BUF_SIZE);
	char* con = malloc(sizeof(char)*BUF_SIZE);
	struct sockaddr_in serv_adr, clnt_adr;
	socklen_t adr_sz;

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));
	
	// Socket binding
	if (bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error\n");

	// Socket listening
	if (listen(serv_sock, 5) == -1)
		error_handling("listen() error\n");

	pid_t pid;
	clnt_num = (int*) mmap(NULL, sizeof(int), PROT_WRITE|PROT_READ|PROT_EXEC, MAP_SHARED|MAP_ANON, -1, 0);
	clnt_sent = (int*) mmap(NULL, sizeof(int), PROT_WRITE|PROT_READ|PROT_EXEC, MAP_SHARED|MAP_ANON, -1, 0);
	clnt_curr = (int*) mmap(NULL, sizeof(int), PROT_WRITE|PROT_READ|PROT_EXEC, MAP_SHARED|MAP_ANON, -1, 0);
	con = (char*) mmap(NULL, sizeof(char)*BUF_SIZE, PROT_WRITE|PROT_READ|PROT_EXEC, MAP_SHARED|MAP_ANON, -1, 0);
	while (1)
	{
		adr_sz = sizeof(clnt_adr);
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
		if (clnt_sock == -1)
			continue;
		else
			*clnt_num += 1;
			char welcome[BUF_SIZE];
			sprintf(welcome, "%d", *clnt_num);

		pid = fork();
		if (pid > 0) {
			close(clnt_sock);
		}
		else if (pid == 0) {
			char name[BUF_SIZE];
			int name_yet = 2;
			int id = *clnt_num;
			close(serv_sock);

			pid = fork();
			if (pid > 0) {
				while(1) {
					if(strcmp(con, buf) && id != *clnt_curr) {
						strcpy(buf, con);
						if (name_yet == 1) {
							write(clnt_sock, buf, BUF_SIZE);
							write(clnt_sock, welcome, BUF_SIZE);
							name_yet = 0;
						} else {
							write(clnt_sock, buf, BUF_SIZE);
						}
						
					}
				}
			}
			else if (pid == 0) {
				while(1) {
					str_len = read(clnt_sock, buf, BUF_SIZE);
					if(str_len != 0) {
						if(!strcmp(buf, "quit") || !strcmp(buf, "Quit")) {
							break;
						}

						if(name_yet == 2) {
							strcpy(name, buf);
							strcpy(con, strcat(buf," has joined the chat"));
							name_yet = 1;
							printf("%s\n", buf);
							printf("%s\n", strcat(welcome, " people online"));
						} else {
							strcpy(msg, name);
							strcat(msg, " : ");
							strcat(msg, buf);
							printf("%s\n", msg);
							strcpy(con, msg);
						}
						*clnt_sent = *clnt_num;
						*clnt_curr = id;
					}
				}
			}
			else {
				*clnt_num -= 1;
				continue;
			}
	
			
			close(clnt_sock);
			*clnt_num -= 1;
			printf("%s left the chat\n", name);
			printf("%d people online\n", *clnt_num);
			return 0;
		}
		else {
			*clnt_num -= 1;
			close(clnt_sock);
			continue;
		}
	}
	close(serv_sock);
	munmap(buf, sizeof(char)*BUF_SIZE);
	munmap(con, sizeof(char)*BUF_SIZE);
	munmap(clnt_sent, sizeof(int));
	munmap(clnt_num, sizeof(int));
	munmap(clnt_curr, sizeof(int));
	return 0;
}

void read_childproc(int sig)
{
	pid_t pid;
	int status;
	pid = waitpid(-1, &status, WNOHANG);

	// remove zombie
	if (WIFEXITED(status))
	{
		printf("Removed proc id: %d \n", pid);
		printf("Removed child id: %d \n", WEXITSTATUS(status));
	}
}
void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
