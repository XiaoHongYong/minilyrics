// WidgetImageButton.h: interface for the CWidgetImageButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WIDGETIMAGEBUTTON_H__B4F4B929_C2E2_4DE4_AE0C_BD0FDB2495ED__INCLUDED_)
#define AFX_WIDGETIMAGEBUTTON_H__B4F4B929_C2E2_4DE4_AE0C_BD0FDB2495ED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CWidgetImageButton : public CWidgetCtrlBase
{
public:
	CWidgetImageButton();
	virtual ~CWidgetImageButton();

	BOOL Attach(CBaseWnd *pWndParent, int nCtrlID);

	void SetImage(CMLImage *pImage);

public:
	void OnPaint(IMLGraphicsBase *pGraph);

	void OnLButtonUp(UINT nFlags, CPoint point);

protected:
	LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);

	WNDPROC			m_pWndProcOrg;
	CMLImage		*m_pImage;
	HBITMAP			m_hBmp;
	int				m_nBmpCx, m_nBmpCy;

};

#endif // !defined(AFX_WIDGETIMAGEBUTTON_H__B4F4B929_C2E2_4DE4_AE0C_BD0FDB2495ED__INCLUDED_)
