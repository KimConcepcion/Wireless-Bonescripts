
#ifndef SERVERMODULE_H_
#define SERVERMODULE_H_

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>

class server
{
public:
	server();
	~server();
	void serverSetup();
	void fileSetup();
	void daemonize();
	void signalSetup();
	void timerSetup();
	void clientHandler();

private:
	fd_set master;
	int fdmax;

	int listener;
	int client;
	int port;
	int bytes, getTemp, setTemp, exitTemp;
	double tempValue;

	socklen_t client_size;
	struct timeval T;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	char buffer[100];
	char msg[100];
};

#endif
