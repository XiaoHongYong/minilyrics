// Event.cpp: implementation of the Event class.
//
//////////////////////////////////////////////////////////////////////

#include "../stdafx.h"
#include "Event.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Event::Event(bool bManualSet, bool bInitialStat)
{
}

Event::~Event()
{
}

bool Event::Set()
{
	return true;
}

bool Event::Reset()
{
	return true;
}

bool Event::Acquire(uint32 nTimeOut)
{
	return false;
}

