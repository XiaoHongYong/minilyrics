// Event.cpp: implementation of the Event class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Event.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Event::Event(bool bManualSet, bool bInitialStat)
{
	m_hObject = CreateEvent(NULL, bManualSet, bInitialStat, NULL);
}

Event::~Event()
{
	CloseHandle(m_hObject);
}

bool Event::Set()
{
	return SetEvent(m_hObject) != 0;
}

bool Event::Reset()
{
	return ResetEvent(m_hObject) != 0;
}

bool Event::Acquire(uint32 nTimeOut)
{
	switch (WaitForSingleObject(m_hObject, nTimeOut))
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

