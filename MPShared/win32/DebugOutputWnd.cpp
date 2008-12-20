// DebugOutputWnd.cpp: implementation of the CDebugOutputWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DebugOutputWnd.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDebugOutputWnd::CDebugOutputWnd()
{

}

CDebugOutputWnd::~CDebugOutputWnd()
{

}

BOOL CDebugOutputWnd::Create()
{
	BOOL		nRet = CBaseWnd::Create(_T("Debug Trace Message"), 0, 0, 600, 300, NULL, DS_NOIDLEMSG | WS_VISIBLE | WS_MINIMIZEBOX | WS_CAPTION | WS_THICKFRAME);

	if (nRet)
	{
		CRect	rc;
		GetClientRect(&rc);
		m_edit.Create(_T(""), rc.left, rc.top, rc.Width(), rc.Height(), this);
		m_edit.ShowWindow(SW_SHOW);
	}

	g_Log.SetTargetWnd(m_hWnd);
/*
	HRGN		hRgn;

	hRgn = CreateRectRgn(0, 0, 1, 1);
	int		n = GetWindowRgn(m_hWnd, hRgn);
	DBGLOG1(_T("Rgn: %d"), n);
	if (n == NULLREGION)
	{
		MessageOut(_T("NULLREGION"));
	}
	else if (n == SIMPLEREGION)
	{
		MessageOut(_T("SIMPLEREGION"));
	}
	else if (n == COMPLEXREGION)
	{
		MessageOut(_T("COMPLEXREGION"));
	}
	else if (n == ERROR)
	{
		MessageOut(_T("ERROR"));
	}
	else 
	{
		MessageOut(_T("UNKNOWN"));
	}

	if (!::SetWindowRgn(m_hWnd, NULL, true))
	{
		MessageOut(_T("Set NULL rgn FAILED!"));
	}

	n = GetWindowRgn(m_hWnd, hRgn);
	DBGLOG1(_T("Rgn: %d"), n);
	if (n == NULLREGION)
	{
		MessageOut(_T("NULLREGION"));
	}
	else if (n == SIMPLEREGION)
	{
		MessageOut(_T("SIMPLEREGION"));
	}
	else if (n == COMPLEXREGION)
	{
		MessageOut(_T("COMPLEXREGION"));
	}
	else if (n == ERROR)
	{
		MessageOut(_T("ERROR"));
	}
	else 
	{
		MessageOut(_T("UNKNOWN"));
	}*/

	return nRet;
}

void CDebugOutputWnd::OnSize(int cx, int cy)
{
	if (m_edit.GetHandle())
	{
		CRect	rc;
		GetClientRect(&rc);
		m_edit.MoveWindow(rc.left, rc.top, rc.Width(), rc.Height(), true);
	}
}

LRESULT CDebugOutputWnd::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_COPYDATA)
	{
		#define SENDDEBUGMESSAGE	19771212
		COPYDATASTRUCT *data = (COPYDATASTRUCT*)lParam;
		if (data->dwData == SENDDEBUGMESSAGE)
		{
			int		ilen = m_edit.GetLength();
			m_edit.SetSel(ilen, ilen);
			tstringex	str;
			ConvertStr((LPCSTR)data->lpData, -1, str);
			str += _T(" \r\n");
			m_edit.ReplaceSel(str.c_str());
		}
		return 0;
	}

	return CBaseWnd::WndProc(message, wParam, lParam);
}
