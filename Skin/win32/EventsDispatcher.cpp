#include "StdAfx.h"
#include "EventsDispatcherBase.h"
#include "EventsDispatcher.h"

#define DISPATCH_EVENT_LPARAM		1212

#define MPWM_DISPATCH		(WM_USER + 1)
#define MPWM_QUIT			(WM_USER + 2)

/*
class CEventDispatcherWnd : public CBaseWnd  
{
public:
	void DispatchSyncEventByNoUIThread(IEvent *pEvent);
	void DispatchUnsyncEvent(IEvent *pEvent);

	virtual LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);

};

void CEventDispatcherWnd::DispatchSyncEventByNoUIThread(IEvent *pEvent)
{
	::SendMessage(m_hWnd, MPWM_DISPATCH, (WPARAM)pEvent, DISPATCH_EVENT_LPARAM);
}

void CEventDispatcherWnd::DispatchUnsyncEvent(IEvent *pEvent)
{
	::PostMessage(m_hWnd, MPWM_DISPATCH, (WPARAM)pEvent, DISPATCH_EVENT_LPARAM);
}

LRESULT CEventDispatcherWnd::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case MPWM_DISPATCH:
		{
			if (lParam == DISPATCH_EVENT_LPARAM)
			{
				IEvent *pEvent = (IEvent *)wParam;
				g_eventDispather.DispatchSyncEvent(pEvent);
				return 0;
			}
			break;
		}
	}

	return CBaseWnd::WndProc(message, wParam, lParam);
}
*/
//////////////////////////////////////////////////////////////////////

int CEventsDispatcher::Init()
{
	if (!CBaseWnd::CreateEx(_T("MPMsg"), _T("MPMsg"), 0, 0, 1, 1, NULL, 0))
		return ERR_FALSE;

	return CEventsDispatcherBase::Init();
}

void CEventsDispatcher::Quit()
{
	CEventsDispatcherBase::Quit();

	CBaseWnd::Destroy();
}

void CEventsDispatcher::DispatchSyncEventByNoUIThread(IEvent *pEvent)
{
	::SendMessage(m_hWnd, MPWM_DISPATCH, (WPARAM)pEvent, DISPATCH_EVENT_LPARAM);
}

void CEventsDispatcher::DispatchUnsyncEvent(IEvent *pEvent)
{
	::PostMessage(m_hWnd, MPWM_DISPATCH, (WPARAM)pEvent, DISPATCH_EVENT_LPARAM);
}

LRESULT CEventsDispatcher::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case MPWM_DISPATCH:
		{
			if (lParam == DISPATCH_EVENT_LPARAM)
			{
				IEvent *pEvent = (IEvent *)wParam;
				DispatchSyncEvent(pEvent);
				return 0;
			}
			break;
		}
	}

	return CBaseWnd::WndProc(message, wParam, lParam);
}
