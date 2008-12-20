// EditCtrl.cpp: implementation of the CWidgetEditBox class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WidgetEditBox.h"

#ifdef _WINCE_
#else
#include <Richedit.h>
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWidgetEditBox::CWidgetEditBox()
{
	m_hWnd = NULL;
}

CWidgetEditBox::~CWidgetEditBox()
{

}

BOOL CWidgetEditBox::Create(LPCTSTR szCaption, int x, int y, int nWidth, int nHeight, CBaseWnd *pWndParent)
{
	DWORD	m_dwStyle = WS_CHILD | WS_VISIBLE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | WS_VSCROLL | WS_HSCROLL| ES_MULTILINE | ES_WANTRETURN;

//	if (GetOperationSystemType() < OPS_WIN2000)
//		m_hWnd = CreateWindow(_T("RICHEDIT"), szCaption, m_dwStyle, x, y, nWidth, nHeight, pWndParent ? pWndParent->GetHandle() : NULL, NULL, GetAppInstance(), NULL);
//		m_hWnd = CreateWindow(_T("RichEdit20W"), szCaption, m_dwStyle, x, y, nWidth, nHeight, pWndParent ? pWndParent->GetHandle() : NULL, NULL, GetAppInstance(), NULL);
//	else
		m_hWnd = CreateWindow(_T("EDIT"), szCaption, m_dwStyle, x, y, nWidth, nHeight, pWndParent ? pWndParent->GetHandle() : NULL, NULL, GetAppInstance(), NULL);

	ASSERT(m_hWnd);
	::SendMessage(m_hWnd, EM_LIMITTEXT, 1 * 1024 * 1024, 0);

	return m_hWnd != NULL;
}

bool CWidgetEditBox::SetWindowText(LPCTSTR szText)
{
	SendMessage(m_hWnd, WM_SETTEXT, 0, (LPARAM)szText);
	SendMessage(m_hWnd, EM_SETSEL, GetWindowTextLength(m_hWnd) - 2, GetWindowTextLength(m_hWnd));

	return true;
}

void CWidgetEditBox::SetBkColor(const MLColor &clr)
{
#ifdef _WINCE_
#else
	SendMessage(m_hWnd, EM_SETBKGNDCOLOR, (WPARAM)0, clr.get());
#endif
}

void CWidgetEditBox::SetMaxLength(int nMaxLen)
{
	SendMessage(m_hWnd, EM_LIMITTEXT, nMaxLen, 0);
}

void CWidgetEditBox::SetFont(const CMLFont &font, const MLColor &clrText)
{
// 	return;
// 	if (GetOperationSystemType() < OPS_WIN2000)
// 	{
// 		CHARFORMATA		cf;
// 
// 		memset(&cf, 0, sizeof(cf));
// 		cf.cbSize = sizeof(cf);
// 		cf.dwMask = CFM_BOLD | CFM_CHARSET | CFM_COLOR | CFM_FACE | CFM_ITALIC | CFM_SIZE;
// 		// cf.dwMask = CFM_FACE;
// 		// cf.yHeight = MulDiv(abs(font.GetHeight()), 72 * 20, GetDeviceCaps(NULL, LOGPIXELSY));
// 		cf.yHeight = font.GetHeight();
// 		cf.bCharSet = font.GetCharset();
// 		cf.crTextColor = clrText.get();
// 		if (font.GetWeight() >= FW_BOLD)
// 			cf.dwEffects |= CFE_BOLD;
// 		if (font.IsItalic())
// 			cf.dwEffects |= CFE_ITALIC;
// 		ConvertStr2(font.GetName(), -1, cf.szFaceName, CountOf(cf.szFaceName));
// 		// _tcscpy_safe(cf.szFaceName, CountOf(cf.szFaceName), font.GetName());
// 
// 		SetLastError(0);
// 		DBGLOG1(_T("EM_SETCHARFORMAT before: %s"), cf.szFaceName);
// 		if (SendMessage(m_hWnd, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf) == 0)
// 			DBGLOG0(_T("EM_SETCHARFORMAT FAILED!"));
// 	}
//  	else
 	{
		m_gdifont.Create(font);

 		SendMessage(m_hWnd, WM_SETFONT, (WPARAM)m_gdifont.GetHandle(), FALSE);
 	}
}

void CWidgetEditBox::AppendText(LPCTSTR szText)
{
	int		n = GetLength();
	SetSel(n, n);
	ReplaceSel(szText);
}
