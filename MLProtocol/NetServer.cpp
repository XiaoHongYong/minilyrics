// NetServer.cpp: implementation of the CNetServer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NetSDK.h"
#include "NetServer.h"
#include "HttpProtocol.h"

//
// member for static
int			g_nConnectedClients = 0;
int			g_nDiscardClients = 0;
int			g_nInQueue = 0;

#ifndef IPPROTO_TCP
#define IPPROTO_TCP		0
#endif

#ifdef _WIN32
#define LOGSOCKERR(strMsg)		g_Log.WriteLog(LOG_LVL_ERROR, _T(__FILE__), __LINE__, strMsg, WSAGetLastError())
#define WSAError()			WSAGetLastError();
typedef int			socklen_t;
#else
#define LOGSOCKERR(strMsg)		g_Log.WriteLog(LOG_LVL_ERROR, _T(__FILE__), __LINE__, strMsg, 0)
#define WSAError()			ERR_SOCKET
#define SOCKET_ERROR		(-1)
#define	SD_SEND				SHUT_WR
#endif

//////////////////////////////////////////////////////////////////////////
//


void sock_disable_nagle(int s, struct sockaddr_in *sin_client)
{
	/* The Nagle algorithm says that we should delay sending partial
	* packets in hopes of getting more data.  We don't want to do
	* this; we are not telnet.  There are bad interactions between
	* persistent connections and Nagle's algorithm that have very severe
	* performance penalties.  (Failing to disable Nagle is not much of a
	* problem with simple HTTP.)
	*
	* In spite of these problems, failure here is not a shooting offense.
	*/
#ifdef _WIN32
#ifndef _WINCE_
    int just_say_no = 1;
	
    if (setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char *) &just_say_no,
		sizeof(int)) < 0)
	{
		//        if (sin_client) {
		//            DBGLOG0("setsockopt: (TCP_NODELAY), client %pA probably "
		//                 "dropped the connection", &sin_client->sin_addr);
		//        }
		//        else {
		DBGLOG0(_T("setsockopt: (TCP_NODELAY)"));
		//        }
    }
#endif
#endif
}

int ServerBindListen(int nPort, SOCKET &sServer, LPCTSTR szIPServer)
{
	int		nRet;

    sServer = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sServer == INVALID_SOCKET)
    {
		LOGSOCKERR(_T("socket() FAILED! LastError: %d"));
        return SOCKET_ERROR;
    }

    sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	if (!szIPServer || IsEmptyString(szIPServer))
	{
		sin.sin_family = PF_INET;
		sin.sin_port = htons(nPort);
		sin.sin_addr.s_addr = INADDR_ANY;
	}
	else
	{
		sin.sin_family = PF_INET;
		sin.sin_port = htons(nPort);
		stringex	strIP;
		ConvertStr(szIPServer, -1, strIP);
		sin.sin_addr.s_addr = inet_addr(strIP.c_str());
	}

    // bind the socket
    nRet = bind(sServer, (sockaddr*)&sin, sizeof(sin));
    if(nRet == SOCKET_ERROR)
    {
		nRet = WSAError();
        LOGSOCKERR(_T("bind() FAILED! LastError: %d"));
  		return nRet;
    }

    // get socket to listen 
    nRet = listen(sServer, SOMAXCONN);
    if(nRet == SOCKET_ERROR)
    {
		nRet = WSAError();
        LOGSOCKERR(_T("listen() FAILED! LastError: %d"));
  		return nRet;
    }

	return ERR_OK;
}

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
	// LOG0(LOG_LVL_EVENT_IMP, _T("CClientCom::ThreadRecv() Exited. Not Release merory and resource!!!!!!!!!"));
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
		nRet = WSAError();
        LOGSOCKERR(_T("gethostbyname() FAILED! LastError: %d"));
        return nRet;
    }

    m_s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	
    if(m_s == SOCKET_ERROR)
    {
		nRet = WSAError();
	    LOGSOCKERR(_T("socket() FAILED! LastError: %d"));
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
	    LOGSOCKERR(_T("connect() FAILED! LastError: %d"));
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
// CSvrSessionBase
CSvrSessionBase::CSvrSessionBase()
{
	m_pServer = NULL;
}

CSvrSessionBase::~CSvrSessionBase()
{
	m_thread.Join();
}

int CSvrSessionBase::Init(CNetServer *pServer)
{
	m_pServer = pServer;

	m_thread.Create(ThreadWorker, this);

	return ERR_OK;
}

void CSvrSessionBase::ThreadWorker(LPVOID lpParam)
{
	CSvrSessionBase	*pSession;
	pSession = (CSvrSessionBase *)lpParam;
	assert(pSession);

	while (1)
	{
		SOCKET	nSock;
		nSock = pSession->m_pServer->RemoveJob();
		if (nSock == -1)
			break;

		//
		// 处理
		sock_disable_nagle(nSock, NULL);

#ifndef _WINCE_
		linger		lingSock;
		lingSock.l_onoff = 1;
		lingSock.l_linger = 20;
		setsockopt(nSock, SOL_SOCKET, SO_LINGER, (const char *)&lingSock, sizeof(lingSock));
#endif

		pSession->Process(nSock);

		shutdown(nSock, SD_SEND);
		closesocket(nSock);
	}
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
#ifndef _WINCE_
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
		// DBGLOG0(CWin32Error().Description());
		*pnRead = 0;
		return WSAError();
	}
//	DBGLOGDUMPS(szBuffer, nBufferLen);
	return ERR_OK;
}

int CNetFile::Write(const char *szBuffer, int nLen, int *pnWrite)
{
	if (SOCKET_ERROR != send(m_s, szBuffer, nLen, 0))
		return ERR_OK;
	else
		return WSAError();
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNetServer::CNetServer() : m_JobSemaphore(0),
	m_ExitEvent(TRUE, FALSE)
{
	m_bPaused = FALSE;
	m_nWorkThreadCount = 0;
}

CNetServer::~CNetServer()
{
//	Stop();
}

CSvrSessionBase *CNetServer::NewSession()
{
	return NULL;
}

void CNetServer::ThreadServerAccept(LPVOID lpParam)
{
	CNetServer	*pServer;
	ServicePort	*pServiceInfo;
	pServiceInfo = (ServicePort *)lpParam;
	ASSERT(pServiceInfo);
	pServer = pServiceInfo->pServer;

	sockaddr_in		AddrIn;
	socklen_t		nAddrLen;
	SOCKET			s;
	int				nRet;

	memset(&AddrIn, 0, sizeof(AddrIn));
	nAddrLen = sizeof(AddrIn);

	ASSERT(INVALID_SOCKET == -1);

	while (1)
	{
		if (pServer->m_ExitEvent.Acquire(0))
			break;

		//wait for a client
		// SOCKET client;
		do
		{
			s = accept(pServiceInfo->sServer, (sockaddr*)&AddrIn, &nAddrLen);
		}
#ifdef WIN32
		while (s != INVALID_SOCKET && WSAGetLastError() == WSAEINTR);
#else
		while (s != INVALID_SOCKET);
#endif

		if (s == INVALID_SOCKET)
		{
#ifdef WIN32
			nRet = WSAError();
	        LOGSOCKERR(_T("accept() FAILED! LastError: %d"));
#endif

			if (pServiceInfo->sServer)
				closesocket(pServiceInfo->sServer);

			nRet = ServerBindListen(pServiceInfo->nPort, pServiceInfo->sServer, pServiceInfo->strServer.c_str());

#ifdef _WIN32
			Sleep(1000);
#else
			usleep(1000);
#endif
		}
		else
			pServer->AddJob(s);

/*#define MAX_IN_QUEUE		1024
//	SOCKET		arrSock[MAX_IN_QUEUE];
//	int			nCurCount = 0;
//	fd_set		fdCheck;
//	timeval		timeOut;

//	FD_ZERO(&fdCheck);
//	FD_SET(pServer->m_sServer, &fdCheck);
		if (nCurCount == 0)
		{
			arrSock[nCurCount] = accept(pServer->m_sServer, (SOCKADDR*)&AddrIn, &nAddrLen);
			nCurCount++;
		}
		else if (nCurCount >= MAX_IN_QUEUE)
		{
			Sleep(100);
		}
		else
		{
			timeOut.tv_sec = 0;
			timeOut.tv_usec = 0;
			if (select(0, &fdCheck, NULL, NULL, &timeOut) > 0)
			{
				arrSock[nCurCount] = accept(pServer->m_sServer, (SOCKADDR*)&AddrIn, &nAddrLen);
				nCurCount++;
			}
		}

		if(nCurCount && arrSock[nCurCount - 1] == INVALID_SOCKET)
		{
			int		nRet;
			nRet = WSAError();
	        LOG1(LOG_LVL_ERROR, _T("accept() FAILED! LastError: %d"), WSAGetLastError());
			return nRet;
		}

		// TCHAR	szClientIP[32] = _T("");
		
		// _stprintf(szClientIP, _T("%d.%d.%d.%d: %d"), AddrIn.sin_addr.S_un.S_un_b.s_b1, AddrIn.sin_addr.S_un.S_un_b.s_b2, AddrIn.sin_addr.S_un.S_un_b.s_b3, AddrIn.sin_addr.S_un.S_un_b.s_b4, AddrIn.sin_port);
		// DBGLOG1(_T("A Client: %s Connected to server!"), inet_ntoa(AddrIn.sin_addr));

//		if (!pServer->m_bPaused)
//		{
			if (pServer->AddJob(arrSock, nCurCount) == ERR_OK)
				nCurCount = 0;
//		}
//		else
//			closesocket(arrSock[nCurCount - 1]);*/
	}
}

// 创建服务器
int CNetServer::Create(int nPort, LPCTSTR szServerIP, int nWorkThreadCount)
{
	int		nRet;

	// 创建认为队列的信号量……
	// m_hJobMutex = CreateMutex(NULL, FALSE, NULL);
	// m_hJobSemaphore = CreateSemaphore(NULL, 0, 0x7FFFFFFF, NULL);
	// m_hJobSemaphore = CreateEvent(NULL, FALSE, FALSE, NULL);
	// m_hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	m_nWorkThreadCount = nWorkThreadCount;

	//
	// 设置工作线程参数
	nRet = CreateSrvice(nPort, szServerIP);
	if (nRet != ERR_OK)
		return nRet;

	// 创建会话处理者:)
	for (int i = 0; i < m_nWorkThreadCount; i++)
	{
		CSvrSessionBase	*pSession;
		pSession = NewSession();

		pSession->Init(this);

		m_vSessions.push_back(pSession);
	}

	return ERR_OK;
}

int CNetServer::CreateSrvice(int nPort, LPCTSTR szServer)
{
	ServicePort		*pService;
	int				nRet;

	pService = new ServicePort;

	pService->nPort = nPort;
	if (szServer)
		pService->strServer = szServer;
	pService->pServer = this;

	nRet = ServerBindListen(nPort, pService->sServer, szServer);
	if (nRet != ERR_OK)
	{
		delete pService;
		return nRet;
	}

	// create accept client thread
	if (pService->thread.Create(ThreadServerAccept, pService))
	{
		m_vServerPort.push_back(pService);
		LOG1(LOG_LVL_EVENT_IMP, _T("Server Started At Port: %d"), nPort);
		return ERR_OK;
	}
	else
	{
		delete pService;
		return ERR_FALSE;
	}
}

void CNetServer::Stop()
{
	int		i;

	for (i = 0; i < (int)m_vServerPort.size(); i++)
	{
		ServicePort		*pService = m_vServerPort[i];

		if (pService->sServer)
			closesocket(pService->sServer);
	}

	// 
	m_ExitEvent.Set();

	// 停止工作线程
	for (i = 0; i < m_nWorkThreadCount; i++)
	{
		AddJob(-1);
	}


	// 等待所有线程结束

	for (i = 0; i < (int)m_vSessions.size(); i++)
	{
		m_vSessions[i]->m_thread.Join();
		delete m_vSessions[i];
	}
	m_vSessions.clear();


	for (i = 0; i < (int)m_vServerPort.size(); i++)
	{
		ServicePort		*pService = m_vServerPort[i];

		pService->thread.Join();
		delete pService;
	}
	m_vServerPort.clear();
}


// 添加一个任务到任务队列中
int CNetServer::AddJob(SOCKET s)
{
	m_JobMutex.Acquire();

	g_nConnectedClients++;

	if (m_listJob.size() > 100)
	{
		ERRLOG1(_T("Too many jobs: %d, remove 50"), m_listJob.size());

		for (int i = 0; i < 50; i++)
		{
			closesocket(m_listJob.front().nSocket);
			m_listJob.erase(m_listJob.begin());
			m_JobSemaphore.Wait(10);
		}
		g_nDiscardClients += 50;
	}

	NetJob		job;
	job.nSocket = s;
	m_listJob.push_back(job);

	m_JobSemaphore.Signal();
	g_nInQueue = m_listJob.size();

	m_JobMutex.Release();
	return ERR_OK;
}

// 添加任务到任务队列中
int CNetServer::AddJob(SOCKET *arrSock, int nCount)
{
	// if (WaitForSingleObject(m_hJobMutex, 0) != WAIT_OBJECT_0)
	m_JobMutex.Acquire(0);

	for (int i = 0; i < nCount; i++)
	{
		NetJob		job;
		job.nSocket = arrSock[i];
		// ASSERT(job.nSocket);
		m_listJob.push_back(job);
	}

	if (nCount > 1)
		DBGLOG2(_T("Add Job: %d, %d"), nCount, m_listJob.size());

	m_JobMutex.Release();
	m_JobSemaphore.Signal();
	return ERR_OK;
}

// 从任务队列中删除一个任务，会阻塞，直到有新任务到达；
// 返回-1，表示接收到的任务线程需要退出
int CNetServer::RemoveJob()
{
    int nRet;
/*
    HANDLE hObjects[2];

    hObjects[0] = m_hJobSemaphore;
    hObjects[1] = m_hExitEvent;

    nRet = WaitForMultipleObjects(2, hObjects, FALSE, INFINITE);

    if (nRet == WAIT_OBJECT_0 + 1)
	{
		DBGLOG0("thread got exit now event");
		return -1;
    }
*/
	if (!m_JobSemaphore.Wait(WAIT_FOREVER))
		return -1;

	if (!m_JobMutex.Acquire(WAIT_FOREVER))
		return -1;

	// ASSERT(m_listJob.size() > 0);
	if (m_listJob.size() > 0)
	{
		nRet = m_listJob.front().nSocket;
		// ASSERT(nRet);
		m_listJob.pop_front();
		g_nInQueue--;
	}
	else
		nRet = -1;

	m_JobMutex.Release();

	return nRet;
}

void CNetServer::OnAcceptFailed(int nLastErr)
{
}
