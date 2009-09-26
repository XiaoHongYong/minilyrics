// Semaphore.cpp: implementation of the Semaphore class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <limits.h>
#include "Semaphore.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Semaphore::Semaphore(int nCount)
{
    m_nCount = nCount;
    m_hSemaphore = CreateSemaphore(NULL, 
		nCount,
		LONG_MAX,
		_T(""));
}

Semaphore::~Semaphore()
{
	CloseHandle(m_hSemaphore);
}

bool Semaphore::Wait(int ms)
{
	if (WaitForSingleObject(m_hSemaphore, ms) == WAIT_TIMEOUT)
		return false;

	return true;
}

void Semaphore::Signal()
{
    ReleaseSemaphore(m_hSemaphore,
		1,
		NULL);
}
