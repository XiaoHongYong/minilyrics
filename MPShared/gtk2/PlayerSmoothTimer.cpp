// PlayerSmoothTimer.cpp: implementation of the CPlayerSmoothTimer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "../MPlayerApp.h"
#include "PlayerSmoothTimer.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlayerSmoothTimer::CPlayerSmoothTimer()
{
	m_dwTimeBeg = 0;
	m_nTimePosOld = 0;
	m_bQuit = true;
	m_nTimeSpan = 12;
}

CPlayerSmoothTimer::~CPlayerSmoothTimer()
{
}

bool CPlayerSmoothTimer::Start()
{
	ASSERT(m_bQuit);
	m_bQuit = false;

	return true;
}

void CPlayerSmoothTimer::Stop()
{
	m_bQuit = true;
}

void CPlayerSmoothTimer::AdjustTimer(int nTimePos)
{
}
