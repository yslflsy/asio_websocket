//
//  WebServer.cpp
//  
//  Created by qiyun on 19-1-18.
//  Copyright (c) 2018 __jin10.com__. All rights reserved.
//
#include "../common/asws/ws_common.h"

#define WEB_SOCKET_SSL 1
#define DEFAULT_WEB_IP   "0.0.0.0"
#define DEFAULT_WEB_PORT 3001

int main(int argc, char **argv)
{

	char tempc[128] = { 0 };
	ws_message tempMsg;


	service_pump sp;

#if WEB_SOCKET_SSL
	wss_server server(DEFAULT_WEB_IP, DEFAULT_WEB_PORT, sp, "./certs/server.crt", "./certs/server.key");
	while (true)
	{
		ws_message msg;
		cin >> msg;
		server.broadcast_msg(msg);

	}
#else
	ws_server server(DEFAULT_WEB_IP, DEFAULT_WEB_PORT, sp);
	while (true)
	{
		ws_message msg;
		cin >> msg;
		server.broadcast_msg(msg);

	}

#endif


	return 0;
}

