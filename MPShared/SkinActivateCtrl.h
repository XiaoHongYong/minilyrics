#pragma once

class CSkinActivateCtrl : public CUIObject
{
	UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
	CSkinActivateCtrl();
	~CSkinActivateCtrl();

	void OnCreate();

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);

	void Draw(CRawGraph *pGraph);

protected:
	enum { BG_CLR_MAX = 6 };

	// This overlapped ctrl shall be hidden when Activate ctrl is showed up.
	CRawImage			m_imgBg;
	MLColor				m_vClrBg[BG_CLR_MAX];
	MLColor				m_clrTxt;
	int					m_nCountBgClrs;
	CSkinFontProperty	m_font;

};
