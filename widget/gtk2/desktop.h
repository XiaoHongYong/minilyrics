#ifndef _DESKTOP_INC_
#define _DESKTOP_INC_

class CBaseWnd;

#include "BaseWnd.h"

void GetMonitorRestrictRect(const RECT &rcIn, RECT &rcRestrict);

bool IsKeyPressed(int nKey);

namespace MLWidget
{
	void GetCursorPos(POINT *lpPoint);

	bool SetCursor(CCursor &Cursor, CCursor *pCursorOld = NULL);

	CBaseWnd *FindWindow(LPCTSTR szClassName, LPCTSTR szWindowName);
}

#endif // !defined(_DESKTOP_INC_)
