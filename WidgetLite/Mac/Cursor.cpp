#include "StdAfx.h"
#include "Cursor.h"

#ifdef _WINCE_
HCURSOR LoadCursorFromFile(LPCTSTR szFile)
{
	return LoadCursor(NULL, IDC_NO);
}
#endif

CCursor::CCursor(void)
{
	m_cursor = NULL;
}

CCursor::~CCursor(void)
{
	Destroy();
}

bool CCursor::LoadStdCursor(STD_CURSOR_TYPE cusorType)
{
	return m_cursor != NULL;
}

bool CCursor::LoadCursorFromFile(LPCTSTR szFile)
{
	return m_cursor != NULL;
}

bool CCursor::LoadCursorFromRes(int nID)
{
	return m_cursor != NULL;
}

void CCursor::Destroy()
{
	if (m_cursor)
	{
		m_cursor = NULL;
	}
}
