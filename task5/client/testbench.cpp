
#include "clientModule.h"
#include "json_protocol_client.h"

using namespace std;

int main(int argC, char **argV)
{
	client Client;
	Client.clientSetup();
	Client.clientHandler();
	return 0;
}
