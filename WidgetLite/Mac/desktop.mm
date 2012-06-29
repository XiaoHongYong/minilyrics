#include "stdafx.h"
#include "widgetif.h"
#include "BaseWnd.h"
#include "desktop.h"


void OpenUrl(CBaseWnd *pWnd, LPCTSTR szUrl)
{
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[NSString stringWithUTF8String:szUrl]]];
}

bool IsModifierKeyPressed(int nKey, UINT nFlags)
{
	ASSERT(nKey == MK_SHIFT || nKey == MK_CONTROL || nKey == (1 << 20));
	return IsFlagSet(nFlags, nKey);
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

