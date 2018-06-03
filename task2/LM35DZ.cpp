
#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdlib.h>
#include <csignal>	//	use this if you're working with signals in c++
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>

using namespace std;

#define TEST_PATH "/home/Riemann/workspaces/ISD2/Task_2/test.txt"	//	path for test
#define ADC_PATH "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"	//	path to read ADC value from channel 0
FILE *fp = NULL;
double temperature;

double readTemp()
{
	double adc_val = 0;
	double temp = 0;
	double temp_cal = 0;

	fstream tempFile (ADC_PATH, fstream::in);
	tempFile >> adc_val;
	tempFile.close();

	//	Calibrate sensorvalue:
	temp = (adc_val*1.8)/4095;
	temp = (temp-0.5)*100;
	temp_cal = temp+49;

	return temp_cal;
}

void terminateHandler(int signum)
{
	if(signum == 1)
	{
		exit(0);	//	If signal is recevied, terminate program
	}
}

void timerHandler(int signum)
{
	if(signum == 14)
	{
		temperature = readTemp();
		fprintf(fp, "Temperature is %.2f °C\n", temperature);
		fflush(fp);
	}
}

int main(int argC, char **argV)
{
		char msg[100];
		int bytes, ret;
		int sock_fd, new_sock_fd, port;
		socklen_t client_size;
		char buffer[256];	//	contain all 256 characters
		struct sockaddr_in server_addr;
		struct sockaddr_in client_addr;

		sock_fd = socket(AF_INET, SOCK_STREAM, 0);
		if(sock_fd < 2)
		{
			fprintf(stderr, "socket error\n");
			exit(1);
		}

		bzero((char *)&server_addr, sizeof(server_addr));
		port = 1955;
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = INADDR_ANY;
		server_addr.sin_port = htons(port);

		if(bind(sock_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
		{
			fprintf(stderr, "bind error\n");
			exit(1);
		}

	if( signal(1, terminateHandler) == SIG_ERR )								//	install the SIGHUB signal_handler to terminate program
		fprintf(stderr, "Could not install handler - SIGHUB: [%d]...\n", 1);

	if(signal(14, timerHandler) == SIG_ERR)										//	install the SIGVTALRM signal handler to read temp value after time configs
		fprintf(stderr, "Could not install handler - SIGVTALRM [%d]...\n", 14);

	//	Set up Daemon:
	pid_t process_id = 0;
	pid_t session_id = 0;
	process_id = fork();

	if(process_id < 0)
	{
		fprintf(stderr, "Error from fork.\n");
		exit(1);
	}

	if(process_id > 0)
	{
		fprintf(stdout, "Child PID <%d>\n", process_id);
		exit(0);
	}

	umask(0);
	session_id = setsid();
	if(session_id < 0)
	{
		fprintf(stderr, "setsid failed.\n");
		exit(1);
	}

	chdir("/");
	close(0);
	close(1);
	close(2);

	fp = fopen("/var/log/messages", "w+");
	fprintf(fp, "Syslog is running...\n");

	struct itimerval timer;
	//	Configure timer - 15sec:
	timer.it_value.tv_sec = 15;
	timer.it_value.tv_usec = 0;
	timer.it_interval.tv_sec = 15;
	timer.it_interval.tv_usec = 0;
	int retval = setitimer(ITIMER_REAL, &timer, NULL);	//	Timer is configured upon real time...
	if(retval < 0)
	{
		fprintf(fp, "Couldn't install the timer...\n");
		exit(1);
	}

	listen(sock_fd, 5);
	fprintf(fp, "Server pending...\n");

	client_size = sizeof(client_addr);
	bzero(buffer, 256);

	new_sock_fd = accept(sock_fd, (struct sockaddr *) &client_addr, &client_size);
	if(new_sock_fd < 0)
	{
		fprintf(fp, "accept error\n");
		exit(1);
	}

	for(;;)
	{
		sleep(1);
		fflush(fp);

		//	Receive data from client:
		bytes = recv(new_sock_fd, buffer, 255, 0);
		if(bytes < 0)
			fprintf(fp, "Error occured while reading from socket\n");

		ret = strncmp(buffer, "GET TEMP", strlen("GET TEMP"));
		if(ret < 0 || ret > 0)
		{
			sprintf(msg, "Write 'GET TEMP' to get temperature value\n");
			bytes = send(new_sock_fd, msg, strlen(msg), 0);
			if(bytes < 0)
				fprintf(fp, "Error occured while writing to socket\n");
		}

		else
		{
			//	Print temperature sensor value:
			sprintf(msg, "%.2f °C\n", temperature);
			bytes = send(new_sock_fd, msg, strlen(msg), 0);
			if(bytes < 0)
				fprintf(fp, "Error occured while writing to socket\n");
		}
	}

	fclose(fp);
	close(new_sock_fd);
	close(sock_fd);
	return 0;
}
