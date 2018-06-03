
#ifndef SERVERMODULE_H_
#define SERVERMODULE_H_
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define THREADS 5
#define PORT 1955

typedef struct Thr_Arg
{
	pthread_t tid[THREADS];
	int socket_file_desciptor;
	int new_socket_file_descriptor;
	int bytes;
	socklen_t size;
	struct sockaddr_in server_adr;
	struct sockaddr_in client_adr;
	char buffer[256];
	char msg[50];
	FILE *file_p;
} thr_arg;

void error(FILE *file_p)
{
	fprintf(file_p, "Error occured.\n");
	exit(1);
}

void signal_handler(int signo)
{
	exit(0);
}




#endif
