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
		C_HOURGLASS		,
		C_ARROW			,
		C_CROSS			,
		C_HELP			,
		C_SIZEALL		,
		C_SIZENESW		,
		C_SIZENS		,
		C_SIZENWSE		,
		C_SIZEWE		,
		C_NO			
		C_IBEAM			,
	};

	CCursor(void);
	virtual ~CCursor(void);

	bool IsValid() const { return false; }

	bool LoadStdCursor(STD_CURSOR_TYPE cusorType);

	bool LoadCursorFromFile(LPCTSTR szFile);

	bool LoadCursorFromRes(int nID);
	
	void Destroy();

public:

};

#endif // _GFX_CURSOR_INC_
