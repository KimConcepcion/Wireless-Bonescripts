/*
 * RPC.h
 *
 *  Created on: 3 Apr 2018
 *      Author: Riemann
 */

#ifndef JSON_PROTOCOL_CLIENT_H_
#define JSON_PROTOCOL_CLIENT_H_

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include <string>

using namespace rapidjson;
using namespace std;

const char *RPCEncode_client(const char version[], const char method[], const char parms[], const char id[]);
int RPCDecode_client(const char *buffer);

#endif /* JSON_PROTOCOL_CLIENT_H_ */
