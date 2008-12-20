#pragma once

#ifndef _WIN32
#define IDC_ARROW           32512
#define IDC_IBEAM           32513
#define IDC_WAIT            32514
#define IDC_CROSS           32515
#define IDC_UPARROW         32516
#define IDC_ICON            32641 
#define IDC_SIZENWSE        32642
#define IDC_SIZENESW        32643
#define IDC_SIZEWE          32644
#define IDC_SIZENS          32645
#define IDC_SIZEALL         32646
#define IDC_NO              32648
#define IDC_HELP            32651
#endif

class CCursor
{
public:

	enum STD_CURSOR_TYPE
	{
		C_HOURGLASS		= (int)IDC_WAIT,
		C_ARROW			= (int)IDC_ARROW,
		C_CROSS			= (int)IDC_CROSS,
		C_HELP			= (int)IDC_HELP,
		C_SIZEALL		= (int)IDC_SIZEALL,
		C_SIZENESW		= (int)IDC_SIZENESW,
		C_SIZENS		= (int)IDC_SIZENS,
		C_SIZENWSE		= (int)IDC_SIZENWSE,
		C_SIZEWE		= (int)IDC_SIZEWE,
		C_NO			= (int)IDC_NO,
		C_IBEAM			= (int)IDC_IBEAM,
	};

	CCursor(void);
	virtual ~CCursor(void);

	bool IsValid() const { return m_cursor != NULL; }

	bool LoadStdCursor(STD_CURSOR_TYPE cusorType);

	bool LoadCursorFromFile(LPCTSTR szFile);

	bool LoadCursorFromRes(int nID);

	void Destroy();

public:
	// for win32 only
	HCURSOR GetHandle() const { return m_cursor; }
	void SetHandle(HCURSOR hCursor) { m_cursor = hCursor; }

public:
	HCURSOR		m_cursor;

};
