// BaseDialog.h: interface for the CBaseDialog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BASEDIALOG_H__97C9A49F_2742_4A43_95A3_E25F59A568FF__INCLUDED_)
#define AFX_BASEDIALOG_H__97C9A49F_2742_4A43_95A3_E25F59A568FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseWnd.h"

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

	virtual LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);
//
//	virtual void OnLButtonUp(UINT nFlags, CPoint point);
//	virtual void OnLButtonDown(UINT nFlags, CPoint point);
//	virtual void OnMouseMove(UINT nFlags, CPoint point);

public:
	// for win32 only
	virtual BOOL DoModalIndirect(LPCTSTR szCaption, int x, int y, int nWidth, int nHeight, CBaseWnd *pWndParent, DWORD dwStyle = WS_CAPTION | DS_SETFONT | DS_MODALFRAME | WS_SYSMENU | WS_VISIBLE, LPCTSTR szFont = _T("MS Sans Serif"), int nFontSize = 8);
	virtual BOOL CreateIndirect(LPCTSTR szCaption, int x, int y, int nWidth, int nHeight, CBaseWnd *pWndParent, DWORD dwStyle = WS_CAPTION | DS_SETFONT | DS_MODALFRAME | WS_SYSMENU | WS_VISIBLE, LPCTSTR szFont = _T("MS Sans Serif"), int nFontSize = 8);
	DLGTEMPLATE		*m_pTemplate;

	DLGTEMPLATE *NewTemplate(LPCTSTR szCaption, int x, int y, int nWidth, int nHeight, DWORD dwStyle, LPCTSTR szFont, int nFontSize);

public:
	CBaseDialog(UINT nIDTemplate);
	virtual ~CBaseDialog();

	BOOL		m_bModal;	// ÊÇ·ñÎªModal Dialog

	UINT		m_nIDTemplate;

};

#endif // !defined(AFX_BASEDIALOG_H__97C9A49F_2742_4A43_95A3_E25F59A568FF__INCLUDED_)
