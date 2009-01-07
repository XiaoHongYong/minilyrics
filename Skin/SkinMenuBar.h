// SkinMenuBar.h: interface for the CSkinMenuBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_SKIN_MENU_BAR_H_)
#define _SKIN_MENU_BAR_H_

#include "UIObject.h"
#include "../MPShared/SkinMenu.h"

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

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
	void EnumProperties(CUIObjProperties &listProperties);

protected:
	int HitTestSubMenu(const POINT point);
	int GetItemX(int n);

	void TrackPopupMenu();

	void ExitTrackingMenuMode();

protected:
	struct SubMenu
	{
		tstring			strText;
		int				nWidth;
		bool			bHasSubMenu;
		UINT			nID;		// If bHasSubMenu is false, this memember is available.
	};
	typedef vector<SubMenu>		VecSubMenus;

	// CSFImage		m_imgNormal, m_imgHover, m_imgSel;
	// tstring			m_strImgNormal, m_strImgHover, m_strImgSel;
	// BlendPixMode	m_bpm;
	CSkinFontProperty	m_font;
	MLColor				m_clrHover, m_clrOutlinedHover, m_clrPressed, m_clrOutlinedPressed;
	bool				m_bOutlinedHover, m_bOutlinedPressed;

	tstring				m_strMenu;

	CMLMenu				*m_pMenu;

	bool				m_bHover;
	bool				m_bLBtDown;
	bool				m_bMenuPopup;

	VecSubMenus			m_vSubMenus;
	int					m_nSelSubMenu;

};

#endif // !defined(_SKIN_MENU_BAR_H_)
