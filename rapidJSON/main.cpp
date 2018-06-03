
#include "json_protocol_client.h"
#include "json_protocol_server.h"
#include <stdio.h>
#include <stdlib.h>

using namespace std;

int main()
{
	const char *RPC_client, *RPC_server;

	//	Pack client call in client:
	RPC_client = RPCEncode_client("2.0", "GETTEMP", "[]", "1");

	//	Unpack client call in server:
	if (RPCDecode_server(RPC_client) != 0)
	{
		printf("Wrong JSON message(unpacking client call)...\n");
		exit(0);
	}
	else
		printf("The JSON message from the client is ok!\n");

	//	Pack server JSON answer in server:
	RPC_server = RPCEncode_server();

	//	Unpack server JSON answer in client:
	if (RPCDecode_client(RPC_server) != 0)
	{
		printf("Wrong JSON message (unpacking server answer)...\n");
		exit(0);
	}

	else
		printf("The JSON message from the server is ok!\n");

	return 0;
}
