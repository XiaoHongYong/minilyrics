#pragma once

class CSkinActivateCtrl : public CSkinContainer
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
public:
	CSkinActivateCtrl();
	~CSkinActivateCtrl();

	void OnCreate();
	bool OnCustomCommand(int nId);
	void OnTimer(int nId);

	bool OnLButtonUp(UINT nFlags, CPoint point);
	bool OnLButtonDown(UINT nFlags, CPoint point);
	bool OnMouseMove(UINT nFlags, CPoint point);

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);

	void Draw(CRawGraph *pGraph);

	int GetOverlappedCtrl() const { return m_nIdOverlappedCtrl; }

protected:
	bool IsInReactArea(POINT pt);

	enum { BG_CLR_MAX = 6 };

	CUIObject			*m_pCloseBtn, *m_pTimerText;
	int					m_nIdTimerText;
	int					m_nCountDown, m_nDuration;
	CCursor				m_Cursor;

	int					m_nIdOverlappedCtrl;

	// This overlapped ctrl shall be hidden when Activate ctrl is showed up.
	CRawImage			m_imgBg;
	MLColor				m_vClrBg[BG_CLR_MAX];
	MLColor				m_clrTxt;
	int					m_nCountBgClrs;
	CSkinFontProperty	m_font;

};
