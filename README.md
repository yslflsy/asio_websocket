asio_websocket (a websocket base on asio)

QQ:421264988

����QQȺ:953980601

=======I am chinese, so chinese takes precedence!===============================


���ȸ�л youngwolf,�� websocket ���� ascs/asio ��װ���� .

- ����:

1.C++���Կ���,֧�ֿ�ƽ̨,windows/linux/mac/ios/android

2.֧��ascs��ȫ������(https://github.com/youngwolf-project/ascs.git)

3.֧��ssl


- ���ٿ�ʼ

1.server:

#include "asio_websocket/asio_websocket.h"
	service_pump sp;

#if WSS
	wss_server server(sp, asio::ssl::context::sslv23_server);

#if VERIFY_NONE//������֤
	server.wss("./certs/verify_node/cacert.pem", "./certs/verify_node/privkey.pem", "789123");
#else
	server.wss("./certs/verify_peer");
#endif

	server.listen("0.0.0.0", 1234);



2.client:
#include "asio_websocket/asio_websocket.h"

	service_pump sp;

#if WSS
	ssl_context ctx(asio::ssl::context::sslv23_client);

#if VERIFY_NONE//������֤
	ctx.c_verify_none();
#else
	ctx.c_verify_peer("./certs/verify_peer");
#endif

	wss_client client(sp, ctx);
	client.connect("ws://xxxxxx");


- ����:
��֧����Ϣѹ��

websocket ���߲��Թ���:
https://easyswoole.com/wstool.html


-������֧��

Visual C++ 11.0, GCC 4.6 or Clang 3.1 at least, with c++11 features;</br>


==enlgish=========================================================================

First of all, thank you for youngwolf, this development based on ascs/asio package.


- Overview:

1.C++ language development, support cross-platform, windows/linux/mac/ios/android

2. Support all features of ASCs (https://github.com/youngwolf-project/ascs.git)

3. support SSL


- Quick Start

Please look above.


- question:

1. Currently, only two-way authentication is supported under WSS

2. Message compression is not supported for the time being


Websocket online testing tool (cannot be tested under WSS because of the need for bidirectional validation):

https://easyswoole.com/wstool.html


- Compiler requirement:

Visual C++ 11.0, GCC 4.6 or Clang 3.1 at least, with c++11 features;</br>

