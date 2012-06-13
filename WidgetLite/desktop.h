#ifndef _DESKTOP_INC_
#define _DESKTOP_INC_

class CBaseWnd;

void OpenUrl(CBaseWnd *pWnd, LPCTSTR szUrl);

// For mac os x, the nFlags is the nFlags in OnKeyDown parameters.
bool IsModifierKeyPressed(int nKey, UINT nFlags = 0);

namespace MLWidget
{
	void GetCursorPos(LPPOINT lpPoint);

	bool SetCursor(CCursor &Cursor, CCursor *pCursorOld = NULL);

	CBaseWnd *FindWindow(LPCTSTR szClassName, LPCTSTR szWindowName);
}

#endif // !defined(_DESKTOP_INC_)
