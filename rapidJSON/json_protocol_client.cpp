
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include <cstdio>
#include <iostream>

#include "json_protocol_client.h"

using namespace std;
using namespace rapidjson;

const char *RPCEncode_client(const char version[], const char method[], const char parms[], const char id[])
{
	Document t_doc;
	Value val;

		//	Empty json document:
		char json[] = "{}";
		t_doc.Parse(json);

		//	Version
		//char version[] = "2.0";
		val.SetString(version, static_cast<SizeType> (strlen(version)), t_doc.GetAllocator());
		t_doc.AddMember("jsonrpc", val, t_doc.GetAllocator());

		//	Method:
		//char method[] = "GETTEMP";
		val.SetString(method, static_cast<SizeType> (strlen(method)), t_doc.GetAllocator());
		t_doc.AddMember("method", val, t_doc.GetAllocator());

		//	Parameters:
		//char parms[] = "[]";
		val.SetString(parms, static_cast<SizeType> (strlen(parms)), t_doc.GetAllocator());
		t_doc.AddMember("parms", val, t_doc.GetAllocator());

		//	ID
		//char ID[] = "1";
		val.SetString(id, static_cast<SizeType> (strlen(id)), t_doc.GetAllocator());
		t_doc.AddMember("id", val, t_doc.GetAllocator());

	StringBuffer buffer;

	//	Stringify the JSON object
	Writer<StringBuffer> writer (buffer);
	t_doc.Accept(writer);
	cout << "The JSON client string:  ";
	cout << buffer.GetString() << endl;	//	Print the document

	return buffer.GetString();
}

int RPCDecode_client(const char *buffer)
{
	char recvBuffer[100];
	strcpy(recvBuffer, buffer);

	Document r_doc;

	if (r_doc.ParseInsitu(recvBuffer).HasParseError()) {
		cerr << "Something wrong was received :(" << endl;
		return -1;
	}

	//	RPC
	//	Check if the JSON document is a rpc call
	if (!r_doc.HasMember("jsonrpc")) {
		cerr << "No /'jsonrpc'/ in the JSON document" << endl;
		return -2;
	}

	// Get an iterator pointing at the member
	Value::MemberIterator jsonrpcIter = r_doc.FindMember("jsonrpc");
	// Check if the iterator is not pointing to the end
	if (jsonrpcIter == r_doc.MemberEnd()) {
		cerr << "Iterator pointing to a wrong position. Has to be the first!"
				<< endl;
		return -3;
	}
	// Check that it is string we received, is it even a string?
	if (!jsonrpcIter->value.IsString()) {
		cerr << "The jsonrcp member is not a string. Bye!" << endl;
		return -4;
	}
	// Finally check that we received the correct JSONRpc version number
	if (strcmp("2.0", jsonrpcIter->value.GetString()) != 0) {
		cerr << "It is not version 2.0 jsonrcp. Bye!" << endl;
		return -5;
	}

	////////////////	Result	////////////////
	if (!r_doc.HasMember("result")) {
		cerr << "No /'result'/ in the JSON document" << endl;
		return -2;
	}

	jsonrpcIter = r_doc.FindMember("result");
	if (jsonrpcIter == r_doc.MemberEnd()) {
		cerr << "Iterator pointing to a wrong position. Has to be the second!"
				<< endl;
		return -3;
	}

	if (!jsonrpcIter->value.IsString()) {
		cerr << "The result member is not a string. Bye!" << endl;
		return -4;
	}

	if (strcmp("temperature", jsonrpcIter->value.GetString()) != 0) {
		cerr << "Result was not: temperature. Bye!" << endl;
		return -5;
	}

	////////////////	ID	////////////////
	if (!r_doc.HasMember("id")) {
		cerr << "No /'id'/ in the JSON document" << endl;
		return -2;
	}

	jsonrpcIter = r_doc.FindMember("id");

	if (jsonrpcIter == r_doc.MemberEnd()) {
		cerr << "Iterator pointing to a wrong position. Has to be the fourth!"
				<< endl;
		return -3;
	}

	if (!jsonrpcIter->value.IsString()) {
		cerr << "The id member is not a string. Bye!" << endl;
		return -4;
	}

	if (strcmp("2", jsonrpcIter->value.GetString()) != 0) {
		cerr << "The id is not 1. Bye!" << endl;
		return -5;
	}
	return 0;
}
