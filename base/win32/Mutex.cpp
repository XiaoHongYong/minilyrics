// Mutex.cpp: implementation of the Mutex class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Mutex.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Mutex::Mutex()
{
	m_hMutex = CreateMutex(	NULL,
		FALSE,
		NULL);
}

Mutex::~Mutex()
{
	CloseHandle(m_hMutex);
}

bool Mutex::Acquire(uint32 nTimeOut)
{
	switch (WaitForSingleObject(m_hMutex, nTimeOut))
	{
	case WAIT_ABANDONED:
	case WAIT_OBJECT_0:
		return true;
	case WAIT_TIMEOUT:
	case WAIT_FAILED:
		break;
	}
	
	return false;
}

void Mutex::Release()
{
	ReleaseMutex(m_hMutex);
}
