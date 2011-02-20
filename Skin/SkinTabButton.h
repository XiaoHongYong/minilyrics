#pragma once

class CSkinTabButton : public CSkinToolbar
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinToolbar)
public:
	CSkinTabButton();
	virtual ~CSkinTabButton();

	void Draw(CRawGraph *pGraph);

	virtual void OnCreate();

	virtual bool OnMenuKey(UINT nChar, UINT nFlags);

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);

protected:
	virtual void DrawButton(CRawGraph *pGraph, int nButton, BT_DRAW_STATE btDrawState, int x, int y, int nHeight, int nBtImageLeft);

protected:
	int					m_nSperatorLineWidth;
	int					m_nButtonFaceWidth, m_nButtonBorderWidth;

};
