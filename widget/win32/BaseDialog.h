#pragma once

#include "Widget.h"
#include "WidgetLocalization.h"

extern HWND		g_hWndActiveModeless;

class CBaseDialog : public CBaseWnd  
{
public:
	virtual BOOL Create(UINT nIDTemplate, CBaseWnd *pWndParent);

	int DoModal(CBaseWnd *pWndParent);
	BOOL IsModal();

	virtual void OnCancel();
	virtual void OnOK();

	void OnCommand(UINT uID, UINT nNotifyCode);
	virtual BOOL OnInitDialog();
	void OnLanguageChanged();

	virtual BOOL OnNotify(WIDGET_NOTIFY_INFO *info) { return true; }

	virtual LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);
//
//	virtual void OnLButtonUp(UINT nFlags, CPoint point);
//	virtual void OnLButtonDown(UINT nFlags, CPoint point);
//	virtual void OnMouseMove(CPoint point);

public:
	// for win32 only
	virtual BOOL DoModalIndirect(LPCTSTR szCaption, int x, int y, int nWidth, int nHeight, CBaseWnd *pWndParent, DWORD dwStyle = WS_CAPTION | DS_SETFONT | DS_MODALFRAME | WS_SYSMENU | WS_VISIBLE, LPCTSTR szFont = _T("MS Sans Serif"), int nFontSize = 8);
	virtual BOOL CreateIndirect(LPCTSTR szCaption, int x, int y, int nWidth, int nHeight, CBaseWnd *pWndParent, DWORD dwStyle = WS_CAPTION | DS_SETFONT | DS_MODALFRAME | WS_SYSMENU | WS_VISIBLE, LPCTSTR szFont = _T("MS Sans Serif"), int nFontSize = 8);
	DLGTEMPLATE		*m_pTemplate;

	DLGTEMPLATE *NewTemplate(LPCTSTR szCaption, int x, int y, int nWidth, int nHeight, DWORD dwStyle, LPCTSTR szFont, int nFontSize);

	int ConvertWMNotifyMsg(LPARAM lParam);

public:
	CBaseDialog(UINT nIDTemplate);
	virtual ~CBaseDialog();

	BOOL		m_bModal;	// 是否为Modal Dialog

	UINT		m_nIDTemplate;

	CLangDialogHelper	m_LangDlgHelper;

};

