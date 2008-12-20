#include "StdAfx.h"
#include "Cursor.h"

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
	m_cursor = gdk_cursor_new_for_display(gdk_display_get_default(), GdkCursorType(cusorType));

	return m_cursor != NULL;
}

bool CCursor::LoadCursorFromFile(LPCTSTR szFile)
{
// 	m_cursor = ::LoadCursorFromFile(szFile);
 	return m_cursor != NULL;
}

bool CCursor::LoadCursorFromRes(int nID)
{
// 	m_cursor = ::LoadCursor(GetAppInstance(), MAKEINTRESOURCE(nID));
 	return m_cursor != NULL;
}

void CCursor::Destroy()
{
	if (m_cursor)
	{
		gdk_cursor_unref(m_cursor);
		m_cursor = NULL;
	}
}
