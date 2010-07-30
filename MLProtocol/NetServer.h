// NetServer.h: interface for the CNetServer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NETSERVER_H__CB6F45A8_33BC_473F_91AA_595B42864240__INCLUDED_)
#define AFX_NETSERVER_H__CB6F45A8_33BC_473F_91AA_595B42864240__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
	int Write(const char *szBuffer, int nLen, int *pnWrite);

protected:
	SOCKET		m_s;

};


//////////////////////////////////////////////////////////////////////////
//

// 任务队列中的任务定义
struct NetJob
{
	int		nSocket;
};

class CNetServer;

class CSvrSessionBase
{
public:
	CSvrSessionBase();
	virtual ~CSvrSessionBase();
	virtual int Init(CNetServer *pServer);
	virtual int Process(SOCKET nSock) = 0;

	// static DWORD WINAPI CSvrSessionBase::ThreadWorker(LPVOID lpParam);
	static void ThreadWorker(LPVOID lpParam);

public:
	CThread				m_thread;
	CNetServer			*m_pServer;

};

class CNetServer
{
public:
	CNetServer();
	virtual ~CNetServer();

	BOOL IsStarted() { return m_vServerPort.size() > 0; }

	struct ServicePort
	{
		SOCKET		sServer;
		tstring		strServer;
		int			nPort;
		CThread		thread;
		CNetServer	*pServer;
	};

public:
	static void ThreadServerAccept(LPVOID lpParam);

	// 创建服务器
	int	Create(int nPort, LPCTSTR szServerIP = NULL, int nWorkThreadCount = 5);

	int CreateSrvice(int nPort, LPCTSTR szServer = NULL);

	// 停止服务器
	void Stop();

	// 添加一个任务到任务队列中
	int AddJob(SOCKET s);

	// 从任务队列中删除一个任务，会阻塞，直到有新任务到达；
	// 返回-1，表示接收到的任务线程需要退出
	int RemoveJob();

	int GetJobCount() const { return (int)m_listJob.size(); }

	int GetWorkThreadCount() const { return m_nWorkThreadCount; }

protected:
	virtual CSvrSessionBase *NewSession();

protected:
	// accept 线程
	vector<ServicePort *>	m_vServerPort;

	// 任务队列
	list<NetJob>		m_listJob;

	// 访问任务队列的互斥量
	Mutex				m_JobMutex;

	// 有任务到达之后的信号
	Semaphore			m_JobSemaphore;

	// 退出信号
	Event				m_ExitEvent;

	// 处理接收的工作线程的数量
	int					m_nWorkThreadCount;
	vector<CSvrSessionBase *>	m_vSessions;

};

#endif // !defined(AFX_NETSERVER_H__CB6F45A8_33BC_473F_91AA_595B42864240__INCLUDED_)
