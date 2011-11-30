#ifndef _ML_WIDGETIF_H_
#define _ML_WIDGETIF_H_

#define ID_UNDEFINE		(0)

//
// The Window size state definition
//
enum WndSizeMode
{
	WndSizeMode_Minimized,
	WndSizeMode_Maximized,
	WndSizeMode_Normal,
};

//
//
//
#ifndef WIN32
#define SB_LINEUP           0
#define SB_LINELEFT         0
#define SB_LINEDOWN         1
#define SB_LINERIGHT        1
#define SB_PAGEUP           2
#define SB_PAGELEFT         2
#define SB_PAGEDOWN         3
#define SB_PAGERIGHT        3
#define SB_THUMBPOSITION    4
#define SB_THUMBTRACK       5
#define SB_TOP              6
#define SB_LEFT             6
#define SB_BOTTOM           7
#define SB_RIGHT            7
#define SB_ENDSCROLL        8

#define SW_HIDE             0
#define SW_MAXIMIZE         3
#define SW_SHOW             5
#define SW_MINIMIZE         6
#define SW_RESTORE          9
#define SW_SHOWMINIMIZED	10
#define SW_SHOWMAXIMIZED	11

#define VK_ESCAPE			1
#define VK_CONTROL			3
#define VK_ALT				4
#define VK_SHIFT			5
#define VK_INSERT			6
#define VK_LEFT				7
#define VK_RIGHT			8
#define VK_UP				9
#define VK_DOWN				10
#define VK_PRIOR			11
#define VK_NEXT				12
#define VK_BACK				13
#define VK_DELETE			14
#define VK_RETURN			15
#define VK_END				16
#define VK_HOME				17

#define MK_SHIFT			18
#define MK_CONTROL			19
#define VK_LBUTTON			20
#define VK_SPACE			21
#define VK_TAB				22

#define WHEEL_DELTA			50



#define MB_ICONINFORMATION	10
#define MB_OK				20

#define IDCLOSE				1

#define IDOK				0
#define IDCANCEL			1
#define MK_LBUTTON			1

#endif

//
// Scroll info define
//
class IScrollNotify;
class IScrollBar
{
public:
	virtual void SetScrollInfo(int nMin, int nMax, int nPage, int nPos = 0, int nLine = 1, bool bRedraw = true) = 0;
	virtual int SetScrollPos(int nPos, bool bRedraw = true) = 0;

	virtual int GetScrollPos() const = 0;
	virtual int GetPage() const = 0;
	virtual int GetMin() const = 0;
	virtual int GetMax() const = 0;
	virtual int GetID() const = 0;

	// For CSkinVScrollBar, it will send the scroll notify event to pNotify, or it will send to parent.
	virtual void SetScrollNotify(IScrollNotify *pNofity) = 0;

	virtual bool IsEnabled() const = 0;
	virtual void DisableScrollBar() = 0;

	// For windows scroll bar, IScrollBar need to handle scroll codes and set scroll pos of the scroll bar.
	// This was done in WM_VSCROLL or WM_HSCROLL message handler.
	// If pos is changed, return true.
	virtual bool HandleScrollCode(UINT nSBCode, int nPos) = 0;

};

class IScrollNotify
{
public:
	virtual void OnVScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar) = 0;
	virtual void OnHScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar) = 0;

};


#endif // _ML_WIDGETIF_H_
