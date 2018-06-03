
#ifndef JSON_PROTOCOL_SERVER_H_
#define JSON_PROTOCOL_SERVER_H_

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"

using namespace rapidjson;
using namespace std;

//	Validate recevied json documents from client
int RPCDecode_server(const char *RPC_call);

//	Send stuff back to client:
const char *RPCEncode_server();

#endif
