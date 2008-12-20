#include "../stdafx.h"
#include "BaseWnd.h"
#include "desktop.h"


void GetMonitorRestrictRect(const RECT &rcIn, RECT &rcRestrict)
{
	rcRestrict.left = 0;
	rcRestrict.right = gdk_screen_width();
	rcRestrict.top = 0;
	rcRestrict.bottom = gdk_screen_height();
}

bool IsKeyPressed(int nKey)
{
	return false;
}

namespace MLWidget
{

void GetCursorPos(POINT *lpPoint)
{
	int				x, y;
	GdkModifierType	mask;

	gdk_display_get_pointer(gdk_display_get_default(), NULL, &x, &y, &mask);

	lpPoint->x = x;
	lpPoint->y = y;
}

// set new cursor, and return the old cursor.
bool SetCursor(CCursor &Cursor, CCursor *pCursorOld)
{
	// ASSERT(0);
	// ERRLOG0("Unimplemented fun: SetCursor()");

	return true;
}

CBaseWnd *FindWindow(LPCTSTR szClassName, LPCTSTR szWindowName)
{
	ASSERT(0);
	return NULL;
}

}
