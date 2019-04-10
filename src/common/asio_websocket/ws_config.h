/*
*	ws_config.h
*
*	Created on: 2019-1-2
*	Author: qiyun
*	QQ: 421264988
*	Community on QQ: 953980601
*	git:https://github.com/yslflsy/asio_websocket.git
*/

#ifndef __WS_CONFIG_H__
#define __WS_CONFIG_H__

//2019/02/3 ver1.0.0
//第一个版本



/*2019/04/10 ver1.1.0
1.调整架构,使用模块减少代码量
2.支持 ssl 单向验证

*/
#define ASWS_VER		10100	
#define ASWS_VERSION	"1.1.0"


#define ASIO_STANDALONE						//不依赖 boost
#define ASCS_REUSE_SSL_STREAM				//ssl socket重连


#define ASCS_FORCE_TO_USE_MSG_RECV_BUFFER 

#define ASCS_HEARTBEAT_INTERVAL		0		//PING/PONG间隔
#define ASCS_RECONNECT_INTERVAL		2000	//重连间隔(ms)
#define ASCS_ASYNC_ACCEPT_NUM		64		//同时接受连接数
#define ST_ASIO_MAX_OBJECT_NUM		4096	//对象池最大数量

#define ASCS_CLIENT_SOCKET_THREAD	1		//客户端线程
#define ASCS_SERVICE_THREAD_NUM		10		//服务端线程

#define ASCS_MSG_BUFFER_SIZE		32000	//单条消息最大长度(最大只支持64K)

#define ASCS_USE_STEADY_TIMER
#define ASCS_ALIGNED_TIMER
//#define ASCS_AVOID_AUTO_STOP_SERVICE
//#define ASCS_DECREASE_THREAD_AT_RUNTIME


#include "ascs/base.h"

#include <iostream>
#include <string>
#include <stdint.h>


using namespace std;



#ifdef NO_SOCKET_UNIFIED_OUT
#define SLOG_FATAL(fmt, ...)		{}
#define SLOG_ERROR(fmt, ...)		{}
#define SLOG_WARN(fmt, ...)			{}
#define SLOG_INFO(fmt, ...)			{}
#define SLOG_DEBUG(fmt, ...)		{}
#else
#define SLOG_FATAL(fmt, ...)		ascs::unified_out::fatal_out(fmt, ##__VA_ARGS__)
#define SLOG_ERROR(fmt, ...)		ascs::unified_out::error_out(fmt, ##__VA_ARGS__)
#define SLOG_WARN(fmt, ...)			ascs::unified_out::warning_out(fmt, ##__VA_ARGS__)
#define SLOG_INFO(fmt, ...)			ascs::unified_out::info_out(fmt, ##__VA_ARGS__)
#define SLOG_DEBUG(fmt, ...)		ascs::unified_out::debug_out(fmt, ##__VA_ARGS__)
#endif





#endif //__WS_CONFIG_H__
