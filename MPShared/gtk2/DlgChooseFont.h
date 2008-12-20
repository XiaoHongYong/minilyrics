// DlgChooseFont.h: interface for the CDlgChooseFont class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DLGCHOOSEFONT_H__53DFBF2E_6C3A_4BC6_B059_FEB4814E504A__INCLUDED_)
#define AFX_DLGCHOOSEFONT_H__53DFBF2E_6C3A_4BC6_B059_FEB4814E504A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDlgChooseFont
{
public:
	CDlgChooseFont();
	virtual ~CDlgChooseFont();

	int DoModal(CBaseWnd *pWndParent, LPCTSTR szFontFaceName, int nFontHeight, int nWeight, int nItalic);

	LPCTSTR GetFaceName();

	int GetHeight();

	int GetWeight();

	int GetItalic();

public:
	tstring			m_strFontFaceName;
	int				m_nFontHeight, m_nWeight;
	int				m_nItalic;

	GtkWidget		*m_window;

};

#endif // !defined(AFX_DLGCHOOSEFONT_H__53DFBF2E_6C3A_4BC6_B059_FEB4814E504A__INCLUDED_)
