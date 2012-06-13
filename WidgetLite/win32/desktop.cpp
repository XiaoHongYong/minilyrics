#include "stdafx.h"
#include "BaseWnd.h"

void OpenUrl(CBaseWnd *pWnd, LPCTSTR szUrl)
{
	ShellExecute(pWnd->GetHandle(), _T("open"), szUrl, NULL, NULL, SW_SHOWNORMAL);
}

bool IsModifierKeyPressed(int nKey, UINT nFlags)
{
	ASSERT(nKey == MK_SHIFT || nKey == MK_CONTROL);
	if (nKey == MK_SHIFT)
		nKey = VK_SHIFT;
	else if (nKey == MK_CONTROL)
		nKey = VK_CONTROL;

	return (GetKeyState(nKey) & 0x8000) == 0x8000;
}

namespace MLWidget
{
void GetCursorPos(LPPOINT lpPoint)
{
	::GetCursorPos(lpPoint);
}

// set new cursor, and return the old cursor.
bool SetCursor(CCursor &Cursor, CCursor *pCursorOld)
{
	HCURSOR		hCursorOld;
	hCursorOld = ::SetCursor(Cursor.m_cursor);

	if (pCursorOld)
		pCursorOld->m_cursor = hCursorOld;

	return true;
}

CBaseWnd *FindWindow(LPCTSTR szClassName, LPCTSTR szWindowName)
{
	HWND		hWnd = ::FindWindow(szClassName, szWindowName);
	if (!hWnd)
		return NULL;

	return CBaseWnd::FromHandle(hWnd);
}

}

#ifdef _WINCE_
HINSTANCE ShellExecute(HWND hwnd, LPCTSTR lpOperation, LPCTSTR lpFile, LPCTSTR lpParameters, LPCTSTR lpDirectory, INT nShowCmd)
{
	SHELLEXECUTEINFO	info;
	memset(&info, 0, sizeof(info));
	info.cbSize = sizeof(info);
	info.hwnd = hwnd;
	info.lpVerb = lpOperation;
	info.lpFile = lpFile;
	info.lpParameters = lpParameters;
	info.lpDirectory = lpDirectory;
	info.nShow = nShowCmd;
	if (ShellExecuteEx(&info))
		return info.hInstApp;

	return NULL;
}
#endif
