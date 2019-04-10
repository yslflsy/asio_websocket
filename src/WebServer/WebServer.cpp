//
//  WebServer.cpp
//  
//  Created by qiyun on 19-1-18.
//  Copyright (c) 2018 __jin10.com__. All rights reserved.
//
#include "../common/asio_websocket/asio_websocket.h"
#include "../common/common.h"


int main(int argc, char **argv)
{

	char tempc[128] = { 0 };
	ws_message tempMsg;
	service_pump sp;


#if WSS


	wss_server server(sp, asio::ssl::context::sslv23_server);

#if VERIFY_NONE//单向认证
	server.wss("./certs/verify_node/cacert.pem", "./certs/verify_node/privkey.pem", "789123");
#else
	server.wss("./certs/verify_peer");
#endif

	server.listen(DEFAULT_WEB_IP, DEFAULT_WEB_PORT);

	while (true)
	{
		ws_message msg;
		cin >> msg;
		server.broadcast_msg(msg);

	}
#else

	ws_server server(sp);
	server.listen(DEFAULT_WEB_IP, DEFAULT_WEB_PORT);

	while (true)
	{
		ws_message msg;
		cin >> msg;
		server.broadcast_msg(msg);

	}

#endif


	return 0;
}

