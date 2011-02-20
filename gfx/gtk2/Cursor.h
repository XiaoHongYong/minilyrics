#ifndef _GFX_CURSOR_INC_
#define _GFX_CURSOR_INC_

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
		C_HOURGLASS		= (int)GDK_WATCH,
		C_ARROW			= (int)GDK_ARROW,
		C_CROSS			= (int)GDK_TREK,
		C_HELP			= (int)GDK_TREK,
		C_SIZEALL		= (int)GDK_FLEUR,
		C_SIZENESW		= (int)GDK_TOP_LEFT_ARROW,
		C_SIZENS		= (int)GDK_SB_V_DOUBLE_ARROW,
		C_SIZENWSE		= (int)GDK_TREK,
		C_SIZEWE		= (int)GDK_SB_H_DOUBLE_ARROW,
		C_NO			= (int)GDK_TREK,
		C_IBEAM			= (int)GDK_XTERM,
		C_HAND			= (int)GDK_ARROW,
	};

	CCursor(void);
	virtual ~CCursor(void);

	bool IsValid() const { return m_cursor != NULL; }

	bool LoadStdCursor(STD_CURSOR_TYPE cusorType);

	bool LoadCursorFromFile(LPCTSTR szFile);

	bool LoadCursorFromRes(int nID);
	
	void Destroy();

public:
	GdkCursor		*m_cursor;

};

#endif // _GFX_CURSOR_INC_
