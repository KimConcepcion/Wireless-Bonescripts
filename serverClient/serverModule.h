
#ifndef SERVERMODULE_H_
#define SERVERMODULE_H_

#include <netinet/in.h>
#include <stdio.h>

class server
{

public:
	server();
	~server();
	void serverSetup();
	void daemonize();
	void signalSetup();
	void timerSetup();
	void clientHandler();

private:
	int socket_fd;
	int new_socket_fd;
	int port;
	int bytes, getTemp, setTemp, exitTemp;
	double tempValue;
	socklen_t client_size;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	char buffer[100];
	char msg[100];
};

#endif
