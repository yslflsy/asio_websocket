//
//  ws_extensions.h
//  
//  Created by qiyun on 19-1-18.
//  Copyright (c) 2019 __jin10.com__. All rights reserved.
//

#ifndef __ws_extensions_h__
#define __ws_extensions_h__

#include <iostream>
#include <openssl/sha.h>
#include <stdint.h>

using namespace std;

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)

#pragma comment(lib, "libeay32.lib")
#pragma comment(lib, "ssleay32.lib")

#endif

#define MAX_HEADERS 100
#define MAX_UINT(_x_, _y_)  (_x_ > _y_ ? _x_ : _y_)
# define SHA1_DIGEST_LENGTH 20

namespace Extensions
{

	// UNSAFETY NOTE: assumes 24 byte input length
	static void base64(unsigned char *src, char *dst)
	{
		static const char *b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		for (int i = 0; i < 18; i += 3)
		{
			*dst++ = b64[(src[i] >> 2) & 63];
			*dst++ = b64[((src[i] & 3) << 4) | ((src[i + 1] & 240) >> 4)];
			*dst++ = b64[((src[i + 1] & 15) << 2) | ((src[i + 2] & 192) >> 6)];
			*dst++ = b64[src[i + 2] & 63];
		}
		*dst++ = b64[(src[18] >> 2) & 63];
		*dst++ = b64[((src[18] & 3) << 4) | ((src[19] & 240) >> 4)];
		*dst++ = b64[((src[19] & 15) << 2)];
		*dst++ = '=';
	}

	struct Header
	{
		char *key, *value;
		unsigned int keyLength, valueLength;

		operator bool() { return key != nullptr; }
		std::string toString() { return std::string(value, valueLength); }
	};


	enum HttpMethod
	{
		METHOD_GET,
		METHOD_POST,
		METHOD_PUT,
		METHOD_DELETE,
		METHOD_PATCH,
		METHOD_OPTIONS,
		METHOD_HEAD,
		METHOD_TRACE,
		METHOD_CONNECT,
		METHOD_INVALID
	};

	struct HttpRequest
	{
		Header *headers;
		Header getHeader(const char *key) { return getHeader(key, strlen(key)); }

		HttpRequest(Header *headers = nullptr) : headers(headers) {}

		Header getHeader(const char *key, size_t length)
		{
			if (headers)
			{
				for (Header *h = headers; *++h; ) { if (h->keyLength == length && !strncmp(h->key, key, length)) { return *h; } }
			}
			return{ nullptr, nullptr, 0, 0 };
		}

		Header getUrl()
		{
			if (headers->key) { return *headers; }
			return{ nullptr, nullptr, 0, 0 };
		}

		HttpMethod getMethod()
		{
			if (!headers->key) { return METHOD_INVALID; }

			switch (headers->keyLength)
			{
			case 3:
				if (!strncmp(headers->key, "get", 3)) { return METHOD_GET; } else if (!strncmp(headers->key, "put", 3)) { return METHOD_PUT; }
				break;
			case 4:
				if (!strncmp(headers->key, "post", 4)) { return METHOD_POST; } else if (!strncmp(headers->key, "head", 4)) { return METHOD_HEAD; }
				break;
			case 5:
				if (!strncmp(headers->key, "patch", 5)) { return METHOD_PATCH; } else if (!strncmp(headers->key, "trace", 5)) { return METHOD_TRACE; }
				break;
			case 6:
				if (!strncmp(headers->key, "delete", 6)) { return METHOD_DELETE; }
				break;
			case 7:
				if (!strncmp(headers->key, "options", 7)) { return METHOD_OPTIONS; } else if (!strncmp(headers->key, "connect", 7)) 	{ return METHOD_CONNECT; }
				break;
			}
			return METHOD_INVALID;
		}
	};

	inline uint32_t parse_url(const string &url)
	{
		string strtmp;
		asio::io_context ios;
		asio::ip::tcp::resolver slv(ios);	//创建resolver对象
		asio::ip::basic_resolver_results<asio::ip::tcp> ret = slv.resolve(url, strtmp);	//使用resolve迭代端点

		uint32_t iSize = 0;
		for (auto itr = ret.begin(); itr != ret.end(); itr++)
		{
			SLOG_INFO("%s:%d", itr->endpoint().address().to_string().c_str(), itr->endpoint().port());
			iSize++;
		}

		return iSize;
	}

	inline bool parseURI(std::string &uri, bool &ssl, std::string &hostname, int &port, std::string &path)
	{
		port = 80;
		ssl = false;
		size_t offset = 5;
		if (!uri.compare(0, 6, "wss://"))
		{
			port = 443;
			ssl = true;
			offset = 6;
		}
		else if (uri.compare(0, 5, "ws://"))
		{
			return false;
		}

		if (offset == uri.length())  return false;


		if (uri[offset] == '[')
		{
			if (++offset == uri.length())  return false;

			size_t endBracket = uri.find(']', offset);
			if (endBracket == std::string::npos)
			{
				return false;
			}
			hostname = uri.substr(offset, endBracket - offset);
			offset = endBracket + 1;
		}
		else
		{
			hostname = uri.substr(offset, uri.find_first_of(":/", offset) - offset);
			offset += hostname.length();
		}

		if (offset == uri.length()) { path.clear(); return true; }

		if (uri[offset] == ':')
		{
			offset++;
			std::string portStr = uri.substr(offset, uri.find('/', offset) - offset);
			if (portStr.length())
			{
				try { port = stoi(portStr); } catch (...) { return false; }
			}
			else
			{
				return false;
			}
			offset += portStr.length();
		}

		if (offset == uri.length()) { 	path.clear(); return true; }

		if (uri[offset] == '/') { path = uri.substr(++offset); }
		return true;
	}


	inline string clientHandshakeString(const string & hostname, int port, const string &path)
	{
		std::string randomKey = "x3JJHMbDL1EzLkh9GBhXDw==";

		string str = "GET /" + path + " HTTP/1.1\r\n"
			"Upgrade: websocket\r\n"
			"Connection: Upgrade\r\n"
			"Sec-WebSocket-Key: " + randomKey + "\r\n"
			"Host: " + hostname + ":" + std::to_string(port) + "\r\n"
			"Sec-WebSocket-Version: 13\r\n";

		str += "\r\n";
		return str;
	}

	// UNSAFETY NOTE: assumes *end == '\r' (might unref end pointer)
	inline char *getHeaders(char *buffer, char *end, Header *headers, size_t maxHeaders)
	{
		for (unsigned int i = 0; i < maxHeaders; i++) {
			for (headers->key = buffer; (*buffer != ':') & (*buffer > 32); *(buffer++) |= 32);
			if (*buffer == '\r') {
				if ((buffer != end) & (buffer[1] == '\n') & (i > 0)) {
					headers->key = nullptr;
					return buffer + 2;
				}
				else 
				{
					return nullptr;
				}
			}
			else 
			{
				headers->keyLength = (unsigned int)(buffer - headers->key);
				for (buffer++; (*buffer == ':' || *buffer < 33) && *buffer != '\r'; buffer++);
				headers->value = buffer;
				buffer = (char *)memchr(buffer, '\r', end - buffer); //for (; *buffer != '\r'; buffer++);
				if (buffer /*!= end*/ && buffer[1] == '\n') 
				{
					headers->valueLength = (unsigned int)(buffer - headers->value);
					buffer += 2;
					headers++;
				}
				else 
				{
					return nullptr;
				}
			}
		}
		return nullptr;
	}

	inline string buildServerHandshakeString(const char *secKey, const char *subprotocol, size_t subprotocolLength)
	{
		std::string extensionsResponse;

		unsigned char shaInput[] = "XXXXXXXXXXXXXXXXXXXXXXXX258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
		memcpy(shaInput, secKey, 24);

 		unsigned char shaDigest[SHA1_DIGEST_LENGTH];
 		SHA1(shaInput, sizeof(shaInput) - 1, shaDigest);

		char upgradeBuffer[1024];
		memcpy(upgradeBuffer, "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: ", 97);
		Extensions::base64((unsigned char*)shaDigest, upgradeBuffer + 97);
		memcpy(upgradeBuffer + 125, "\r\n", 2);
		size_t upgradeResponseLength = 127;

		if (extensionsResponse.length() && extensionsResponse.length() < 200) 
		{
			memcpy(upgradeBuffer + upgradeResponseLength, "Sec-WebSocket-Extensions: ", 26);
			memcpy(upgradeBuffer + upgradeResponseLength + 26, extensionsResponse.data(), extensionsResponse.length());
			memcpy(upgradeBuffer + upgradeResponseLength + 26 + extensionsResponse.length(), "\r\n", 2);
			upgradeResponseLength += 26 + extensionsResponse.length() + 2;
		}
		// select first protocol
		for (unsigned int i = 0; i < subprotocolLength; i++) 
		{
			if (subprotocol[i] == ',') 
			{
				subprotocolLength = i;
				break;
			}
		}

		if (subprotocolLength && subprotocolLength < 200) 
		{
			memcpy(upgradeBuffer + upgradeResponseLength, "Sec-WebSocket-Protocol: ", 24);
			memcpy(upgradeBuffer + upgradeResponseLength + 24, subprotocol, subprotocolLength);
			memcpy(upgradeBuffer + upgradeResponseLength + 24 + subprotocolLength, "\r\n", 2);
			upgradeResponseLength += 24 + subprotocolLength + 2;
		}

		static char stamp[] = "Sec-WebSocket-Version: 13\r\nWebSocket-Server: asws-websockets\r\n\r\n";
		memcpy(upgradeBuffer + upgradeResponseLength, stamp, sizeof(stamp) - 1);
		upgradeResponseLength += sizeof(stamp) - 1;

		return string(upgradeBuffer, upgradeResponseLength);

	}


	inline string onHandshake(char *data, uint32_t length, int & parselen, bool isServer)
	{
		parselen = 0;
		char *end = data + length;
		char *cursor = data;
		*end = '\r';
		Header headers[MAX_HEADERS];
		do {
			char *lastCursor = cursor;
			cursor = getHeaders(cursor, end, headers, MAX_HEADERS);
			if (cursor == nullptr) break;
			HttpRequest req(headers);

			if(isServer)//server
			{
				headers->valueLength = MAX_UINT(0, headers->valueLength - 9);
				if (req.getHeader("upgrade", 7))
				{
					Header secKey = req.getHeader("sec-websocket-key", 17);
					Header extensions = req.getHeader("sec-websocket-extensions", 24);
					Header subprotocol = req.getHeader("sec-websocket-protocol", 22);
					if (secKey.valueLength == 24)
					{
						string str = buildServerHandshakeString(secKey.value, subprotocol.value, subprotocol.valueLength);
						parselen = (end - data);
						return str;
					}
					else//will close this connection
					{
						parselen = -1;
					}
				}
			}
			else//client
			{
				if (req.getHeader("upgrade", 7))
				{
					parselen = length; 
					return string(data, length);
				}
				else //will close this connection
				{
					parselen = -1;
				}
			}

		} while (cursor != end);

		return "";
	}



};

#endif // __ws_extensions_h__
