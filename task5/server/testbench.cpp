
#include "LM35DZ.h"
#include "serverModule.h"

using namespace std;

int main(int argC, char **argV)
{
	server Server;
	Server.serverSetup();
	Server.daemonize();
	Server.signalSetup();
	Server.timerSetup();
	Server.clientHandler();
	return 0;
}
