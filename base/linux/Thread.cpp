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
	m_lpData = NULL;
}

CThread::~CThread()
{
}

bool CThread::Create(FUNThread function, void* lpData)
{
	m_funThead = function;
	m_lpData = lpData;

	return false;
}

void CThread::Destroy()
{
}

void CThread::Suspend()
{
}

void CThread::Resume()
{
}

void CThread::Join()
{
}

uint32 CThread::GetPriority() const
{
	return 0;
}

uint32 CThread::SetPriority(uint32 priority)
{
	return 0;
}
