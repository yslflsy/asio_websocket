//
//  WebClient.cpp
//  
//  Created by qiyun on 19-1-18.
//  Copyright (c) 2018 __jin10.com__. All rights reserved.
//
#include "../common/asws/ws_common.h"


#define WEB_SOCKET_SSL 1


int main(int argc, char **argv)
{


	char tempc[128] = { 0 };
	ws_message tempMsg;

	service_pump sp;

#if WEB_SOCKET_SSL
	wss_client client("wss://127.0.0.1:3001", sp, "./certs/client.crt","./certs/client.key");
	while (true)
	{
		ws_message msg;
		cin >> msg;

		client.sendPacket(msg);
	}
#else

	ws_client client("ws://127.0.0.1:3001", sp);

	while (true)
	{
		ws_message msg;
		cin >> msg;

		client.sendPacket(msg);
	}
#endif




	return 0;
}

