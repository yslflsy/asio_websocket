//
//  WebClient.cpp
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

	ssl_context ctx(asio::ssl::context::sslv23_client);

#if VERIFY_NONE
	ctx.c_verify_none();
#else
	ctx.c_verify_peer("./certs/verify_peer");
#endif

	wss_client client(sp, ctx);
	client.connect(WS_CLIENT_ADDR);

	while (true)
	{
		ws_message msg;
		cin >> msg;

		client.sendPacket(msg);
	}
#else

	ws_client client(sp);
	client.connect(WS_CLIENT_ADDR);

	while (true)
	{
		ws_message msg;
		cin >> msg;

		client.sendPacket(msg);
	}
#endif




	return 0;
}

