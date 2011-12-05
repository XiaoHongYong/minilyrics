#pragma once

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

};
