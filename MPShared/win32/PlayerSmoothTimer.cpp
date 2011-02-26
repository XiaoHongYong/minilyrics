// PlayerSmoothTimer.cpp: implementation of the CPlayerSmoothTimer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "PlayerSmoothTimer.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define COMPARE_PRECISION		300
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
	// m_nTimeSpan = nTimerSpan;
#ifdef _WIN32_WCE
	m_nTimeSpan = 20;		// 1000 ms / 50 fps
#else
	m_nTimeSpan = 12;		// 1000 ms / 85 fps
#endif
	ASSERT(m_bQuit);
	m_bQuit = false;
	m_thread.Create(ThreadTimer, this);

	return true;
}

void CPlayerSmoothTimer::Stop()
{
	m_bQuit = true;
}

void CPlayerSmoothTimer::AdjustTimer(int nTimePos)
{
	int		n = GetTickCount() - m_dwTimeBeg - nTimePos;

	if (n >= COMPARE_PRECISION || n <= -COMPARE_PRECISION)
	{
		if (m_nTimePosOld != nTimePos)
		{
			m_dwTimeBeg = GetTickCount() - nTimePos;

			g_LyricData.SetPlayElapsedTime(nTimePos);

			CMPlayerAppBase::GetEventsDispatcher()->DispatchUnsyncEvent(ET_LYRICS_DRAW_UPDATE);
		}
	}
	m_nTimePosOld = nTimePos;
}

void CPlayerSmoothTimer::ThreadTimer(LPVOID lpParam)
{
	CPlayerSmoothTimer	*pThis;
	DWORD				dwTimeNow;
	int					n;

	pThis = (CPlayerSmoothTimer*)lpParam;

	while (!pThis->m_bQuit)
	{
		Sleep(pThis->m_nTimeSpan);

		dwTimeNow = GetTickCount();
		// g_wndLyricShow.OnPlayTimeChanged(dwTimeNow - dwTimeBeg);
		n = dwTimeNow - pThis->m_dwTimeBeg;
		if ((int)(n - pThis->m_nTimePosOld) >= COMPARE_PRECISION)
			continue;

		g_LyricData.SetPlayElapsedTime(n);

		CMPlayerAppBase::GetEventsDispatcher()->DispatchUnsyncEvent(ET_LYRICS_DRAW_UPDATE);
	}
}
