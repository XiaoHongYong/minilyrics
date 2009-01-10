// SkinMenuBar.h: interface for the CSkinMenuBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_SKIN_MENU_BAR_H_)
#define _SKIN_MENU_BAR_H_

#include "UIObject.h"
#include "../MPShared/SkinMenu.h"

class CSkinMenuBar;

class CMenuBarMsgHandler : public CBaseWnd
{
public:
	enum
	{
		WM_MB_TRACKMENU		= WM_USER + 1,
	};

	CMenuBarMsgHandler()
	{
		m_pMenuBar = NULL;
		m_bMenuPopup = false;
	}

	void PostTrackPopupMenu();

	bool TrackPopupSubMenu(int x, int y, HMENU hMenu, CBaseWnd *pWnd, RECT *prcNotOverlap);

	bool IsMenuPopuped() { return m_bMenuPopup; }


	void OnKeyDown(UINT nChar, UINT nFlags);

	void OnMouseMove(UINT nFlags, CPoint point);

	virtual LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);

protected:
	//
	// Hook messages while menu is popup.
	//
	static LRESULT CALLBACK MessageHookProc(int code, WPARAM wParam, LPARAM lParam);
	static void ProcessHookMessage(UINT message, WPARAM wParam, LPARAM lParam);

	static HHOOK		ms_hMsgHook;
	static bool			ms_bPrimaryMenu;
	static bool			ms_bSubMenuItem;
	static HWND			ms_hWndMsgReceiver;
	static HMENU		ms_hMenuPopup;

public:
	CSkinMenuBar		*m_pMenuBar;

protected:
	bool				m_bMenuPopup;

};

class CSkinMenuBar : public CUIObject  
{
UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
	CSkinMenuBar();
	virtual ~CSkinMenuBar();

	void OnCreate();

	void Draw(CRawGraph *pGraph);

	bool OnLButtonDown(UINT nFlags, CPoint point);
	bool OnLButtonUp(UINT nFlags, CPoint point);
	bool OnLButtonDblClk(UINT nFlags, CPoint point);

	bool OnMouseMove(UINT nFlags, CPoint point);

	void OnKeyDown(UINT nChar, UINT nFlags);

	bool OnMenuKey(UINT nChar, UINT nFlags);

	void OnSetFocus();
	void OnKillFocus();

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
	void EnumProperties(CUIObjProperties &listProperties);

protected:
	int HitTestSubMenu(const POINT point);
	int GetItemX(int n);

	void TrackPopupMenu();

	friend class CMenuBarMsgHandler;

protected:
	struct SubMenu
	{
		tstring			strText;
		int				nWidth;
		bool			bHasSubMenu;
		int				chMenuKey;
		UINT			nID;		// If bHasSubMenu is false, this memember is available.
	};
	typedef vector<SubMenu>		VecSubMenus;

	// CSFImage		m_imgNormal, m_imgHover, m_imgSel;
	// tstring			m_strImgNormal, m_strImgHover, m_strImgSel;
	// BlendPixMode	m_bpm;
	CSkinFontProperty	m_font;
	MLColor				m_clrHover, m_clrOutlinedHover, m_clrPressed, m_clrOutlinedPressed;
	MLColor				m_clrBgPressed;
	bool				m_bOutlinedHover, m_bOutlinedPressed;

	tstring				m_strMenu;

	bool				m_bHover;
	bool				m_bLBtDown;
	POINT				m_ptLast;

	VecSubMenus			m_vSubMenus;
	int					m_nSelSubMenu;

	CMLMenu				*m_pMenu;

	CMenuBarMsgHandler	m_wndMsgReceiver;

};

#endif // !defined(_SKIN_MENU_BAR_H_)
