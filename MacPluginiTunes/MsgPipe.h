
#pragma once

#include "../base/OsBaseInc.h"

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

	bool IsOK();

	bool Write(uint32 msgId, LPCVOID lpData, size_t len);
	bool Write(uint32 msgId, LPCSTR szData);

	bool Read(uint32 &msgId, string &data);
	
protected:
	int m_fd;

};
