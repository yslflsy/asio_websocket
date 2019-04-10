//
//  ws_base.h
//  
//  Created by qiyun on 19-3-18.
//  Copyright (c) 2019 __jin10.com__. All rights reserved.
//

#ifndef __ws_base_h__
#define __ws_base_h__

#include "ws_config.h"
#include <ascs/ext/tcp.h>
#include <ascs/ext/ssl.h>
#include "ws_packer.h"


using namespace ascs;
using namespace ascs::tcp;
using namespace ascs::ext;
using namespace ascs::ext::tcp;

enum SOCKET_TYPE { CLIENT = 0, SERVER = 1};

template<typename Socket_Base, typename Server_T, const bool SERVER>
class ws_socket_base : public Socket_Base
{
public:
	//for ws
	ws_socket_base(Server_T& server_)
	: Socket_Base(server_), m_bHandshaked(false), m_bIsServer(SERVER), m_iPort(0)
	{
		m_packer = dynamic_pointer_cast<DEFAULT_WS_PACKER>(packer());
		m_packer->setIsServer(m_bIsServer);

		m_unpacker = dynamic_pointer_cast<DEFAULT_WS_UNPACKER>(unpacker());
		m_unpacker->setIsServer(m_bIsServer);

	}

	//for wss
	ws_socket_base(Server_T& server_, asio::ssl::context& ctx)
		: Socket_Base(server_, ctx), m_bHandshaked(false), m_bIsServer(SERVER), m_iPort(0)
	{
		m_packer = dynamic_pointer_cast<DEFAULT_WS_PACKER>(packer());
		m_packer->setIsServer(m_bIsServer);

		m_unpacker = dynamic_pointer_cast<DEFAULT_WS_UNPACKER>(unpacker());
		m_unpacker->setIsServer(m_bIsServer);
	}

	~ws_socket_base() {  }


	virtual void reset() 
	{
		m_bHandshaked = false;
		m_strIP = "";
		m_iPort = 0;
		m_packer->reset();
		m_unpacker->reset();
		Socket_Base::reset();
	}

	virtual void on_connect() 
	{
		Socket_Base::on_connect();
		SLOG_DEBUG("socket[%s] on connect++++++++++++", m_strIP.c_str(), m_iPort);

		if (!m_bIsServer)
		{
			string strHandshake = Extensions::clientHandshakeString(m_strIP, m_iPort, "");//生成握手信息
			send_msg((char*)strHandshake.data(), strHandshake.length(), false);
		}
	}
	virtual void safeSendPacket(ws_message &msg) { if (msg.length() > 0) { safe_send_msg(msg.packcode(), msg.getlen(), false); } }
	virtual void sendPacket(ws_message &msg) { if (msg.length() > 0 ) { send_msg(msg.packcode(), msg.getlen(), false); } }
	virtual void on_unpack_error() { Socket_Base::on_unpack_error(); SLOG_ERROR("socket can not unpack msg."); }

	//for client
	virtual void set_addr(uint16_t port, const string &strIP) { m_strIP = strIP; m_iPort = port; }

	string GetIP()
	{
		if (m_strIP.empty())//for server
		{
			try { auto ep = lowest_layer().remote_endpoint(); m_strIP = ep.address().to_string(); m_iPort = ep.port(); }
			catch (std::exception& e) { SLOG_ERROR("get socket ip error:%s", e.what()); }
		}

		return m_strIP;
	}

protected:

	virtual void on_close() { Socket_Base::on_close(); reset(); }
	virtual void sendPong() { auto msg = m_packer->pack_pong(); direct_send_msg(std::move(msg)); }
	virtual bool on_msg_handle(ws_message& msg)
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
		case 10:{ SLOG_DEBUG("+ on PONG"); return true; }//0xA表示pong

		default:
			break;
		}

		if (msg.size() == 0 || msg.data() == nullptr) return true;

		if (!m_bHandshaked)//未握手
		{
			m_bHandshaked = true;
			if (m_bIsServer)//服务端,第一条是解包器生成的握手消息,直接发送
			{
				send_msg((char*)msg.data(), msg.size(), false);
				SLOG_DEBUG("server send handshake:%s", msg.data());
			}
			else
			{
				SLOG_DEBUG("client %s handshake success!!!", m_strIP.c_str());
			}
		}
		else
		{
			SLOG_DEBUG("recv[%d]:%s", msg.opcode, msg.data());
		}
		return true;
	}

protected:


	bool		m_bHandshaked;		//是否已握手
	bool		m_bIsServer;		//是否服务端
	string		m_strIP;
	uint16_t	m_iPort;

	std::shared_ptr<DEFAULT_WS_PACKER> m_packer;
	std::shared_ptr<DEFAULT_WS_UNPACKER> m_unpacker;


};



//server_listener//////////////////////////////////////////////////////////////////////////////////////////////////
template<typename Server_T, typename Socket_T>
class server_listener : public Server_T
{
public:
	typedef std::shared_ptr<Socket_T>	socket_t;

	//for ws
	server_listener(service_pump & sp)
		: Server_T(sp), m_service_pump(sp)
	{
		Server_T::max_size(ST_ASIO_MAX_OBJECT_NUM);
	}
	//for wss
// 	server_listener(service_pump & sp, const asio::ssl::context &method /*= asio::ssl::context::sslv23_server*/)
	server_listener(service_pump & sp, const asio::ssl::context_base::method &method /*= asio::ssl::context::sslv23_server*/)
		: Server_T(sp, method), m_service_pump(sp)
	{
		Server_T::max_size(ST_ASIO_MAX_OBJECT_NUM);
	}

	//wss verify_none
	void wss(const string &strCert, const string &strKey, const string &password)// int verify_mode = asio::ssl::context::verify_none)
	{
		m_strPassword = password;
		ssl_context::s_verify_none(context(), strCert, strKey, std::bind(&server_listener::get_password, this));
	}

	//wss verify_peer
	void wss(const string &strCertPath)// int verify_mode = asio::ssl::context::verify_none)
	{
		ssl_context::s_verify_peer(context(), strCertPath);
	}

	void listen(const string &ip, uint16_t port, uint32_t threadNum = ASCS_SERVICE_THREAD_NUM)
	{
		Server_T::set_server_addr(port, ip);
		m_service_pump.start_service(threadNum);

		SLOG_INFO("ws_listener[%s:%d] started with thread:%d", ip.c_str(), port, threadNum);
	}

	std::string get_password() const { return m_strPassword; }
	socket_t  GetClient(uint_fast64_t clientID) { return Server_T::find(clientID); }

	virtual void broadcastPacket(ws_message &msg)
	{
		this->do_something_to_all([&](const socket_t & item) { item->sendPacket(msg); });
	}
	virtual bool sendPacketToClient(uint_fast64_t clientID, ws_message &msg)
	{
		socket_t obj = GetClient(clientID);
		if (obj != nullptr) { obj->sendPacket(msg); return true; }
		return false;
	}
protected:
	service_pump &	m_service_pump;
	string			m_strPassword;
private:


};

//client_service//////////////////////////////////////////////////////////////////////////////////////////////////
template<class Socket_T>
class client_service : public Socket_T
{
public:
	client_service(service_pump &sp) :m_service_pump(sp), Socket_T(sp), m_context(nullptr){ }
	client_service(service_pump &sp, asio::ssl::context & ctx): m_context(&ctx), m_service_pump(sp), Socket_T(sp, ctx){}
	~client_service() { }

	void connect(const std::string& strIPPort, uint32_t threadNum = ASCS_CLIENT_SOCKET_THREAD)
	{
		bool bSSL = false;
		int port = 0;
		string strTempIP = strIPPort;
		string strIP;
		string strPath;

		if (!Extensions::parseURI(strTempIP, bSSL, strIP, port, strPath))
		{
			SLOG_FATAL("failed to parse url:%s", strTempIP.c_str());
			return;
		}
		
		Socket_T::set_server_addr(port, strIP);
		Socket_T::set_addr(port, strIP);

		Socket_T::force_shutdown(true);
		m_service_pump.start_service(threadNum);

		SLOG_INFO("client[%s] started with thread:%d", strIPPort.c_str(), threadNum);
	}


protected:
	service_pump & m_service_pump;
	asio::ssl::context	*m_context;

private:

};

//for ssl context
class ssl_context : public asio::ssl::context
{
public:
	ssl_context(asio::ssl::context_base::method method) :asio::ssl::context(method){}

	//客户端 单向认证
	void c_verify_none()
	{
		set_options(asio::ssl::context::default_workarounds | asio::ssl::context::no_sslv2 | asio::ssl::context::single_dh_use);
		set_verify_mode(asio::ssl::context::verify_none);
	}

	//服务端 单向认证
	template <typename PasswordCallback>
	static void s_verify_none(asio::ssl::context &ctx, const string &strCert, const string &strKey, PasswordCallback passwordCallback)
	{
		ctx.set_options(asio::ssl::context::default_workarounds | asio::ssl::context::no_sslv2 | asio::ssl::context::single_dh_use);
		ctx.set_verify_mode(asio::ssl::context::verify_none);// | asio::ssl::context::verify_fail_if_no_peer_cert

		ctx.set_password_callback(passwordCallback);
		ctx.use_certificate_chain_file(strCert);
		ctx.use_private_key_file(strKey, asio::ssl::context::pem);

	}

	//客户端 双向认证
	void c_verify_peer(const string &strCertPath)
	{
		set_options(asio::ssl::context::default_workarounds | asio::ssl::context::no_sslv2 | asio::ssl::context::single_dh_use);
		set_verify_mode(asio::ssl::context::verify_peer | asio::ssl::context::verify_fail_if_no_peer_cert);

		load_verify_file(strCertPath + "/server.crt");
		use_certificate_chain_file(strCertPath + "/client.crt");
		use_private_key_file(strCertPath + "/client.key", asio::ssl::context::pem);
		use_tmp_dh_file(strCertPath + "/client.pem");
	}


	//服务端 双向认证
	static void s_verify_peer(asio::ssl::context &ctx, const string &strCertPath)
	{
		ctx.set_options(asio::ssl::context::default_workarounds | asio::ssl::context::no_sslv2 | asio::ssl::context::single_dh_use);
		ctx.set_verify_mode(asio::ssl::context::verify_peer | asio::ssl::context::verify_fail_if_no_peer_cert);

		ctx.load_verify_file(strCertPath + "/client.crt");
		ctx.use_certificate_chain_file(strCertPath + "/server.crt");
		ctx.use_private_key_file(strCertPath + "/server.key", asio::ssl::context::pem);
		ctx.use_tmp_dh_file(strCertPath + "/server.pem");
	}
};

#endif//__ws_base_h__