// BaseDialog.cpp: implementation of the CBaseDialog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BaseDialog.h"

HWND		g_hWndActiveModeless = NULL;

#define WM_MOUSEWHEEL                   0x020A

CBaseDialog::CBaseDialog(UINT nIDTemplate)
{
	m_nIDTemplate = nIDTemplate;

	m_bModal = FALSE;
	m_pTemplate = NULL;
}

CBaseDialog::~CBaseDialog()
{
	if (m_pTemplate)
		delete[] (char *)m_pTemplate;
}

LRESULT CBaseDialog::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return OnInitDialog();
		break;
	case ML_WM_LANGUAGE_CHANGED:
		OnLanguageChanged();
		break;
	case ML_WM_USER:
		OnUserMessage(wParam, lParam);
		break;
	case WM_ACTIVATE:
		{
			WORD		wActived;
			// BOOL		bMinimized;
			// HWND		hWndOld;

			wActived = LOWORD(wParam);

			if (!m_bModal)
			{
				if (wActived)
					g_hWndActiveModeless = m_hWnd;
				else
					g_hWndActiveModeless = NULL;
			}

			OnActivate(Tobool(wActived));
		}
		break;
	case WM_NOTIFY:
		return ConvertWMNotifyMsg(lParam);
	case WM_CREATE:
		if (OnCreate())
			return 0;
		else
			return -1;
// 	case WM_CLOSE:
// 		OnClose();
// 		return 0;
	case WM_COMMAND:
		{
			UINT uId    = LOWORD(wParam);
			UINT uEvent = HIWORD(wParam);
			OnCommand(uId, uEvent);
		}
		break;
	case WM_CONTEXTMENU:
		{
			//			int xPos = LOWORD(lParam);
			//			int yPos = HIWORD(lParam);
			OnContexMenu(LOWORD(lParam), HIWORD(lParam));
		}
		break;
	case WM_PAINT:
		PAINTSTRUCT		ps;
		HDC				hdc;
		hdc = BeginPaint(m_hWnd, &ps);
		IMLGraphicsBase	*pGraph;

		pGraph = CMLGraphicsFactory::CreateMLGraphics(hdc);
		assert(pGraph);

		OnPaint(pGraph);

		CMLGraphicsFactory::ReleaseMLGraphics(pGraph);

		EndPaint(m_hWnd, &ps);
		break;
	case WM_DESTROY:
		{
			HWND		hWnd = m_hWnd;
			OnDestroy();
			SetWindowLong(hWnd, GWL_USERDATA, (LONG)0);
			DefWindowProc(hWnd, message, wParam, lParam);
			// g_mapWnd.remove(m_hWnd);
		}
		return 0;
	case WM_DROPFILES:
		OnDropFiles((HDROP)wParam);
		break;
	case WM_HSCROLL:
		OnHScroll(LOWORD(wParam), HIWORD(wParam), NULL);
		break;
	case WM_KEYDOWN: // pass keyboard messages to main winamp window (for processing)
		OnKeyDown(wParam, lParam);
		break;
	case WM_KEYUP:
		OnKeyUp(wParam, lParam);
		break;
		return 0;
	case WM_LBUTTONDOWN:
		OnLButtonDown((UINT)wParam, CPoint((short)LOWORD(lParam), (short)HIWORD(lParam)));
		break;
	case WM_LBUTTONDBLCLK:
		OnLButtonDblClk((UINT)wParam, CPoint((short)LOWORD(lParam), (short)HIWORD(lParam)));
		break;
	case WM_LBUTTONUP:
		OnLButtonUp((UINT)wParam, CPoint((short)LOWORD(lParam), (short)HIWORD(lParam)));
		break;
	case WM_MOUSEMOVE:
		OnMouseMove((UINT)wParam, CPoint((short)LOWORD(lParam), (short)HIWORD(lParam)));
		break;;
	case WM_MOUSEWHEEL:
		{
			OnMouseWheel(short(HIWORD(wParam)), LOWORD(wParam), CPoint((short)LOWORD(lParam), (short)HIWORD(lParam)));
		}
		break;
	case WM_MOVE:
		OnMove(LOWORD(lParam), HIWORD(lParam));
		return 0;
	case WM_KILLFOCUS:
		OnKillFocus();
		break;
	case WM_SETFOCUS:
		OnSetFocus();
		break;
	case WM_SIZE:
		if (wParam == SIZE_RESTORED)
		{
			if (m_WndSizeMode != WndSizeMode_Normal)
			{
				m_WndSizeMode = WndSizeMode_Normal;
				OnSizeModeChanged(m_WndSizeMode);
			}
			OnSize(LOWORD(lParam), HIWORD(lParam));
		}
		else if (wParam == SIZE_MINIMIZED)
		{
			m_WndSizeMode = WndSizeMode_Minimized;
			OnSizeModeChanged(m_WndSizeMode);
		}
		else if (wParam == SIZE_MAXIMIZED)
		{
			m_WndSizeMode = WndSizeMode_Maximized;
			OnSizeModeChanged(m_WndSizeMode);
			OnSize(LOWORD(lParam), HIWORD(lParam));
		}
		break;
	case WM_VSCROLL:
		OnVScroll(LOWORD(wParam), HIWORD(wParam), NULL);
		break;
	case WM_TIMER:
		OnTimer((UINT)wParam);
	default:
//		if (!m_bModal)
//			return DefWindowProc(m_hWnd, message, wParam, lParam);
		break;
	}
	return 0;
}

int CBaseDialog::DoModal(CBaseWnd *pWndParent)
{
	m_bModal = TRUE;

	return (int)DialogBoxParam(GetAppInstance(), MAKEINTRESOURCE(m_nIDTemplate), pWndParent ? pWndParent->GetHandle() : NULL, (DLGPROC)BaseWndProc, (LPARAM)this);
}

BOOL CBaseDialog::Create(UINT nIDTemplate, CBaseWnd *pWndParent)
{
	m_bModal = FALSE;

	m_hWnd = CreateDialogParam(GetAppInstance(), MAKEINTRESOURCE(nIDTemplate), pWndParent ? pWndParent->GetHandle() : NULL, (DLGPROC)BaseWndProc, (LPARAM)this);

	return m_hWnd != NULL;
}

DLGTEMPLATE *CBaseDialog::NewTemplate(LPCTSTR szCaption, int x, int y, int nWidth, int nHeight, DWORD dwStyle, LPCTSTR szFont, int nFontSize)
{
	const int		nBuffLen = 1000;
	DLGTEMPLATE		*dlgTpl = (DLGTEMPLATE*)new char[nBuffLen];
	WORD			*p;
	byte			*pEnd = (byte*)dlgTpl + nBuffLen;

	memset(dlgTpl, 0, nBuffLen);

	if (szFont)
		dwStyle |= DS_SETFONT;
	else
		dwStyle &= ~DS_SETFONT;

	p = (WORD *)dlgTpl;
	*p++ = LOWORD (dwStyle);
	*p++ = HIWORD (dwStyle);
	*p++ = 0;		/* LOWORD (lExtendedStyle) */
	*p++ = 0;		/* HIWORD (lExtendedStyle) */
	*p++ = 0;		/* NumberOfItems */
	*p++ = x;		/* x  (dummy value) */
	*p++ = y;		/* y  (dummy value) */
	*p++ = nWidth;	/* cx (dummy value) */
	*p++ = nHeight;	/* cy (dummy value) */
	*p++ = 0;		/* Menu */
	*p++ = 0;		/* Class */

	/* copy the title of the dialog */
	_tcscpy_safe((TCHAR *)p, int(pEnd - (byte*)p), szCaption);
	p += _tcslen(szCaption) + 1;

	if (szFont)
	{
		/* Font information because of DS_SETFONT */
		*p++ = nFontSize;		/* point size */
		/* Face name */
		_tcscpy_safe((TCHAR *)p, int(pEnd - (byte*)p), szFont);
		p += _tcslen(szFont) + 1;
	}

	return dlgTpl;
}

BOOL CBaseDialog::DoModalIndirect(LPCTSTR szCaption, int x, int y, int nWidth, int nHeight, CBaseWnd *pWndParent, DWORD dwStyle, LPCTSTR szFont, int nFontSize)
{
	int				nRet;
	DLGTEMPLATE		*dlgTpl = NewTemplate(szCaption, x, y, nWidth, nHeight, dwStyle, szFont, nFontSize);
	if (!dlgTpl)
		return FALSE;

	nRet = (int)DialogBoxIndirectParam(GetAppInstance(), dlgTpl, pWndParent ? pWndParent->GetHandle() : NULL, (DLGPROC)BaseWndProc, (LPARAM)this);

	delete[] (char *)dlgTpl;

	return nRet;
}

BOOL CBaseDialog::CreateIndirect(LPCTSTR szCaption, int x, int y, int nWidth, int nHeight, CBaseWnd *pWndParent, DWORD dwStyle, LPCTSTR szFont, int nFontSize)
{
	if (m_pTemplate)
		delete[] (char *)m_pTemplate;
	m_pTemplate = NewTemplate(szCaption, x, y, nWidth, nHeight, dwStyle, szFont, nFontSize);
	if (!m_pTemplate)
		return FALSE;

	m_hWnd = CreateDialogIndirectParam(GetAppInstance(), m_pTemplate, pWndParent ? pWndParent->GetHandle() : NULL, (DLGPROC)BaseWndProc, (LPARAM)this);

	return m_hWnd != NULL;
}

void CBaseDialog::OnCommand(UINT uID, UINT nNotifyCode)
{
	if (uID == IDOK)
		OnOK();
	else if (uID == IDCANCEL)
		OnCancel();
}

void CBaseDialog::OnOK()
{
	if (IsModal())
		EndDialog(m_hWnd, IDOK);
	else
		Destroy();
}

void CBaseDialog::OnCancel()
{
	if (IsModal())
		EndDialog(m_hWnd, IDCANCEL);
	else
		Destroy();
}

BOOL CBaseDialog::IsModal()
{
	return m_bModal;
}

BOOL CBaseDialog::OnInitDialog()
{
	// 在开始的时候初始化多语言支持对象
	m_LangDlgHelper.InitAddAll(m_hWnd);

	// 切换为本地语言
	OnLanguageChanged();

	//
	// 居中此对话框
	if (!IsChildWnd(m_hWnd))
	{
		CenterWindowToMonitor(m_hWnd);
	}

	return TRUE;
}
