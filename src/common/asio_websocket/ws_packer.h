//
//  ws_packer.h
//  
//  Created by qiyun on 19-1-21.
//  Copyright (c) 2019 __jin10.com__. All rights reserved.
//

#ifndef __ws_unpacker_h__
#define __ws_unpacker_h__
#include "ascs/ext/ext.h"
#include "ws_ext.h"


using namespace ascs;
using namespace ascs::ext;


#ifdef __APPLE__
#include <libkern/OSByteOrder.h>
#define htobe64(x) OSSwapHostToBigInt64(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#endif



#ifdef __MINGW32__
// Windows has always been tied to LE
#define htobe64(x) __builtin_bswap64(x)
#define be64toh(x) __builtin_bswap64(x)
#endif

#ifdef _WIN32
#define htobe64(x) htonll(x)
#define be64toh(x) ntohll(x)
#endif

#define MAX_MSG_PARSE_ONCE 100//每次最多解析几条消息

#define MASK_LEN 4

/*帧类型*/
enum OpCode : unsigned char 
{
	eOpCode_Added = 0,	//0x0表示附加数据帧
	eOpCode_TEXT = 1,	//0x1表示文本数据帧
	eOpCode_BINARY = 2,	//0x2表示二进制数据帧
						//0x3 - 7暂时无定义，为以后的非控制帧保留
	eOpCode_CLOSE = 8,	//0x8表示连接关闭
	eOpCode_PING = 9,	//0x9表示ping
	eOpCode_PONG = 10,	//0xA表示pong
						//0xB - F暂时无定义，为以后的控制帧保留
};

enum SND_CODE
{
	SND_CONTINUATION = 1,
	SND_NO_FIN = 2,
	SND_COMPRESSED = 64
};

//WebSocket 数据包头
struct WsMsg_info
{

	bool isEof;//是否是结束帧 data[0]>>7
	char dfExt;//是否有扩展定义 data[0] & 0x70

	OpCode type; //OpCode

	bool hasMask;//是否有掩码 data[1]>>7

				 //Other-->
	char mask[MASK_LEN];//掩码
	char *pData;//指向真实数据(动态分配,自动管理);其长度为 Len
	uint64_t Len;//真实数据长度  (data[1] & 0x7f)计算出来的数据长度

	WsMsg_info() { clear(); }

	~WsMsg_info() {};
	void clear()
	{
		type = eOpCode_TEXT;
		dfExt = Len = 0;
		isEof = hasMask = false;
		pData = nullptr;
		memset(mask, 0, MASK_LEN);
	}
	static void do_mask(char *data, uint32_t ilen, char mask[MASK_LEN])
	{
		for (uint64_t i = 0; i < ilen; i++)
		{
			data[i] = data[i] ^ mask[i % MASK_LEN];
		}
	}

};

struct ws_message : public std::string
{
	ws_message():opcode(eOpCode_TEXT), bAppend(false){}
	ws_message(const char* data, size_t size, OpCode code) :std::string(data, size), opcode(code), bAppend(false){  }
	void swap(ws_message &other) { string::swap(other); opcode = other.opcode; bAppend = other.bAppend; }
	void clear() { opcode = eOpCode_TEXT; std::string::clear(); bAppend = false; }

	//把opcode合并到消息未尾,packer时用
	const char* packcode() { if (!bAppend) { std::string::push_back((unsigned char)opcode); bAppend = true; } return data(); }
	size_t getlen() { return bAppend ? size() - 1 : size(); }
	
	OpCode  opcode;
	bool    bAppend;
};


class ws_packer : public i_packer<ws_message>
{
public:
	static size_t get_max_msg_size() { return ASCS_MSG_BUFFER_SIZE - ASCS_HEAD_LEN; }

	ws_packer() :m_bHandshaked(false), isServer(true){ memset(msg_data, 0, ASCS_MSG_BUFFER_SIZE); }
	using i_packer<msg_type>::pack_msg;
	void setIsServer(bool bset) { isServer = bset; }
	virtual void reset() { m_bHandshaked = false; }

	virtual msg_type pack_msg(const char* const pstr[], const size_t len[], size_t num, bool native = false)
	{
		msg_type msg;
		for (int iIndex = 0; iIndex < num; iIndex++)
		{
			if (pstr[iIndex] == nullptr || len[iIndex] == 0) continue;

			if (m_bHandshaked)//已握手
			{
				uint8_t opcode = ((char*)(pstr[iIndex]))[len[iIndex]];//最后一位记录消息类型
				uint64_t msglen = formatMessage(msg_data, (const char*)pstr[iIndex], len[iIndex], (OpCode)opcode, len[iIndex], isServer);
				msg.append(msg_data, msglen);
			}
			else
			{
				m_bHandshaked = true;
				msg.append((const char*)pstr[iIndex], len[iIndex]);
			}
		}

		return msg;

	}
	virtual msg_type pack_heartbeat()
	{
		return pack_ping();
	}
	virtual msg_type pack_ping()
	{
		uint64_t msglen = formatMessage(msg_data, nullptr, 0, eOpCode_PING, 0, isServer);
		return msg_type((const char*)msg_data, msglen, eOpCode_PING);
	}
	virtual msg_type pack_pong()
	{
		uint64_t msglen = formatMessage(msg_data, nullptr, 0, eOpCode_PONG, 0, isServer);
		return msg_type((const char*)msg_data, msglen, eOpCode_PONG);
	}

	static size_t formatMessage(char *dst, const char *src, size_t length, OpCode opCode, size_t reportedLength, bool isServer)
	{
		size_t messageLength;
		size_t headerLength;
		if (reportedLength < 126) 
		{
			headerLength = 2;
			dst[1] = reportedLength;
		}
		else if (reportedLength <= UINT16_MAX)
		{
			headerLength = 4;
			dst[1] = 126;
			*((uint16_t *)&dst[2]) = htons(reportedLength);
		}
		else 
		{
			headerLength = 10;
			dst[1] = 127;

			//数据长度
			char uint64len[8];
			uint64len[0] = (reportedLength >> 56) & 255;
			uint64len[1] = (reportedLength >> 48) & 255;
			uint64len[2] = (reportedLength >> 40) & 255;
			uint64len[3] = (reportedLength >> 32) & 255;
			uint64len[4] = (reportedLength >> 24) & 255;
			uint64len[5] = (reportedLength >> 16) & 255;
			uint64len[6] = (reportedLength >> 8) & 255;
			uint64len[7] = (reportedLength) & 255;

			*((uint64_t *)&dst[2]) = *((uint64_t*)uint64len);// htobe64(reportedLength);
		}

		int flags = 0;
		dst[0] = (flags & SND_NO_FIN ? 0 : 128) | 0;// (compressed ? SND_COMPRESSED : 0);
		if (!(flags & SND_CONTINUATION)) 
		{
			dst[0] |= opCode;
		}

		char mask[MASK_LEN] = { 0 };
		if (!isServer) //服务端发出的消息无掩码
		{
			dst[1] |= 0x80;
			uint32_t random = rand();
			memcpy(mask, &random, MASK_LEN);
			memcpy(dst + headerLength, &random, MASK_LEN);
			headerLength += MASK_LEN;
		}

		//SLOG_DEBUG("send[%d|%d|%d|%d]:%s", mask[0], mask[1], mask[2], mask[3], string(src, length).c_str());

		messageLength = headerLength + length;
		memcpy(dst + headerLength, src, length);

		if (!isServer && length > 0) 
		{
			WsMsg_info::do_mask(dst + headerLength, length, mask);
		}
		return messageLength;
	}

	protected:

		char msg_data[ASCS_MSG_BUFFER_SIZE];
		bool m_bHandshaked;//是否已握手
		bool isServer;//是否服务器端
};

class ws_unpacker : public i_unpacker<ws_message>
{
public:
	ws_unpacker() :m_bHandshaked(false), isServer(true), remain_len(0){ }
	void setIsServer(bool bset) { isServer = bset; }

	virtual void reset() { m_bHandshaked = false; remain_len = 0; }
	virtual bool parse_msg(size_t bytes_transferred, container_type& msg_can)
	{
		if (m_bHandshaked)
		{
			return do_parse_msg(bytes_transferred, msg_can);
		}
		else
		{
			return handshake(bytes_transferred, msg_can);
		}

		//if unpacking failed, successfully parsed msgs will still returned via msg_can(sticky package), please note.
	}

	bool do_parse_msg(size_t bytes_transferred, container_type& msg_can)
	{

		//length + msg
		remain_len += bytes_transferred;
		assert(remain_len <= ASCS_MSG_BUFFER_SIZE);

		auto pnext = &*std::begin(raw_buff);
		auto unpack_ok = true;
		while (unpack_ok) //considering sticky package problem, we need a loop
		{
			cur_msg_len = parse_one_msg(ws_msg_info, pnext, remain_len);//返回解析完此消息使用的长度
			if (cur_msg_len > 0 && ws_msg_info.pData != nullptr)//消息已接收完成
			{
				//SLOG_DEBUG("recv[%d|%d|%d|%d]:%s", ws_msg_info.mask[0], ws_msg_info.mask[1], ws_msg_info.mask[2], ws_msg_info.mask[3], string(ws_msg_info.pData, ws_msg_info.Len).c_str());
				msg_can.emplace_back(ws_msg_info.pData, ws_msg_info.Len, ws_msg_info.type);
				remain_len -= cur_msg_len;
				std::advance(pnext, cur_msg_len);
			}
			else
			{
				break;
			}

			if (remain_len == 0) break;
		}

		if (unpack_ok && remain_len > 0)
		{
			memmove(&*std::begin(raw_buff), pnext, remain_len); //left behind unparsed data
			SLOG_DEBUG("left behind unparsed data:%u", remain_len);
		}

		return true;
	}


	virtual bool handshake(size_t bytes_transferred, container_type& msg_can)
	{
		remain_len += bytes_transferred;

		if (!m_bHandshaked)////未握手
		{
			int parselen = 0;
			auto pnext = &*std::begin(raw_buff);

			string strHandshake = Extensions::onHandshake(pnext, remain_len, parselen, isServer);//生成握手信息
			if (!strHandshake.empty())
			{
				m_bHandshaked = true;
				msg_can.emplace_back((char*)strHandshake.data(), strHandshake.length(), eOpCode_TEXT);

				remain_len -= parselen;
			}
			else if (parselen == -1)//error
			{
				msg_can.emplace_back("", 0, eOpCode_CLOSE);
			}
		}

		return m_bHandshaked;
	}

	virtual size_t completion_condition(const asio::error_code& ec, size_t bytes_transferred) {return ec || bytes_transferred > 0 ? 0 : asio::detail::default_max_transfer_size;}

	virtual buffer_type prepare_next_recv() {return asio::buffer(raw_buff);}



	//从贞数据中取得结构信息,并解码掩码得到原始数据,返回解析完整一条消息使用了多少字节
	uint32_t parse_one_msg(WsMsg_info& msg_info, char* data, size_t dataLen)
	{
		if (data == nullptr || dataLen == 0) return 0;
		msg_info.clear();

		uint32_t headlen = 0;

		msg_info.isEof = (char)(data[0] >> 7);//是否结束
		msg_info.dfExt = (data[0] & 0x70);//扩展码
		msg_info.type = (OpCode)(data[0] & 0xF);//OPCode
		msg_info.hasMask = (char)(data[1] >> 7);


		//Payload长度是ExtensionData长度与ApplicationData长度之和。
		//ExtensionData长度可能是0，这种情况下，Payload长度即是ApplicationData长度(默认ExtensionData长度是0)
		unsigned long tLen = msg_info.Len;
		msg_info.Len = data[1] & 0x7f;

		//当前帧,第一截数据
		if (msg_info.Len < 126) //如果其值在0-125，则是payload的真实长度
		{
			headlen = (msg_info.hasMask) ? 6 : 2;
		}
		else if (msg_info.Len == 126)//126，后面2个字节形成的16位无符号整型数(unsigned short)的值是payload的真实长度，掩码就紧更着后面,此消息最大长度为65535字节
		{
			uint16_t head = *((uint16_t*)(data + 2));
			msg_info.Len = htons(head);// (uint16_t)(data[2] << 8 | data[3]);// ntohll(u.n);//网络字节转换
			headlen = (msg_info.hasMask) ? 8 : 4;
		}
		else if (msg_info.Len == 127)//127，后面8个字节形成的64位无符号整型数(unsigned int64)的值是payload的真实长度，掩码就紧更着后面,此消息最大长度为uint64最大值
		{
			char uInt64Bytes[8];
			for (int i = 0; i < 8; i++)
			{
				uInt64Bytes[i] = data[9 - i];
			}
			msg_info.Len = *(uint64_t *)uInt64Bytes;

			headlen = (msg_info.hasMask) ? 14 : 10;
		}

		if (headlen + msg_info.Len > dataLen) return 0;//消息未接收完整

		if ((dataLen - headlen) > 0 && headlen > MASK_LEN)//防止结尾帧数据不够长度的错误
		{
			if (msg_info.hasMask)
			{
				memcpy(msg_info.mask, &data[headlen - MASK_LEN], MASK_LEN);
			}
		}

		msg_info.pData = data + headlen;

		if (msg_info.Len > 0 && msg_info.hasMask)		//掩码解码/加码
		{
			WsMsg_info::do_mask(msg_info.pData, msg_info.Len, msg_info.mask);
		}

		return (headlen + msg_info.Len);//返回解析此消息使用长度
	}

protected:
	std::array<char, ASCS_MSG_BUFFER_SIZE> raw_buff;//单条消息最大长度为 ASCS_MSG_BUFFER_SIZE 字节
	size_t remain_len; //half-baked msg
	size_t cur_msg_len; //-1 means head not received, so msg length is not available.

	bool m_bHandshaked;
	bool isServer;//是否服务器端
	WsMsg_info  ws_msg_info;

};

#define DEFAULT_WS_PACKER	ws_packer
#define DEFAULT_WS_UNPACKER ws_unpacker

#endif//__ws_unpacker_h__