
#pragma once

#include "WidgetCtrlBase.h"

class CWidgetEditBox : public CWidgetCtrlBase
{
public:
	CWidgetEditBox();
	virtual ~CWidgetEditBox();

public:
	virtual BOOL Create(LPCTSTR szCaption, int x, int y, int nWidth, int nHeight, CBaseWnd *pWndParent);

	bool SetWindowText(LPCTSTR szText);

	bool IsEditable();

	void SetEditable(bool bEditable);

	void SetPasswordMode(bool bInputPassword);

	void SetSel(int nStart, int nEnd) { SendMessage(m_hWnd, EM_SETSEL, (WPARAM)nStart, (LPARAM)nEnd); }
	void ReplaceSel(LPCTSTR szText) { SendMessage(m_hWnd, EM_REPLACESEL, true, (LPARAM)szText); }
	void SetBkColor(const MLColor &clr);

	void SetMaxLength(int nMaxLen);

	void InsertText(LPCTSTR szText, int nPos);
	void DeleteText(int nStart, int nEnd);
	void DeleteSel();

	int GetLength() { return SendMessage(m_hWnd, WM_GETTEXTLENGTH, 0, 0); }

	void AppendText(LPCTSTR szText);

	void SetFont(const CFontInfo &font, const MLColor &clrText);

public:
//	HWND		m_hWnd;
	//CGdiFont		m_gdifont;

protected:

};
