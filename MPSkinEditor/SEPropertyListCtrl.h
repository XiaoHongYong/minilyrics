// SEPropertyListCtrl.h: interface for the CSEPropertyListCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SEPROPERTYLISTCTRL_H__E86DCDE7_09BC_4684_A7B5_896177DD9BDA__INCLUDED_)
#define AFX_SEPROPERTYLISTCTRL_H__E86DCDE7_09BC_4684_A7B5_896177DD9BDA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SESkinWnd.h"

class CSESkinWnd;

class CSESkinPropertyAdapter
{
public:
	CSESkinPropertyAdapter()
	{
		m_hWndCommand = NULL;
		m_pwndSkin = NULL;
		m_pObj = NULL;
	}

	void UpdatePropertyToUI(LPCTSTR szName, LPCTSTR szValue)
	{
		SendMessage(m_hWndCommand, SNM_UPDATEPROPERTYITEM, (WPARAM)szName, (LPARAM)szValue);
	}

	void UpdatePropertyToSkin()
	{
		PostMessage(m_hWndCommand, WM_COMMAND, IDC_PROPERTY_SAVE, 0);
	}

	void UpdatePropertyToSkin(LPCTSTR szName, LPCTSTR szValue)
	{
		PostMessage(m_hWndCommand, WM_COMMAND, IDC_PROPERTY_SAVE, 0);
	}

	void TrackPopupMenu(HMENU hMenu, int x, int y)
	{
		::TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_LEFTALIGN, x, y, 0, m_hWndCommand, NULL);
	}

	bool IsSrcChanged(CSESkinWnd *pwndSkin, CUIObject *pObj)
	{
		return m_pwndSkin != pwndSkin || m_pObj != pObj;
	}

	void SetSrc(CSESkinWnd *pwndSkin, CUIObject *pObj, HWND hWndCmd)
	{
		m_pwndSkin = pwndSkin;
		m_pObj = pObj;
		m_hWndCommand = hWndCmd;
	}

	HWND			m_hWndCommand;
	CSESkinWnd		*m_pwndSkin;
	CUIObject		*m_pObj;

};

extern CSESkinPropertyAdapter		g_seSkinPropertyAdapter;

struct SEPLItem
{
	SEPLItem()
	{
		pBrush = NULL;
		nOptSelected = 0;
		nAlignment = DT_LEFT;
		nWidth = 0;
		bComboEditable = false;
		bComboSorted = false;
		LogFont.lfHeight = 12;
	}
	CUIObjProperty		prop;
	CBrush				*pBrush;
	int					nWidth;
	int					nAlignment;
	int					nOptSelected;
	BOOL				bComboEditable;
	BOOL				bComboSorted;
	LOGFONT				LogFont;

	bool IsEdit()
	{
		return prop.valueType == CUIObjProperty::VT_INT ||
			prop.valueType == CUIObjProperty::VT_STR ||
			prop.valueType == CUIObjProperty::VT_RECT ||
			prop.valueType == CUIObjProperty::VT_VAR_INT;
	}
};

/////////////////////////////////////////////////////////////////////////////
// CComboButton window
class CComboButton : public CButton
{
	void DrawTriangle(CDC* pDC, CRect Rect);

// Construction
public:
	BOOL	Create( CRect Rect, CWnd* pParent, UINT uID);
	CComboButton();

// Attributes
public:
	CPen*		m_pBkPen;
	CPen*		m_pGrayPen;
	CBrush*		m_pBkBrush;  
	CBrush*		m_pBlackBrush;

// Operations
public:

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct );
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CComboButton)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CComboButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(CComboButton)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

class CSEPropertyListCtrl : public CListBox  
{
	DECLARE_MESSAGE_MAP()

public:
	CSEPropertyListCtrl();
	virtual ~CSEPropertyListCtrl();

private:
	// Helper Functions
	void	DrawItem(CDC* pDC, CRect ItemRect, BOOL bSelected);
	void	DrawPropertyText(CDC* pDC, CRect ItemRect);
	void	CreateControl(CUIObjProperty::VALUE_TYPE valueType);
	void	HideControls();

	void OnPropertyChanged(int nIndex);

// Operations
public:
	// GUI Functions
	void			SetFont(CFont* pFont);
	void			SetBkColor(COLORREF crColor);
	void			SetPropertyBkColor(COLORREF crColor);
	void			SetHighlightColor(COLORREF crColor);
	void			SetLineStyle(COLORREF crColor, int nStyle = PS_SOLID);
	inline	void	SetBoldSelection(BOOL bBoldSelection)			{ m_bBoldSelection = bBoldSelection; };
	inline	void	SetTextColor(COLORREF crColor)					{ m_crTextColor = crColor; };
	inline	void	SetTextHighlightColor(COLORREF crColor)			{ m_crTextHighlightColor = crColor; };
	inline	void	SetPropertyTextColor(COLORREF crColor)			{ m_crPropertyTextColor = crColor; };

	// Add the data
	BOOL AddProperty(CUIObjProperty &prop, int nAlignment = DT_LEFT);
	BOOL AddProperties(CUIObjProperties &properties, int nAlignment = DT_LEFT);

	BOOL SetProperty(LPCTSTR szProp, LPCTSTR szValue);

	CUIObjProperty *GetProperty(int nIndex);
	void GetProperties(vector<tstring> &vProperties);

	void ResetContent();
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSEPropertyListCtrl)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	//}}AFX_VIRTUAL

protected:
	int GetTextExtent(LPCTSTR szText);

	void OnChooseIDClick();
	void OnImagePropertyClick();
	void OnFontPropertyClick();
	void OnPathPropertyClick();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSEPropertyListCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg void OnSelchange();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDblclk();
	afx_msg void OnEditLostFocus();
	afx_msg void OnEditChange();
	afx_msg void OnButtonClick();
	afx_msg void OnComboBoxClick();
	afx_msg void OnSelChange();
	afx_msg void OnListboxLostFocus();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG

protected:
	typedef vector<SEPLItem>	V_SEPLITEM;
	V_SEPLITEM				m_vProperies;
	SEPLItem				*m_pCurItem, *m_pCurDrawItem;

	int				m_nWidestItem;
	BOOL			m_bDeleteFont;
	BOOL			m_bBoldSelection;
	BOOL			m_bChanged;
	CPen*			m_pBorderPen;
	CRect			m_CurRect;
	CFont*			m_pTextFont;
	CFont*			m_pSelectedFont;
	CFont*			m_pCurFont;
	CString			m_csText;
	CBrush*			m_pCurBrush;
	CBrush*			m_pBkBrush;
	CBrush*			m_pBkHighlightBrush;
	CBrush*			m_pBkPropertyBrush;

	CButton			*m_pButton;
	CComboButton*	m_pComboButton;
	CListBox*		m_pListBox;

	// Controls
	CEdit*			m_pEditWnd;

	COLORREF		m_crBorderColor;
	COLORREF		m_crBkColor;
	COLORREF		m_crTextColor;
	COLORREF		m_crTextHighlightColor;
	COLORREF		m_crHighlightColor;
	COLORREF		m_crPropertyBkColor;
	COLORREF		m_crPropertyTextColor;

};

#endif // !defined(AFX_SEPROPERTYLISTCTRL_H__E86DCDE7_09BC_4684_A7B5_896177DD9BDA__INCLUDED_)
