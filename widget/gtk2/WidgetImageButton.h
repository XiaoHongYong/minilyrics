// WidgetImageButton.h: interface for the CWidgetImageButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WIDGETIMAGEBUTTON_H__B4F4B929_C2E2_4DE4_AE0C_BD0FDB2495ED__INCLUDED_)
#define AFX_WIDGETIMAGEBUTTON_H__B4F4B929_C2E2_4DE4_AE0C_BD0FDB2495ED__INCLUDED_

#pragma once

class CWidgetImageButton : public CWidgetCtrlBase
{
public:
	CWidgetImageButton();
	virtual ~CWidgetImageButton();

	BOOL Attach(CBaseWnd *pWndParent, int nCtrlID);

	void SetImage(CMLImage *pImage);

protected:
	CMLImage		* m_pImage;

};

#endif // !defined(AFX_WIDGETIMAGEBUTTON_H__B4F4B929_C2E2_4DE4_AE0C_BD0FDB2495ED__INCLUDED_)
