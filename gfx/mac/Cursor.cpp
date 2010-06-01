#include "StdAfx.h"
#include "Cursor.h"

CCursor::CCursor(void)
{
}

CCursor::~CCursor(void)
{
	Destroy();
}

bool CCursor::LoadStdCursor(STD_CURSOR_TYPE cusorType)
{
	return false;
}

bool CCursor::LoadCursorFromFile(LPCTSTR szFile)
{
	return false;
}

bool CCursor::LoadCursorFromRes(int nID)
{
	return false;
}

void CCursor::Destroy()
{
}
