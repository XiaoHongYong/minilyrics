#include "StdAfx.h"
#include "TinyBaseWnd.h"
#include <tchar.h>


LRESULT CALLBACK TinyBaseWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_CREATE || uMsg == WM_NCCREATE)
	{
		CREATESTRUCT	*cs = (CREATESTRUCT *)lParam;
		assert(cs->lpCreateParams != NULL);
		CTinyBaseWnd		*pWnd;

		// stash pointer to self
		pWnd = (CTinyBaseWnd *)cs->lpCreateParams;
		if (pWnd)
			pWnd->m_hWnd = hWnd;
		SetWindowLong(hWnd, GWL_USERDATA, (LONG)(cs->lpCreateParams));
	}
	else if (uMsg == WM_PAINT)
	{
		ShowWindow(hWnd, SW_HIDE);
	}

	// pass the messages into the BaseWnd
	CTinyBaseWnd	*pBaseWnd = (CTinyBaseWnd *)GetWindowLong(hWnd, GWL_USERDATA);

	if (pBaseWnd == NULL)
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	else
		return pBaseWnd->WndProc(uMsg, wParam, lParam);
}

CTinyBaseWnd::CTinyBaseWnd()
{
	m_hWnd = NULL;
}

CTinyBaseWnd::~CTinyBaseWnd()
{
}

bool CTinyBaseWnd::Create(HINSTANCE hInstance, LPCTSTR szGetPosWndClass, LPCTSTR szGetPosWndName)
{
	if (!szGetPosWndClass)
		szGetPosWndClass = _T("GetPosWnd");
	if (!szGetPosWndName)
		szGetPosWndName = _T("GetPosWnd");

	WNDCLASS wc;

	if (!GetClassInfo(hInstance, szGetPosWndClass, &wc))
	{
		memset(&wc, 0, sizeof(wc));
		wc.lpfnWndProc = (WNDPROC)TinyBaseWndProc;
		wc.hInstance		= hInstance;
		wc.hbrBackground	= (HBRUSH)GetStockObject(NULL_BRUSH);
		wc.lpszClassName	= szGetPosWndClass;

		int r = RegisterClass(&wc);
		if (r == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
			return FALSE;
	}

	m_hWnd = CreateWindow(szGetPosWndClass, szGetPosWndName, WS_OVERLAPPEDWINDOW,
		0, 0, 100, 100, NULL, NULL, hInstance, this);
	if (!m_hWnd)
		return FALSE;

	ShowWindow(m_hWnd, SW_MINIMIZE);
	ShowWindow(m_hWnd, SW_HIDE);

	return TRUE;
}

LRESULT CTinyBaseWnd::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		OnCreate();
		break;
	case WM_DESTROY:
		OnDestroy();
		SetWindowLong(m_hWnd, GWL_USERDATA, (LONG)0);
		DefWindowProc(m_hWnd, WM_DESTROY, 0, 0);
		m_hWnd = NULL;
		break;
	case WM_TIMER:
		OnTimer(wParam);
		break;
	case WM_COPYDATA:
		OnCopyData(wParam, (COPYDATASTRUCT*)lParam);
		return 1;
	default:
		DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	}

	return 1;
}
