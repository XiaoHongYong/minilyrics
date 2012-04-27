
#pragma once

#include "../base/OsBaseInc.h"

class CMsgPipe
{
public:
	struct Msg
	{
		uint16 type;
		uint16 len;
		uint32 msg;
	};

	enum MsgType
	{
		T_MESSAGE		= 1,
	};

	bool Open(const char *szName, mode_t mode);
	
	bool IsOK();

	bool Write(uint32 msg, LPCVOID lpData, size_t len);
	bool Write(uint32 msg, LPCSTR szData);

	bool Read(uint32 &msg, string &data);

};
