
#pragma once

class CDlgChooseFont
{
public:
	CDlgChooseFont();
	virtual ~CDlgChooseFont();

	int DoModal(CBaseWnd *pWndParent, LPCTSTR szFontFaceName, int nFontSize, int nWeight, int nItalic);

	LPCTSTR GetFaceName();

	int GetSize();

	int GetWeight();

	int GetItalic();

public:
	tstring			m_strFontFaceName;
	int				m_nFontSize, m_nWeight;
	int				m_nItalic;

};
