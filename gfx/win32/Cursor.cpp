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
}

CCursor::~CCursor(void)
{
	Destroy();
}

bool CCursor::LoadStdCursor(STD_CURSOR_TYPE cusorType)
{
	m_cursor = ::LoadCursor(NULL, MAKEINTRESOURCE(cusorType));
	return m_cursor != NULL;
}

bool CCursor::LoadCursorFromFile(LPCTSTR szFile)
{
	m_cursor = ::LoadCursorFromFile(szFile);
	return m_cursor != NULL;
}

bool CCursor::LoadCursorFromRes(int nID)
{
	m_cursor = ::LoadCursor(GetAppInstance(), MAKEINTRESOURCE(nID));
	return m_cursor != NULL;
}

void CCursor::Destroy()
{
	if (m_cursor)
	{
		m_cursor = NULL;
	}
}
