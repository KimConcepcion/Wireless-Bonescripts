
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <string.h>
#include <strings.h>
#include <iostream>
#include <string>
#include <errno.h>

#define PORT 1955
#define beagle "192.168.7.2"
#define localhost "127.0.0.1"

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

int timeout(int file_des, unsigned int sec)
{
	fd_set set;
	struct timeval timeout;

	FD_ZERO (&set);
	FD_SET (file_des, &set);

	timeout.tv_sec = sec;
	timeout.tv_usec = 0;

	return TEMP_FAILURE_RETRY (select(FD_SETSIZE, &set, NULL, NULL, &timeout));
}

int main(int argC, char **argV)
{

	char buffer[256];
	char msg[100];
	int bytes;

	int client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(client_socket < 0)
	{
		printf("Error occured when trying to create socket\n");
		exit(1);
	}

	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);
	server_address.sin_addr.s_addr = inet_addr(beagle);	//	Address of beaglebone

	//timeout(client_socket, 2);

	int connection_status = connect(client_socket, (struct sockaddr *) &server_address, sizeof(server_address));
	if(connection_status == -1)
	{
		printf("Error occured when trying to establish connection to the specified server\n");
		exit(1);
	}

	else
	{
		printf("Connection to server was succesfully established!\n");
	}

	for(;;)
	{
		bytes = recv(client_socket, &buffer, sizeof(buffer), 0);	//	Receive the welcome menu
		if(bytes < 0)
			printf("Error occured while receiving data from socket\n");

		buffer[bytes] = '\0';	//	Place NULL character at the end to terminate properly...
		printf(buffer);			//	Print the received data from server

		s_gets(msg, sizeof(msg));	//	Be able to send something back to the server
		bytes = send(client_socket, &msg, strlen(msg), 0);
		if(bytes < 0)
			printf("Error occured while writing to socket\n");

		//	GET TEMP:
		if( strncmp(msg, "GET TEMP", strlen("GET TEMP")) == 0 )
		{
			bytes = recv(client_socket, &buffer, sizeof(buffer), 0);	//	Server asks to enter temperature
			if(bytes < 0)
				printf("Error occured while receiving data from socket\n");
			buffer[bytes] = '\0';
			printf(buffer);
		}

		//	SET TEMP:
		else if( strncmp(msg, "SET TEMP", strlen("SET TEMP")) == 0 )
		{
			bytes = recv(client_socket, &buffer, sizeof(buffer), 0);	//	Server asks to enter temperature
			if(bytes < 0)
				printf("Error occured while receiving data from socket\n");
			buffer[bytes] = '\0';
			printf(buffer);

			s_gets(msg, sizeof(msg));									//	You enter the temperature value
			bytes = send(client_socket, &msg, strlen(msg), 0);
			if(bytes < 0)
				printf("Error occured while writing to socket\n");

			bytes = recv(client_socket, &buffer, sizeof(buffer), 0);	//	Receive verification message on desired temperature being set
			if(bytes < 0)
				printf("Error occured while receiving data from socket\n");
			buffer[bytes] = '\0';
			printf(buffer);
		}

		//	EXIT
		else if(strncmp(msg, "EXIT", strlen("EXIT")) == 0)
		{
			bytes = recv(client_socket, &buffer, sizeof(buffer), 0);	//	Receive the welcome menu
			if(bytes < 0)
				printf("Error occured while receiving data from socket\n");
			buffer[bytes] = '\0';
			printf(buffer);
			exit(0);
		}

		//	If none of the options were entered
		else
		{
			bytes = recv(client_socket, &buffer, sizeof(buffer), 0);	//	Receive the welcome menu
			if(bytes < 0)
				printf("Error occured while receiving data from socket\n");
			buffer[bytes] = '\0';

			printf(buffer);
		}

	}

	close(client_socket);
	return 0;
}
