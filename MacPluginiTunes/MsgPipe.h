
#pragma once

#include "../base/OsBaseInc.h"

/*
 All text in message will be encoded.
 
 * Message items will be spearated with 0xFF
 * 0xFE in message will be encoded as 0xFE 0x01
 * 0xFF in message will be encoded as 0xFE 0x02
*/

class CMsgPipe
{
public:
	struct Msg
	{
		uint16 type;
		uint16 len;
		uint32 msgId;
	};

	enum MsgType
	{
		T_MESSAGE		= 1,
	};

	CMsgPipe();
	virtual ~CMsgPipe();

	bool Open(const char *szName, bool isRead);
	void Close();

	bool Write(uint32 msgId, LPCVOID lpData, size_t len);
	bool Write(uint32 msgId, LPCSTR szData);

	bool Read(uint32 &msgId, string &data);
	
protected:
	bool WriteEncoded(LPCVOID lpData, size_t len);
	bool ReadEncodedMessage(string &data);
	bool ReadToSeparator();
	
protected:
	int m_fd;
	string m_pipeName;
	byte *m_readBuf;
	int m_nSizeRead, m_nPosRead;

};
