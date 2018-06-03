
#include <unistd.h>
#include <sys/socket.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <csignal>
#include "clientModule.h"
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "json_protocol_client.h"

using namespace std;

char *s_gets(char *st, int n)
{
	char *ret_val;
	int i = 0;

	ret_val = fgets(st, n, stdin);
	if(ret_val)
	{
		while(st[i] != '\n' && st[i] != '\0')
			i++;
		if(st[i] == '\n')
			st[i] = '\0';
		else
			while(getchar() != '\n')
				continue;
	}
	return ret_val;
}

client::client()
{
	bytes = 0;
	socket_fd = 0;
	beagle = "192.168.7.2";
	port = 1955;
}

client::~client(){
	close(socket_fd);

}

void client::clientSetup()
{

	if( (socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
		fprintf(stderr, "socket()\n");

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	//server_addr.sin_addr.s_addr = inet_addr(beagle);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	int connection_status = connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
	if(connection_status == -1)
	{
		fprintf(stderr, "Error occured when trying to establish connection to the specified server\n");
		exit(1);
	}

	else
	{
		printf("You are connected!\nNetwork details:\n");
		printf("--------------------------------------\n");
		printf("IP: %s\n", beagle);
		printf("Port: %d\n", port);
		printf("--------------------------------------\n");
	}
}

void client::timerSetup()
{
	struct itimerval timer;

	//	Configure timer - 15sec:
	timer.it_value.tv_sec = 15;
	timer.it_value.tv_usec = 0;
	timer.it_interval.tv_sec = 15;
	timer.it_interval.tv_usec = 0;
	int retval = setitimer(ITIMER_REAL, &timer, NULL);	//	Timer is configured upon real time...
	if(retval < 0)
	{
		cerr << "Could not install timer" << endl;
		exit(1);
	}
}

void client::clientSend(const char *RPC_text)
{
	if( (bytes = send(socket_fd, &RPC_text, strlen(RPC_text), 0)) == -1)
		cerr << "send()" << endl;
}

void client::clientRecv()
{

	if( (bytes = recv(socket_fd, &buffer, sizeof(buffer), 0)) == -1)
		cerr << "recv()" << endl;
	buffer[bytes] = '\0';

	printf(buffer);
	printf("\n");
}

void client::clientRecv_json()
{

	if( (bytes = recv(socket_fd, &buffer, sizeof(buffer), 0)) == -1)
		cerr << "recv()" << endl;
	buffer[bytes] = '\0';

	RPCDecode_client(buffer);
	printf(buffer);
	printf("\n");
}

void client::clientHandler()
{
	const char *RPC_Client;

	for(;;)
	{
		bytes = recv(socket_fd, &buffer, sizeof(buffer), 0);	//	Receive the welcome menu
		if(bytes < 0)
			printf("Error occured while receiving data from socket\n");
		buffer[bytes] = '\0';
		printf(buffer);

		s_gets(msg, sizeof(msg));						//	Input the method
		RPC_Client = RPCEncode_client(msg);				//	Pack up the json string
		strcpy(msg, RPC_Client);						//	Insert the JSON string in msg buffer
		bytes = send(socket_fd, &msg, strlen(msg), 0);	//	Send JSON string
		if(bytes < 0)
			printf("Error occured while writing to socket\n");

		bytes = recv(socket_fd, &buffer, sizeof(buffer), 0);	//	Receive response on JSON string
		if(bytes < 0)
			printf("Error occured while receiving data from socket\n");

		if(RPCDecode_client(buffer) == 0)
		{
			buffer[bytes] = '\0';	//	Place NULL character at the end to terminate properly...
			printf("Server: ");
			printf(buffer);
			printf("\n");
		}

		else
			cerr << "Client: Error during decoding of server-RPC answer..." << endl;
	}
}
