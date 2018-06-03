
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include "ServerModule.h"

void *client_handler(void *arg)
{
	thr_arg *client;
	client = (thr_arg *)arg;

	int bytes;
	char msg[50];
	char buffer[256];
	memset(msg, '\0', sizeof(msg));
	strcpy(msg, "Message received\n");

	for(;;)
	{
		bytes = read(client->new_socket_file_descriptor, buffer, 255);
		if(bytes < 0)
			error(client->file_p);

		while(bytes > 0)
		{
				fprintf(client->file_p, "Message received from client: %s", buffer);
				bzero(buffer, 256);

				fflush(client->file_p);
				bytes = write(client->new_socket_file_descriptor, msg, sizeof(msg));
				if(bytes < 0)
					error(client->file_p);

				bytes = read(client->new_socket_file_descriptor, buffer, 255);
				if(bytes < 0)
					error(client->file_p);
		}
			fprintf(client->file_p, "Client left session.\n");
			close(client->new_socket_file_descriptor);
	}

	pthread_exit(NULL);
}

void *server_handler(void *arg)
{
	thr_arg *input;
	input = (thr_arg *) arg;

	int sock_fd, port = 0;
	int newsock_fd = 0;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	socklen_t size;
	char buffer[256];

	int rc;
	thr_arg *myData;
	myData = (thr_arg *) arg;

	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd < 2)
		error(stderr);

	bzero((char *)&server_addr, sizeof(server_addr));
	port = PORT;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	if(bind(sock_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
		error(stderr);

	listen(sock_fd, THREADS);

	for(;;)
	{
		sleep(1);
		size = sizeof(client_addr);
		bzero(buffer, 256);

		//	Accept new client:
		newsock_fd = accept(sock_fd, (struct sockaddr *) &client_addr, &size);
		if(newsock_fd < 0)
			error(myData->file_p);

		else
		{
			input->new_socket_file_descriptor = newsock_fd;
			pthread_t tid;
			rc = pthread_create(&tid, NULL, client_handler, (void *) &input);
			if(rc != 0)
				error(myData->file_p);
		}
	}

	close(sock_fd);
	pthread_exit(NULL);
}

int main(void)
{
	thr_arg mInput;
	int rc;

	printf("Daemon is running...\n");
	if(signal(SIGTERM, signal_handler) == SIG_ERR)
		error(stderr);

	FILE *fp = NULL;
	pid_t process_id = 0;
	pid_t session_id = 0;
	process_id = fork();	//	Let the parents are have some fun...

	if (process_id < 0) {
		printf("fork failed!\n");
		exit(1);
	}
	if (process_id > 0) {
		printf("[PID] of the child process: [%d] \n", process_id);
		exit(0);
	}

	umask(0);	//	Sat til file mode - minder meget om chmod, men arbejder med nye skabte filer i stedet, herefter gælder umask ikke længere...
	session_id = setsid();
	if (session_id < 0)
		exit(1);

	chdir("/");
	close(0);
	close(1);
	close(2);

	fp = fopen("/var/log/messages", "a+");	//	Syslog

	//	Udfyld struct:
	mInput.file_p = fp;

	rc = pthread_create(&mInput.tid[0], NULL, server_handler, (void *)&mInput);	//	Opretter tråd til at lytte på port 1955
	if(rc != 0)
		error(fp);

	pthread_join(mInput.tid[0], NULL);

	fclose(fp);
	pthread_exit(NULL);
}
