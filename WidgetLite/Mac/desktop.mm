#include "stdafx.h"
#include "widgetif.h"
#include "BaseWnd.h"
#include "desktop.h"


void OpenUrl(CBaseWnd *pWnd, LPCTSTR szUrl)
{
#pragma warning("Unplememented method: OpenUrl");
}

bool IsKeyPressed(int nKey)
{
	// return (GetKeyState(nKey) & 0x8000) == 0x8000;
#pragma warning("Unplememented method: IsKeyPressed");
	return false;
}

namespace MLWidget
{
void GetCursorPos(LPPOINT lpPoint)
{
	NSPoint pt = [NSEvent mouseLocation];
	lpPoint->x = pt.x;
	lpPoint->y = pt.y;
}

// set new cursor, and return the old cursor.
bool SetCursor(CCursor &Cursor, CCursor *pCursorOld)
{
#pragma warning("Unplememented method: ");

	return true;
}

CBaseWnd *FindWindow(LPCTSTR szClassName, LPCTSTR szWindowName)
{
#pragma warning("Unplememented method: ");

	return NULL;
}

}

