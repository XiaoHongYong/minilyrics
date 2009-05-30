// HotkeyCtrlEx.cpp: implementation of the CHotkeyCtrlEx class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HotkeyCtrlEx.h"

bool IsExtendedKey(UINT nVk)
{
	return nVk >= VK_SPACE && nVk <= VK_HELP;
}

void FormatHotkeyText(tstring &strText, UINT nVirtKey, UINT fsModifiers)
{
	TCHAR	szKeyName[256];
	UINT	nScanCode;

	strText.resize(0);

	EmptyStr(szKeyName);
	nScanCode = MapVirtualKey(nVirtKey, 0) << 16;
	if (IsExtendedKey(nVirtKey))
		nScanCode |= 1 << 24;
	GetKeyNameText(nScanCode, szKeyName, CountOf(szKeyName));

	if (IsFlagSet(fsModifiers, MOD_WIN))
		strText += _T("Winkey + ");
	if (IsFlagSet(fsModifiers, MOD_CONTROL))
		strText += _T("Ctrl + ");
	if (IsFlagSet(fsModifiers, MOD_SHIFT))
		strText += _T("Shift + ");
	if (IsFlagSet(fsModifiers, MOD_ALT))
		strText += _T("Alt + ");
	strText += szKeyName;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHotkeyCtrlEx::CHotkeyCtrlEx()
{
	m_pWndProcOrg = NULL;
	m_bShiftDown = m_bAltDown = m_bCtrlDown = m_bWinDown = false;
	m_bCharDown = false;
	m_nChar = 0;
	m_nHotVk = 0;
	m_nfsModifiers = 0;
}

CHotkeyCtrlEx::~CHotkeyCtrlEx()
{

}

BOOL CHotkeyCtrlEx::Init(CBaseWnd *pParentWnd, UINT uChildId)
{
	m_hWnd = ::GetDlgItem(pParentWnd->GetHandle(), uChildId);
	if (!m_hWnd)
		return FALSE;

	m_bShiftDown = false;
	m_bAltDown = false;
	m_bCtrlDown = false;
	m_bWinDown = false;
	m_bCharDown = false;

	m_nChar = 0;

	m_nHotVk = 0;
	m_nfsModifiers = 0;

	// stash pointer to self
	SetWindowLong(m_hWnd, GWL_USERDATA, (LONG)this);

	m_pWndProcOrg = (WNDPROC)GetWindowLong(m_hWnd, GWL_WNDPROC);
	SetWindowLong(m_hWnd, GWL_WNDPROC, (LONG)BaseWndProc);

	return TRUE;
}

void CHotkeyCtrlEx::OnKeyDown(UINT nChar, UINT nFlags)
{
/*	if (nChar == VK_CONTROL)
		m_bCtrlDown = true;
	else if (nChar == VK_SHIFT)
		m_bShiftDown = true;
	else if (nChar == VK_MENU)
		m_bAltDown = true;
	else if (nChar == VK_LWIN || nChar == VK_RWIN)
		m_bWinDown = true;
	else
	{
		m_bCharDown = true;
		m_nChar = nChar;
	}*/
	m_bWinDown = m_bCtrlDown = m_bAltDown = m_bShiftDown = false;

	if (IsKeyPressed(VK_CONTROL))
		m_bCtrlDown = true;
	if (IsKeyPressed(VK_SHIFT))
		m_bShiftDown = true;
	if (IsKeyPressed(VK_MENU))
		m_bAltDown = true;
	if (IsKeyPressed(VK_LWIN) || IsKeyPressed(VK_RWIN))
		m_bWinDown = true;

	if (nChar != VK_CONTROL && nChar != VK_MENU && nChar != VK_SHIFT 
		&& nChar != VK_LWIN && nChar != VK_RWIN)
	{
		m_bCharDown = true;
		m_nChar = nChar;
	}

	UpdateHotKeyText();
}

void CHotkeyCtrlEx::OnKeyUp(UINT nChar, UINT nFlags)
{
/*	if (nChar == VK_CONTROL)
		m_bCtrlDown = false;
	else if (nChar == VK_SHIFT)
		m_bShiftDown = false;
	else if (nChar == VK_MENU)
		m_bAltDown = false;
	else if (nChar == VK_APPS)
		m_bWinDown = false;
	else
	{
		m_bCharDown = false;
		m_nChar = 0;
		return;
	}*/
	m_bWinDown = m_bCtrlDown = m_bAltDown = m_bShiftDown = false;

	if (GetKeyState(VK_CONTROL) & 0x8000)
		m_bCtrlDown = true;
	if (GetKeyState(VK_SHIFT) & 0x8000)
		m_bShiftDown = true;
	if (GetKeyState(VK_MENU) & 0x8000)
		m_bAltDown = true;
	if ((GetKeyState(VK_LWIN) & 0x8000) || (GetKeyState(VK_RWIN) & 0x8000))
		m_bWinDown = true;

	if (nChar != VK_CONTROL && nChar != VK_MENU && nChar != VK_SHIFT 
		&& nChar != VK_LWIN && nChar != VK_RWIN)
	{
		m_bCharDown = false;
		m_nChar = nChar;
	}

	if (m_bCharDown)
		UpdateHotKeyText();
}

void CHotkeyCtrlEx::GetHotkey(UINT &nVirKey, UINT &nfsModifiers)
{
	nVirKey = m_nHotVk;
	nfsModifiers = m_nfsModifiers;
}

void CHotkeyCtrlEx::SetHotkey(UINT nVirKey, UINT nfsModifiers)
{
	m_nHotVk = nVirKey;
	m_nfsModifiers = nfsModifiers;

	tstring	strText;

	FormatHotkeyText(strText, m_nHotVk, m_nfsModifiers);

	SetWindowText(strText.c_str());
}

LRESULT CHotkeyCtrlEx::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
// 	if (message == WM_KEYDOWN)
// 	{
// 		TCHAR	szText[128];
// 		GetKeyNameText(lParam, szText, CountOf(szText));
// 		DBGLOG1(_T("Keydown: %s"), szText);
// 		UINT	a, b, c, d, e;
// 		HKL	kb = LoadKeyboardLayout(CStrPrintf(_T("%08X"), GetUserDefaultLangID()).c_str(), KLF_ACTIVATE);
// 		a = MapVirtualKeyEx(wParam, 0, kb);
// 		b = MapVirtualKeyEx(wParam, 1, kb);
// 		c = MapVirtualKeyEx(wParam, 2, kb);
// 		d = MapVirtualKeyEx(a, 1, kb);
// 		e = MapVirtualKeyEx(b, 0, kb);
// 		UnloadKeyboardLayout(kb);
// 	}
// 	else if (message == WM_KEYUP)
// 	{
// 		TCHAR	szText[128];
// 		GetKeyNameText(lParam, szText, CountOf(szText));
// 		DBGLOG1(_T("Keyup: %s"), szText);
// 	}

	if (message == WM_KEYDOWN || message == WM_KEYUP || message == WM_CHAR
		|| message == WM_RBUTTONUP || message == WM_CONTEXTMENU)
		return CBaseWnd::WndProc(message, wParam, lParam);
	else if (message == WM_SYSKEYDOWN)
	{
		OnKeyDown(wParam, lParam);
		return 0;
	}
	else if (message == WM_SYSKEYUP)
	{
		OnKeyUp(wParam, lParam);
		return 0;
	}
	else if (message == WM_DESTROY)
	{
		SetWindowLong(m_hWnd, GWL_WNDPROC, (LONG)m_pWndProcOrg);
	}
	else if (message == WM_GETDLGCODE)
		return DLGC_WANTALLKEYS;

	return CallWindowProc(m_pWndProcOrg,m_hWnd, message, wParam, lParam);
}

void CHotkeyCtrlEx::UpdateHotKeyText()
{
	if (m_bCharDown)
		m_nHotVk = m_nChar;
	else
		m_nHotVk = 0;

	m_nfsModifiers = 0;
	if (m_bWinDown)
		m_nfsModifiers |= MOD_WIN;
	if (m_bShiftDown)
		m_nfsModifiers |= MOD_SHIFT;
	if (m_bCtrlDown)
		m_nfsModifiers |= MOD_CONTROL;
	if (m_bAltDown)
		m_nfsModifiers |= MOD_ALT;

	tstring	strText;

	FormatHotkeyText(strText, m_nHotVk, m_nfsModifiers);

	SetWindowText(strText.c_str());
}
