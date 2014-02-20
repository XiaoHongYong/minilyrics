#include "StdAfx.h"
#include "ClientCom.h"


#ifdef _WIN32
#define WSAError()			WSAGetLastError()
#else
#define WSAError()			(errno + ERR_C_ERRNO_BASE)
#define SOCKET_ERROR		(-1)
#endif

//////////////////////////////////////////////////////////////////////////
//
CClientCom::CClientCom()
{
	m_s = 0;
}

CClientCom::~CClientCom()
{
	if (m_s)
		closesocket(m_s);
}

/*
#define RECV_BUFF_LEN		1024

DWORD WINAPI  CClientCom::ThreadRecv(LPVOID lpParam)
{
	CClientCom *pThis;
	pThis = (CClientCom *)lpParam;
	ASSERT(pThis);

	char*	szBuffer;
	int		nLen;
	int		nTotal;
	CString &buff = pThis->m_bufRecv;

	do
	{
		nTotal = buff.GetLength();
		szBuffer = buff.GetBuffer(RECV_BUFF_LEN + nTotal);

		nLen = pThis->Recieve(szBuffer + nTotal, RECV_BUFF_LEN - 1, 0);
		if (nLen > 0)
		{
			nTotal += nLen;
			szBuffer[nTotal] = _T('\0');
			buff.ReleaseBuffer(nTotal);

			if (pThis->OnRecieve(&buff))
				buff.Empty();
		}
	}
	while (nLen > 0);

	int		nLastErr = WSAGetLastError();

//	DBGLOG1(_T("CClientCom::ThreadRecv() Exited. LastError: %d"), nLastErr);
	// LOG0(LOG_LVL_INFO, _T("CClientCom::ThreadRecv() Exited. Not Release merory and resource!!!!!!!!!"));
	return ERR_OK;
}*/


int CClientCom::Connect(LPCSTR szServer, int nPort)
{
	hostent		*hostEntry;
	int			nRet;
	LPCSTR		szServerAscii = szServer;

    hostEntry = gethostbyname(szServerAscii);
	
    if(!hostEntry)
    {
		return ERR_NET_HOST_NOT_FOUND;
    }

    m_s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	
    if(m_s == SOCKET_ERROR)
    {
		nRet = WSAError();
        return nRet;
    }
	
    // Fill in the sockaddr_in struct
    sockaddr_in serverInfo;

	memset(&serverInfo, 0, sizeof(serverInfo));
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr = *((in_addr *)*hostEntry->h_addr_list);
    serverInfo.sin_port = htons(nPort);

    nRet = connect(m_s, (sockaddr *)&serverInfo, sizeof(serverInfo));
    if(nRet==SOCKET_ERROR)
    {
		nRet = WSAError();
		closesocket(m_s);
		m_s = 0;
        return nRet;
    }

#ifndef _WINCE_
		linger		lingSock;
		lingSock.l_onoff = 1;
		lingSock.l_linger = 20;
		setsockopt(m_s, SOL_SOCKET, SO_LINGER, (const char *)&lingSock, sizeof(lingSock));
#endif

	return ERR_OK;
}

int CClientCom::Connect(LPCWSTR szServer, int nPort)
{
	char		szServerAnsi[128];

	ConvertStr2(szServer, -1, szServerAnsi, CountOf(szServerAnsi));
	return Connect(szServerAnsi, nPort);
}

bool CClientCom::IsValid()
{
	return m_s != 0;
}

//////////////////////////////////////////////////////////////////////////
//
CNetFile::CNetFile()
{
	m_s = 0;
}

CNetFile::~CNetFile()
{
}

int CNetFile::SetTimeOut(int nTimeOut)
{
#if !defined(_WINCE_) && !defined(_IPHONE)
	// 设置接收延时和发送延时
	if (setsockopt(m_s, SOL_SOCKET, SO_SNDTIMEO, (const char *)&nTimeOut, sizeof(nTimeOut)) == SOCKET_ERROR)
	{
		// Task.WriteLog(DOWN_LOG_INFO_ERROR, "Error occured! Set Socket Send TimeOut Error! Scoket Error=%d", WSAGetLastError());
		// errResult = SockErrorToMLError(WSAGetLastError());
		return 	WSAError();
	}
	if (setsockopt(m_s, SOL_SOCKET, SO_RCVTIMEO, (const char *)&nTimeOut, sizeof(nTimeOut)) == SOCKET_ERROR)
	{
		// Task.WriteLog(DOWN_LOG_INFO_ERROR, "Error occured! Set Socket Send TimeOut Error! Scoket Error=%d", WSAGetLastError());
		return 	WSAError();
	}
#endif

	return ERR_OK;
}

int CNetFile::Read(char *szBuffer, int nBufferLen, int *pnRead)
{
	*pnRead = recv(m_s, szBuffer, nBufferLen, 0);
	if (*pnRead == SOCKET_ERROR)
	{
		*pnRead = 0;
		return WSAError();
	}
	return ERR_OK;
}

int CNetFile::ReadExactly(CCharBuffer &buf, int nToRead)
{
	if (nToRead <= 0)
		return ERR_OK;

	if (buf.capacity() - buf.size() < (size_t)nToRead)
		buf.reserve(buf.size() + nToRead);

	while (nToRead > 0)
	{
		int got = recv(m_s, buf.data() + buf.size(), nToRead, 0);
		if (got == SOCKET_ERROR)
			return WSAError();
		else if (got == 0)
			return ERR_NOT_RECIVED_ALL;
		buf.resize(buf.size() + got);
		nToRead -= got;
	}

	return ERR_OK;
}

int CNetFile::ReadTillDataStreamClosed(CCharBuffer &buf)
{
	while (true)
	{
		int nToRead = 4096;
		if (buf.capacity() - buf.size() < (size_t)nToRead)
			buf.reserve(buf.size() + nToRead);

		int got = recv(m_s, buf.data() + buf.size(), nToRead, 0);
		if (got == SOCKET_ERROR)
			return WSAError();
		else if (got == 0)
			return ERR_OK;
		buf.resize(buf.size() + got);
	}

	return ERR_OK;
}

int CNetFile::Read(CCharBuffer &buf, int nMaxToRead, int *pnRead)
{
	if (nMaxToRead == -1)
	{
		*pnRead = 0;

		const int RESERVE_SPACE = 4096;
		while (1)
		{
			if (buf.capacity() - buf.size() < RESERVE_SPACE)
				buf.reserve(buf.size() + RESERVE_SPACE);

			int n = recv(m_s, buf.data() + buf.size(), RESERVE_SPACE, 0);
			if (n == SOCKET_ERROR)
				return WSAError();
			if (n == 0)
				break;
			buf.resize(buf.size() + n);
			*pnRead += n;
		}
	}
	else
	{
		if (buf.capacity() - buf.size() < (size_t)nMaxToRead)
			buf.reserve(buf.size() + nMaxToRead);

		*pnRead = recv(m_s, buf.data() + buf.size(), nMaxToRead, 0);
		if (*pnRead == SOCKET_ERROR)
		{
			*pnRead = 0;
			return WSAError();
		}
		buf.resize(buf.size() + *pnRead);
	}

	return ERR_OK;
}

int CNetFile::Write(const char *szBuffer, int nLen, int *pnWrite)
{
	if (SOCKET_ERROR != send(m_s, szBuffer, nLen, 0))
		return ERR_OK;
	else
		return WSAError();
}

