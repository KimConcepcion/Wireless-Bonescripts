//	My	interfaces:
#include "rapidjson/json_protocol_server.h"
#include "LM35DZ/LM35DZ.h"
#include "serverModule.h"

#include <unistd.h>
#include <sys/socket.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <csignal>
#include <fcntl.h>
#include <errno.h>
#include <iostream>

#define BACKLOG 5
using namespace std;
FILE *syslogger;

//	##########	Detect if write fd is ready	##########
static int write_ready(int s) {
	int status = 0;
	int sret;
	fd_set fd;
	FD_ZERO(&fd);
	FD_SET(s, &fd);

	sret = select(FD_SETSIZE, NULL, &fd, NULL, NULL);
	if (sret == -1) {
		fprintf(syslogger, "write_ready - select()\n");
		return -1;
	}

	else if (sret > 0)	//	bigger than 0 means there is no timeout
			{
		if ( FD_ISSET(s, &fd) == true)
			status = 1;
	}
	return status;
}

//	##########	Detect if read fd is ready	##########
static int read_ready(int s) {

	int status = 0;
	int sret;
	fd_set fd;
	FD_ZERO(&fd);
	FD_SET(s, &fd);

	sret = select(FD_SETSIZE, &fd, NULL, NULL, NULL);
	if (sret == -1) {
		fprintf(syslogger, "read_ready - select()\n");
		return -1;
	}

	else if (sret > 0)	//	bigger than 0 means there is no timeout
			{
		if ( FD_ISSET(s, &fd) == true)
			status = 1;
	}
	return status;
}

//	##########	Construct the server object	##########
server::server() {
	listener = 0;
	client = 0;
	fdmax = 0;

	bytes = 0;
	getTemp = 0;
	exitTemp = 0;
	setTemp = 0;
	tempValue = 0;
	client_size = 0;
	port = 1955;
	syslogger = NULL;
}

//	##########	Destructor	##########
server::~server() {
	fclose(syslogger);
	close(client);
	close(listener);
	offLED();
}

//	##########	Setup the server	##########
void server::serverSetup() {

	if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "socket()\n");
		exit(1);
	}

	bzero((char *) &server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(listener, (struct sockaddr *) &server_addr, sizeof(server_addr))
			== -1) {
		fprintf(stderr, "bind()\n");
		exit(2);
	}
}

void server::fileSetup() {
	//	Work with non blocking sockets:
}

//	##########	Daemonize setup	##########
void server::daemonize() {
	pid_t process_id = 0;
	pid_t session_id = 0;
	process_id = fork();

	if (process_id < 0) {
		fprintf(stderr, "Error from fork.\n");
		exit(1);
	}

	if (process_id > 0) {
		fprintf(stdout, "Child PID <%d>\n", process_id);
		exit(0);
	}

	umask(0);
	session_id = setsid();
	if (session_id < 0) {
		fprintf(stderr, "setsid failed.\n");
		exit(1);
	}

	chdir("/");
	close(0);
	close(1);
	close(2);

	syslogger = fopen("/var/log/messages", "w+");
	fprintf(syslogger, "Syslog was started by %d\n", getuid());
}

//	##########	Signal handlers	##########
void terminateHandler(int signum) {
	if (signum == 1) {
		exit(0);	//	If signal is recevied, terminate program
	}
}

void timerHandler(int signum) {
	double temperature = 0;

	if (signum == 14) {
		temperature = readTemp();
		fprintf(syslogger, "Temperature is %.2f °C\n", temperature);
		fflush(syslogger);
	}
}

void server::signalSetup() {
	if (signal(1, terminateHandler) == SIG_ERR)	//	install the SIGHUB signal_handler to terminate program
		fprintf(stderr, "Could not install handler - SIGHUB: [%d]...\n", 1);

	if (signal(14, timerHandler) == SIG_ERR)//	install the SIGVTALRM signal handler to read temp value after time configs
		fprintf(stderr, "Could not install handler - SIGVTALRM [%d]...\n", 14);
}

//	########## Timer setup ##########
void server::timerSetup() {
	struct itimerval timer;

	//	Configure timer - 15sec:
	timer.it_value.tv_sec = 15;
	timer.it_value.tv_usec = 0;
	timer.it_interval.tv_sec = 15;
	timer.it_interval.tv_usec = 0;
	int retval = setitimer(ITIMER_REAL, &timer, NULL);//	Timer is configured upon real time...
	if (retval < 0) {
		fprintf(syslogger, "Couldn't install the timer...\n");
		exit(1);
	}
}

//	##########	Handling incomming connection/client	##########
void server::clientHandler() {

	if (fcntl(listener, F_SETFL, O_NONBLOCK) != 0) {
		fprintf(syslogger, "Could not make socket non blocking\n");
		exit(-1);
	}

	int sret;
	const char *error = "Something wrong was unpacked...\n";
	const char *RPC_server;
	char tempValue[50];
	//char clientIPs[INET_ADDRSTRLEN];	//	INET_ADDRSTRLEN is a nice macro, since an IP should not be more than 4x4=16 ciphers

	if (listen(listener, BACKLOG) == -1) {
		fprintf(syslogger, "listen()\n");
		exit(3);
	}

	FD_ZERO(&master);
	FD_SET(listener, &master);	//	Add the file sets:
	fdmax = listener;			//	The biggest file descriptor

	//	Setup the timeout intervals:
	//T.tv_sec = 5;
	//T.tv_usec = 0;

	sret = select(FD_SETSIZE, &master, NULL, NULL, NULL);
	if (sret == -1) {
		fprintf(syslogger, "master - select()\n");
		exit(4);
	}

	//else if(sret == 0)
	//fprintf(syslogger, "timeout reached!\n");

	else {
		if (FD_ISSET(listener, &master) == true) {
			client_size = sizeof(client_addr);
			bzero(buffer, 256);
			client = accept(listener, (struct sockaddr *) &client_addr,
					&client_size);		//	Accept

			//if(client == EAGAIN || client == EWOULDBLOCK)
			//continue;

			if (client == -1) {
				fprintf(syslogger, "accept()\n");
				exit(5);
			}

			FD_CLR(listener, &master);
			FD_SET(client, &master);
			if (client > fdmax)
				fdmax = client;
			fprintf(syslogger, "New connection was accepted on socket: [%d]\n",
					client);

			for (;;) {
				sleep(1);
				fflush(syslogger);

				//	Check if new socket is ready for writing operation:
				if (write_ready(client) == 1) {
					sprintf(msg, "Enter RPC method(GETTEMP)>>");
					bytes = send(client, msg, strlen(msg), 0);
					if (bytes < 0)
						fprintf(syslogger,
								"Error occured while writing to socket\n");
					bzero(msg, 256);//	Clear buffer, otherwise you will have double content...
				}

				else {
					fprintf(syslogger,
							"Socket [%d] is not ready for writing yet\n",
							client);
					continue;
				}

				//	Check if new socket is ready for reading operation:
				if (read_ready(client) == 1) {
					//	Receive data from client:
					bytes = recv(client, buffer, 255, 0);
					if (bytes < 0)
						fprintf(syslogger,
								"Error occured while reading from socket\n");
				}

				else {
					fprintf(syslogger, "Socket [%d] is not ready for reading yet\n", client);
					continue;
				}

				//	Verify RPC call
				if (RPCDecode_server(buffer) == 0) {
					sprintf(tempValue, "temperature %.2lf°C", readTemp());//	Enter temperature value
					RPC_server = RPCEncode_server(tempValue);//	Combine temperature value in JSON string
					strcpy(msg, RPC_server);//	Insert RPC call into msg buffer

					if (write_ready(client) == 1) {
						bytes = send(client, msg, strlen(msg), 0);
						if (bytes < 0)
							fprintf(syslogger,
									"Error occured while writing to socket\n");
					}

					else
					{
						fprintf(syslogger, "Socket [%d] is not ready for writing yet\n", client);
						continue;
					}
				}

				else {
					if (write_ready(client) == 1) {
						bytes = send(client, error, strlen(error), 0);
						if (bytes < 0)
							fprintf(syslogger,
									"Server: Error during decoding of client-RPC answer...\n");
					}

					else
					{
							fprintf(syslogger, "Socket [%d] is not ready for writing yet\n", client);
							continue;
					}
				}
			}
		}
	}

}
