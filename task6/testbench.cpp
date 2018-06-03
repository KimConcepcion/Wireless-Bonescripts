
#include "LM35DZ/LM35DZ.h"
#include "serverModule.h"

using namespace std;

int main(int argC, char **argV)
{
	server Server;
	Server.serverSetup();
	Server.fileSetup();		//	Member to setup the file master etc.
	Server.daemonize();
	Server.signalSetup();
	Server.timerSetup();
	Server.clientHandler();

	return 0;
}
