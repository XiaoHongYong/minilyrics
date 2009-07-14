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
// 	TCHAR	szKeyName[256];
// 	UINT	nScanCode;
// 
// 	strText.resize(0);
// 
// 	EmptyStr(szKeyName);
// 	nScanCode = MapVirtualKey(nVirtKey, 0) << 16;
// 	if (IsExtendedKey(nVirtKey))
// 		nScanCode |= 1 << 24;
// 	GetKeyNameText(nScanCode, szKeyName, CountOf(szKeyName));
// 
// 	if (IsFlagSet(fsModifiers, MOD_WIN))
// 		strText += _T("Winkey + ");
// 	if (IsFlagSet(fsModifiers, MOD_CONTROL))
// 		strText += _T("Ctrl + ");
// 	if (IsFlagSet(fsModifiers, MOD_SHIFT))
// 		strText += _T("Shift + ");
// 	if (IsFlagSet(fsModifiers, MOD_ALT))
// 		strText += _T("Alt + ");
// 	strText += szKeyName;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHotkeyCtrlEx::CHotkeyCtrlEx()
{
}

CHotkeyCtrlEx::~CHotkeyCtrlEx()
{

}

BOOL CHotkeyCtrlEx::Init(CBaseWnd *pParentWnd, UINT uChildId)
{
	m_bShiftDown = false;
	m_bAltDown = false;
	m_bCtrlDown = false;
	m_bWinDown = false;
	m_bCharDown = false;

	m_nChar = 0;

	m_nHotVk = 0;
	m_nfsModifiers = 0;

	return TRUE;
}

void CHotkeyCtrlEx::OnKeyDown(UINT nChar, UINT nFlags)
{

	UpdateHotKeyText();
}

void CHotkeyCtrlEx::OnKeyUp(UINT nChar, UINT nFlags)
{

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

void CHotkeyCtrlEx::UpdateHotKeyText()
{
// 	if (m_bCharDown)
// 		m_nHotVk = m_nChar;
// 	else
// 		m_nHotVk = 0;
// 
// 	m_nfsModifiers = 0;
// 	if (m_bWinDown)
// 		m_nfsModifiers |= MOD_WIN;
// 	if (m_bShiftDown)
// 		m_nfsModifiers |= MOD_SHIFT;
// 	if (m_bCtrlDown)
// 		m_nfsModifiers |= MOD_CONTROL;
// 	if (m_bAltDown)
// 		m_nfsModifiers |= MOD_ALT;
// 
// 	tstring	strText;
// 
// 	FormatHotkeyText(strText, m_nHotVk, m_nfsModifiers);
// 
// 	SetWindowText(strText.c_str());
}
