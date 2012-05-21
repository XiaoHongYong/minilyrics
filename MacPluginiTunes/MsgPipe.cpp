#include <stdio.h>
#include "../base/base.h"
#include "MsgPipe.h"

CMsgPipe::CMsgPipe()
{
	m_fd = -1;
}

CMsgPipe::~CMsgPipe()
{
	Close();
}

bool CMsgPipe::Open(const char *szName, bool isRead)
{
	int nRet = mkfifo(szName, S_IRWXO | S_IRWXG | S_IRWXU);
	if (nRet != 0 && errno != EEXIST)
		return false;

	m_fd = ::open(szName, O_NONBLOCK | (isRead ? O_RDONLY : O_WRONLY));

	return m_fd != -1;
}

void CMsgPipe::Close()
{
	if (m_fd != -1)
	{
		::close(m_fd);
		m_fd = -1;
	}
}


bool CMsgPipe::IsOK()
{
	return m_fd != -1;
}

bool CMsgPipe::Write(uint32 msgId, LPCVOID lpData, size_t len)
{
	Msg msg;
	msg.msgId = msgId;
	msg.len = len;
	msg.type = T_MESSAGE;
	
	if (::write(m_fd, &msg, sizeof(msg)) != sizeof(msg))
		return false;
	return ::write(m_fd, lpData, len);
}

bool CMsgPipe::Write(uint32 msgId, LPCSTR szData)
{
	return Write(msgId, szData, strlen(szData));
}

bool CMsgPipe::Read(uint32 &msgId, string &data)
{
	Msg msg;
	return read(m_fd, &msg, sizeof(msg)) == sizeof(msg);
}

