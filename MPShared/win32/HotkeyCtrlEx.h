// HotkeyCtrlEx.h: interface for the CHotkeyCtrlEx class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HOTKEYCTRLEX_H__C50B3FA5_4505_4569_9A4E_A1C5CDC6C311__INCLUDED_)
#define AFX_HOTKEYCTRLEX_H__C50B3FA5_4505_4569_9A4E_A1C5CDC6C311__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

void FormatHotkeyText(tstring &strText, UINT nVirtKey, UINT fsModifiers);

bool IsKeyPressed(int nKey);

class CHotkeyCtrlEx : public CBaseWnd
{
public:
	CHotkeyCtrlEx();
	virtual ~CHotkeyCtrlEx();

	BOOL Init(CBaseWnd *pParentWnd, UINT uChildId);

	virtual void OnKeyDown(UINT nChar, UINT nFlags);
	virtual void OnKeyUp(UINT nChar, UINT nFlags);

	void GetHotkey(UINT &nVirKey, UINT &nfsModifiers);
	void SetHotkey(UINT nVirKey, UINT nfsModifiers);

protected:
	LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);

	void UpdateHotKeyText();

	WNDPROC			m_pWndProcOrg;

	bool			m_bShiftDown, m_bAltDown, m_bCtrlDown, m_bWinDown;

	bool			m_bCharDown;
	UINT			m_nChar;

	UINT			m_nHotVk, m_nfsModifiers;

};

#endif // !defined(AFX_HOTKEYCTRLEX_H__C50B3FA5_4505_4569_9A4E_A1C5CDC6C311__INCLUDED_)
