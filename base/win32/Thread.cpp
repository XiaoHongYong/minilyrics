// Thread.cpp: implementation of the CThread class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Thread.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CThread::CThread()
{
	m_funThead = NULL;
	m_hThread = NULL;
	m_lpData = NULL;
}

CThread::~CThread()
{
	if(m_hThread)
	{
		ASSERT(!IsRunning());
		CloseHandle(m_hThread);
	}
}

bool CThread::Create(FUNThread function, void* lpData)
{
	DWORD		dwThreadId;
	m_funThead = function;
	m_lpData = lpData;

	if (m_hThread)
	{
		ASSERT(!IsRunning());
		CloseHandle(m_hThread);
	}

	m_hThread = ::CreateThread(NULL,
									0,
									ThreadFunction,
									this,
									0,
									&dwThreadId);
	if(m_hThread)
	{
		return true;
	}

	return false;
}

void CThread::Destroy()
{
	if (m_hThread)
		TerminateThread(m_hThread, 0);
}

void CThread::Suspend()
{
	if (m_hThread)
		SuspendThread(m_hThread);
}

void CThread::Resume()
{
	if (m_hThread)
		ResumeThread(m_hThread);
}

void CThread::Join()
{
	if (m_hThread)
		WaitForSingleObject(m_hThread, INFINITE);
}

uint32 CThread::GetPriority() const
{
	if (m_hThread)
		return GetThreadPriority(m_hThread);
	else
		return 0;
}

uint32 CThread::SetPriority(uint32 priority)
{
	if (!m_hThread)
		return 0;

	uint32	nOld;

	nOld = GetThreadPriority(m_hThread);

	SetThreadPriority(m_hThread, priority);

	return nOld;
}

bool CThread::IsRunning()
{
	if (m_hThread)
	{
		if (WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT)
			return true;
	}

	return false;
}

DWORD WINAPI CThread::ThreadFunction(LPVOID lpParam)
{
	CThread	*pwin32Thread;
	pwin32Thread = (CThread *)lpParam;

	pwin32Thread->m_funThead(pwin32Thread->m_lpData);

	return 0;
}
