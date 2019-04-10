/*
* common.h
*
*	Created on: 2019-4-10
*	Author: qiyun
*	QQ: 421264988
*	Community on QQ: 953980601
*	git:https://github.com/yslflsy/asws.git
*/

#ifndef __COMMON_H__
#define __COMMON_H__

#define WSS					1	//是否ssl
#define VERIFY_NONE			1	//0双向认证,1单向认证


#define DEFAULT_WEB_IP		"0.0.0.0"
#define DEFAULT_WEB_PORT	3001

#if WSS
#define WS_CLIENT_ADDR		string("wss://127.0.0.1:") + to_string(DEFAULT_WEB_PORT)
#else
#define WS_CLIENT_ADDR		string("ws://127.0.0.1:") + to_string(DEFAULT_WEB_PORT)
#endif


#endif //__WS_COMMON_H__
