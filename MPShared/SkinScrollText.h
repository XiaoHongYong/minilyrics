
#pragma once


class CSkinScrollText : public CSkinStaticText  
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinStaticText)
public:
	CSkinScrollText();
	virtual ~CSkinScrollText();

	void Draw(CRawGraph *pGraph);

// 	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
// 	void EnumProperties(CUIObjProperties &listProperties);

//	void OnCreate();

	bool OnLButtonUp(UINT nFlags, CPoint point);
	bool OnLButtonDown(UINT nFlags, CPoint point);

	void OnTimer(int nId);

protected:
	int			m_nTimerIDScroll;
	int			m_nPosScroll;
	bool		m_bToLeft;
	int			m_nWidthText;

};
