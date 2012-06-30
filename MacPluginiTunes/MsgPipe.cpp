#include <stdio.h>
#include "../base/base.h"
#include "MsgPipe.h"

#define MSG_ESCAPE_CHAR		(byte)(0xFE)
#define MSG_SEP_CHAR		(byte)(0xFF)

#define MSG_ESCAPE_CHAR_TO	(byte)(0x1)
#define MSG_SEP_CHAR_TO		(byte)(0x2)

#define LEN_BUF_READ			512

CMsgPipe::CMsgPipe()
{
	m_fd = -1;
	m_readBuf = NULL;
	m_nSizeRead = m_nPosRead = 0;
}

CMsgPipe::~CMsgPipe()
{
	Close();
}

bool CMsgPipe::Open(const char *szName, bool isRead)
{
	m_pipeName = szName;
	int nRet = mkfifo(szName, S_IRWXO | S_IRWXG | S_IRWXU);
	if (nRet != 0 && errno != EEXIST)
		return false;

	return true;
}

void CMsgPipe::Close()
{
	if (m_fd != -1)
	{
		::close(m_fd);
		m_fd = -1;
	}
	
	if (m_readBuf != NULL)
	{
		delete[] m_readBuf;
		m_readBuf = NULL;
	}
}

bool CMsgPipe::Write(uint32 msgId, LPCVOID lpData, size_t len)
{
	if (m_fd == -1)
	{
		m_fd = ::open(m_pipeName.c_str(), O_NONBLOCK | O_WRONLY);
		if (m_fd == -1)
			return false;
	}

	byte separator = MSG_SEP_CHAR;
	if (::write(m_fd, &separator, sizeof(separator)) != sizeof(separator))
		return false;

	if (!WriteEncoded(&msgId, sizeof(msgId)))
		return false;

	if (!WriteEncoded(lpData, len))
		return false;

	if (::write(m_fd, &separator, sizeof(separator)) != sizeof(separator))
		return false;
	return true;
}

bool CMsgPipe::Write(uint32 msgId, LPCSTR szData)
{
	return Write(msgId, szData, strlen(szData));
}

/*
void Log(LPCTSTR szFormat, ...);

void PrintBuf(const char *Title, byte *buf, int len)
{
	string s;
	char tmp[4];
	for (int i = 0; i < len; i++)
	{
		sprintf(tmp, "%02x", buf[i]);
		s.append(tmp, 2);
	}
	Log("%s: %s", Title, s.c_str());
}*/

bool CMsgPipe::WriteEncoded(LPCVOID lpData, size_t len)
{
	// PrintBuf("Before", (byte*)lpData, len);

	byte *p = (byte *)lpData;
	const int LEN_BUF = 256;
	byte buf[LEN_BUF + 2];
	while (len > 0)
	{
		int i = 0;
		while (i < LEN_BUF && len > 0)
		{
			len--;
			if (*p == MSG_SEP_CHAR)
				buf[i + 1] = MSG_SEP_CHAR_TO;
			else if (*p == MSG_ESCAPE_CHAR)
				buf[i + 1] = MSG_ESCAPE_CHAR_TO;
			else
			{
				buf[i] = *p;
				i++;
				p++;
				continue;
			}

			buf[i] = MSG_ESCAPE_CHAR;
			i += 2;
			p++;
		}
		// PrintBuf("Encoded", buf, i);
		if (::write(m_fd, buf, i) != i)
			return false;
	}
	
	return true;
}

bool CMsgPipe::ReadEncodedMessage(string &data)
{
	if (m_readBuf == NULL)
		m_readBuf = new byte[LEN_BUF_READ];

	// Read to seppartor char
	if (!ReadToSeparator())
		return false;
	
	while (true)
	{
		// PrintBuf("RawRead", m_readBuf + m_nPosRead, m_nSizeRead - m_nPosRead);

		int start = m_nPosRead;
		for (; m_nPosRead < m_nSizeRead; m_nPosRead++)
		{
			if (m_readBuf[m_nPosRead] == MSG_SEP_CHAR)
			{
				data.append((const char *)m_readBuf + start, m_nPosRead - start);
				m_nPosRead++;
				// PrintBuf("Decoded", (byte*)data.c_str(), data.size());
				return true;
			}
			else if (m_readBuf[m_nPosRead] == MSG_ESCAPE_CHAR)
			{
				data.append((const char *)m_readBuf + start, m_nPosRead - start);
				if (m_readBuf[m_nPosRead + 1] == MSG_SEP_CHAR_TO)
					data.append(1, (char)MSG_SEP_CHAR);
				else if (m_readBuf[m_nPosRead + 1] == MSG_ESCAPE_CHAR_TO)
					data.append(1, (char)MSG_ESCAPE_CHAR);
				m_nPosRead++;
				start = m_nPosRead + 1;
			}
		}
		data.append((const char *)m_readBuf + start, m_nPosRead - start);
		
		timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		fd_set rdset;
		FD_ZERO(&rdset);
		FD_SET(m_fd, &rdset);
		int ret = ::select(m_fd + 1, &rdset, NULL, NULL, &timeout);
		if (ret < 0)
			return false;
		else if (ret == 0) {
			if (m_fd == -1)
				return false;
			continue;
		}

		m_nSizeRead = read(m_fd, m_readBuf, LEN_BUF_READ);
		m_nPosRead = 0;
		if (m_nSizeRead <= 0)
			return false;
	}

	return false;
}

bool CMsgPipe::ReadToSeparator()
{
	while (true)
	{
		for (; m_nPosRead < m_nSizeRead; m_nPosRead++)
		{
			if (m_readBuf[m_nPosRead] == MSG_SEP_CHAR)
			{
				m_nPosRead++;
				if (m_nPosRead < m_nSizeRead && m_readBuf[m_nPosRead] == MSG_SEP_CHAR)
					m_nPosRead++;
				return true;
			}
		}
		
		timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		fd_set rdset;
		FD_ZERO(&rdset);
		FD_SET(m_fd, &rdset);
		int ret = ::select(m_fd + 1, &rdset, NULL, NULL, &timeout);
		if (ret < 0)
			return false;
		else if (ret == 0) {
			if (m_fd == -1)
				return false;
			continue;
		}
		m_nSizeRead = read(m_fd, m_readBuf, LEN_BUF_READ);
		m_nPosRead = 0;
		if (m_nSizeRead <= 0)
			return false;
	}
	
	return false;
}


bool CMsgPipe::Read(uint32 &msgId, string &data)
{
	if (m_fd == -1)
	{
		m_fd = ::open(m_pipeName.c_str(), O_RDONLY);
		if (m_fd == -1)
			return false;
	}
	
	data.clear();
	
	if (!ReadEncodedMessage(data))
		return false;
	
	msgId = *((uint32 *)data.c_str());

	data.erase(0, sizeof(uint32));

	return true;
}

