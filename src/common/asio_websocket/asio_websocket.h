/*
* asio_websocket.h
*
*	Created on: 2019-1-2
*	Author: qiyun
*	QQ: 421264988
*	Community on QQ: 953980601
*	git:https://github.com/yslflsy/asws.git
*/

#ifndef __ASIO_WEBSOCKET_H__
#define __ASIO_WEBSOCKET_H__
#include "ws_base.h"


//ws_client
typedef ascs::tcp::client_socket_base< DEFAULT_WS_PACKER, DEFAULT_WS_UNPACKER > ws_nor_connector;
typedef ws_socket_base<ws_nor_connector, service_pump, SOCKET_TYPE::CLIENT>	ws_client_socket;
typedef ascs::tcp::single_client_base<ws_client_socket> ws_single_client;

typedef client_service<ws_single_client>		ws_client;

//wss_client
typedef ascs::ssl::client_socket_base< DEFAULT_WS_PACKER, DEFAULT_WS_UNPACKER > wss_nor_connector;
typedef ws_socket_base<wss_nor_connector, service_pump, SOCKET_TYPE::CLIENT> wss_client_socket;
typedef ascs::ssl::single_client_base<wss_client_socket> wss_single_client;

typedef client_service<wss_single_client>		wss_client;


//ws_server
typedef server_socket_base< DEFAULT_WS_PACKER, DEFAULT_WS_UNPACKER, i_server > ws_socket_connector;
typedef ws_socket_base<ws_socket_connector, i_server, SOCKET_TYPE::SERVER>	ws_socket;
typedef std::shared_ptr<ws_socket>				WsSocketPtr;
typedef server_base<ws_socket, object_pool<ws_socket>, i_server> ws_listen_server;

typedef server_listener<ws_listen_server, ws_socket>		ws_server;


//wss_server
typedef ascs::ssl::server_socket_base< DEFAULT_WS_PACKER, DEFAULT_WS_UNPACKER > wss_socket_connector;
typedef ws_socket_base<wss_socket_connector, i_server, SOCKET_TYPE::SERVER>	wss_socket;
typedef std::shared_ptr<wss_socket>				WssSocketPtr;
typedef ascs::ssl::server_base<wss_socket>		wss_listen_server;

typedef server_listener<wss_listen_server, wss_socket>	wss_server;


#endif //__ASIO_WEBSOCKET_H__
