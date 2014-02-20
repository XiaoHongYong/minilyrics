#pragma once

#include "sockadapt.h"

//////////////////////////////////////////////////////////////////////////
//

class CClientCom
{
public:
	CClientCom();
	virtual ~CClientCom();

	bool IsValid();

	int Connect(LPCSTR szServer, int nPort);

	int Connect(LPCWSTR szServer, int nPort);

	int Recieve(char * buf, int nLen, int flags = 0)
	{ return recv(m_s, buf, nLen, flags); }

	int Send(const char *buf, int nLen, int flags = 0)
	{ return send(m_s, buf, nLen, flags); }

	int Close() { closesocket(m_s); m_s = 0; return 0; }

	SOCKET GetSocket() { return m_s; }

protected:
	SOCKET		m_s;

};

//////////////////////////////////////////////////////////////////////////
//

class CNetFile
{
public:
	CNetFile();
	~CNetFile();

	int Attach(SOCKET s) { m_s = s; return 0; }
	SOCKET GetHandle() { return m_s; }

	void Close() { closesocket(m_s); }

	int SetTimeOut(int nTimeOut);

	int Read(char *szBuffer, int nBufferLen, int *pnRead);
	int Read(CCharBuffer &buf, int nMaxToRead, int *pnRead);
	int ReadExactly(CCharBuffer &buf, int nToRead);
	int ReadTillDataStreamClosed(CCharBuffer &buf);
	int Write(const char *szBuffer, int nLen, int *pnWrite);

protected:
	SOCKET		m_s;

};
