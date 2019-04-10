asio_websocket (a websocket base on asio)

QQ:421264988

交流QQ群:953980601

=======I am chinese, so chinese takes precedence!===============================


首先感谢 youngwolf,此 websocket 基于 ascs/asio 封装开发 .

- 功能:

1.C++语言开发,支持跨平台,windows/linux/mac/ios/android

2.支持ascs的全部特性(https://github.com/youngwolf-project/ascs.git)

3.支持ssl


- 快速开始

1.server:

#include "asio_websocket/asio_websocket.h"
	service_pump sp;

#if WSS
	wss_server server(sp, asio::ssl::context::sslv23_server);

#if VERIFY_NONE//单向认证
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

#if VERIFY_NONE//单向认证
	ctx.c_verify_none();
#else
	ctx.c_verify_peer("./certs/verify_peer");
#endif

	wss_client client(sp, ctx);
	client.connect("ws://xxxxxx");


- 问题:
不支持消息压缩

websocket 在线测试工具:
https://easyswoole.com/wstool.html


-编译器支持

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

