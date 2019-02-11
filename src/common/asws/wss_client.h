//
//  wss_client.h
//  
//  Created by qiyun on 19-1-18.
//  Copyright (c) 2019 __jin10.com__. All rights reserved.
//

#ifndef __wss_client_h__
#define __wss_client_h__
#include "ws_client.h"
#include <ascs/ext/ssl.h>



using namespace std;
using namespace ascs;
using namespace ascs::ext;

typedef ascs::ssl::client_socket_base< DEFAULT_WS_PACKER, DEFAULT_WS_UNPACKER > wss_nor_connector;

class wss_client_socket : public wss_nor_connector
{
public:
	wss_client_socket(asio::io_context& io_service_, asio::ssl::context& ctx)
	: wss_nor_connector(io_service_, ctx), m_bHandshaked(false)
	{
		//打包器
		m_packer = std::make_shared<DEFAULT_WS_PACKER>();
		m_packer->setIsServer(false);
		packer(m_packer);

		wsunpacker()->setIsServer(false);
	}


	virtual void connect_addr(const string &strIP, uint32_t port, const string &path)
	{
		m_strPath = path;
		m_strIP = strIP;
		m_iPort = port;

		set_server_addr(port, strIP);

	}
	virtual void safeSendPacket(out_msg_type &msg)
	{
		if (msg.length() > 0)
		{
			safe_send_msg((char*)msg.data(), msg.length(), false);
		}
	}
	virtual void sendPacket(out_msg_type &msg)
	{
		if (msg.length() > 0)
		{
			send_msg((char*)msg.data(), msg.length(), false);
		}
	}
	std::shared_ptr<DEFAULT_WS_UNPACKER>  wsunpacker() { return  std::dynamic_pointer_cast<DEFAULT_WS_UNPACKER>(unpacker()); }

protected:
	virtual void sendPong()
	{
		auto msg = m_packer->pack_pong();
		direct_send_msg(std::move(msg));
	}
	virtual void on_unpack_error() { SLOG_ERROR("can not unpack msg."); wss_nor_connector::on_unpack_error(); }

	virtual void on_recv_error(const asio::error_code& ec)
	{
		SLOG_DEBUG("client socket[%s:%d] on recv error [%d]:%s", m_strIP.c_str(), m_iPort, ec.value(), ec.message().c_str());
		wss_nor_connector::on_recv_error(ec);
	}

	virtual void on_connect()
	{
		wss_nor_connector::on_connect();


		SLOG_DEBUG("client socket[%s:%d] on connect++++++++++++", m_strIP.c_str(), m_iPort);

		string strHandshake = Extensions::clientHandshakeString(m_strIP, m_iPort, m_strPath);//生成握手信息
		send_msg((char*)strHandshake.data(), strHandshake.length(), false);

	}
	virtual void on_close()
	{

		SLOG_WARN("client socket[%s:%d] on close------------", m_strIP.c_str(), m_iPort);

		reset();
		
		wss_nor_connector::on_close();

	}
	virtual void reset()
	{
		wss_nor_connector::reset();
		m_bHandshaked = false;
		m_packer->reset();
		wsunpacker()->reset();

	}

protected:


	virtual bool on_msg_handle(out_msg_type& msg)
	{

		/*帧类型 type 的定义data[0] & 0xF
		0x3-7暂时无定义，为以后的非控制帧保留
		0xB-F暂时无定义，为以后的控制帧保留
		*/
		switch (msg.opcode)
		{
// 		case 0: { printf("* recv added\n"); break; }//0x0表示附加数据帧
// 		case 1: { printf("* recv text\n"); break; }//0x1表示文本数据帧
// 		case 2: { printf("* recv binary\n"); break; }//0x2表示二进制数据帧
		case 8: { SLOG_DEBUG("* recv shutdown"); force_shutdown(); return true; }//0x8表示连接关闭
		case 9: { SLOG_DEBUG("- on PING"); sendPong(); return true; }//0x9表示ping
		case 10: {SLOG_DEBUG("+ on PONG"); return true; }//0xA表示pong

		default:
			break;
		}

		if (msg.size() == 0 || msg.data() == nullptr) return true;

		if (!m_bHandshaked)//如果未握手,第一条是解包器生成的握手消息,直接发送
		{
			m_bHandshaked = true;

			SLOG_DEBUG("%s:%d handshake success!!!", m_strIP.c_str(), m_iPort);
		}
		else
		{
			SLOG_DEBUG("recv[%d]:%s", msg.opcode, msg.data());
		}

		return true;
	}

protected:
	std::shared_ptr<DEFAULT_WS_PACKER> m_packer;

	string m_strPath;
	string m_strIP;
	uint16_t m_iPort;

	bool m_bHandshaked;
};

typedef ascs::ssl::single_client_base<wss_client_socket> wss_single_client;



//wss_client//////////////////////////////////////////////////////////////
template<class T_socket>
class wss_client_service : public asio::ssl::context
{
public:
	 
	wss_client_service(const string &strIPPort, service_pump &sp, const string &strCert, const string &strKey)
		:asio::ssl::context(asio::ssl::context::sslv23_client), m_service_pump(sp), m_socket(sp, *this)
	{
		asio::ssl::context::set_options(asio::ssl::context::default_workarounds | asio::ssl::context::no_sslv2 | asio::ssl::context::single_dh_use);
		asio::ssl::context::set_verify_mode(asio::ssl::context::verify_none);

		//asio::ssl::context::load_verify_file(strCertPath + "/server.crt");
		asio::ssl::context::use_certificate_chain_file(strCert);
		asio::ssl::context::use_private_key_file(strKey, asio::ssl::context::pem);
		//asio::ssl::context::use_tmp_dh_file(strCertPath + "/client.pem");


		changeUrl(strIPPort);

		m_service_pump.start_service(ASCS_CLIENT_SOCKET_THREAD);

		SLOG_INFO("wss client[%s] started with thread:%d", strIPPort.c_str(), ASCS_CLIENT_SOCKET_THREAD);
	}

	~wss_client_service() { }

	void disconnect()
	{
		m_service_pump.stop_service();
	}

	void connect(const std::string& strIPPort)//重新连接另一个地址
	{
		changeUrl(strIPPort);
		m_socket.force_shutdown(true);
		m_service_pump.start_service(ASCS_CLIENT_SOCKET_THREAD);
	}

	virtual void sendPacket(ws_message &msg) { m_socket.sendPacket(msg); }
	virtual void safeSendPacket(ws_message &msg) { m_socket.safeSendPacket(msg); }

	T_socket& GetSocket() { return m_socket; }

protected:
	bool changeUrl(const string &url)
	{
		bool bSSL = false;
		string strTempIP = url;
		string strIP;
		string strPath;
		int port = 0;

		if (Extensions::parseURI(strTempIP, bSSL, strIP, port, strPath))
		{
			m_socket.connect_addr(strIP, port, strPath);
			return true;
		}
		return false;
	}
protected:
	T_socket m_socket;
	service_pump & m_service_pump;

private:

};

typedef wss_client_service<wss_single_client>		wss_client;


#endif//__wss_client_h__