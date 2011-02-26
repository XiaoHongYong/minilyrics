// MPSkinWnd.h: interface for the CMPSkinWnd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPSKINWND_H__C466015C_6B75_4118_98E2_AEDA64F864DA__INCLUDED_)
#define AFX_MPSKINWND_H__C466015C_6B75_4118_98E2_AEDA64F864DA__INCLUDED_

#pragma once

#include "LyrPlaylistAutoSwitch.h"

class ISkinCmdHandler;


class IEventHandlerEx : public IEventHandler
{
public:
	virtual void Unregister() = 0;

};

typedef map<CUIObject *, IEventHandlerEx *>		MAP_EVENT_HANDLER;


class CMPSkinWnd : public CSkinWnd, public IEventHandler
{
public:
	CMPSkinWnd();
	virtual ~CMPSkinWnd();

	void OnActivate(bool bActived);

	virtual void OnPreCreate(DWORD &dwStyle, DWORD &dwExStyle);

	virtual bool OnCreate();

	// IEventHandler
	virtual void OnEvent(const IEvent *pEvent);

	void OnCommand(UINT uID, UINT nNotifyCode);

	bool OnCustomCommand(int nId);

	void OnUIObjNotify(IUIObjNotify *pNotify);

	void OnKeyDown(UINT nChar, UINT nFlags);

	void OnSizeModeChanged(WndSizeMode sizeMode);

	void OnVScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar);

	virtual void OnSkinLoaded();

	virtual void OnAddUIObj(CUIObject *pObj);

	virtual void OnRemoveUIObj(CUIObject *pObj);

	virtual BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);

	virtual void CloseSkin();

	virtual BOOL SetProperty(SXNode *pXmlNode);
#ifdef _MPLAYER
	virtual void OnLButtonUp(UINT nFlags, CPoint point) { m_autoSwitchToLyrShow.OnUserAction(); CSkinWnd::OnLButtonUp(nFlags, point); }
	// virtual void OnLButtonDown(UINT nFlags, CPoint point) { m_autoSwitchToLyrShow.OnUserAction(); CSkinWnd::OnLButtonDown(nFlags, point); }
	virtual void OnLButtonDblClk(UINT nFlags, CPoint point) { m_autoSwitchToLyrShow.OnUserAction(); CSkinWnd::OnLButtonDblClk(nFlags, point); }
	virtual void OnRButtonUp(UINT nFlags, CPoint point) { m_autoSwitchToLyrShow.OnUserAction(); CSkinWnd::OnRButtonUp(nFlags, point); }
	virtual void OnRButtonDown(UINT nFlags, CPoint point) { m_autoSwitchToLyrShow.OnUserAction(); CSkinWnd::OnRButtonDown(nFlags, point); }
	virtual void OnMouseMove(UINT nFlags, CPoint point) { m_autoSwitchToLyrShow.OnUserAction(); CSkinWnd::OnMouseMove(nFlags, point); }
	virtual void OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt) { m_autoSwitchToLyrShow.OnUserAction(); CSkinWnd::OnMouseWheel(nWheelDistance, nMkeys, pt); }
#endif // _MPLAYER

	virtual void OnTimer (UINT nIDEvent) { if (m_autoSwitchToLyrShow.OnTimer(nIDEvent)) return; CSkinWnd::OnTimer(nIDEvent); }


public:
	// IUICheckStatus interface
	virtual bool GetChecked(UINT nID, bool &bChecked);
	virtual bool GetRadioChecked(vector<UINT> &vIDs, UINT &nIDChecked);

#ifdef _WIN32_DESKTOP
protected:
	virtual void OnDropFiles(HDROP hDrop);
#endif

	void AddCmdHandler(LPCTSTR szName);

	void SetTopmost(bool bTopmost);
	CBaseWnd *GetParentOrSelf();

	//
	// Special settings for different kind of winodw (Floating lyrics and Normal window).
	//
	virtual bool SettingGetTopmost();
	virtual int SettingGetOpaquePercent();
	virtual bool SettingGetClickThrough();

	virtual void SettingReverseTopmost();
	virtual void SettingSetOpaquePercent(int nPercent);
	virtual void SettingReverseClickThrough();

protected:
	typedef list<ISkinCmdHandler *>	LIST_SKINCMDHANDLER;

	tstring					m_strCmdHandler;
	LIST_SKINCMDHANDLER		m_listSkinCmdHandler;

	MAP_EVENT_HANDLER		m_mapEventHandlers;

	//
	// Window status and properties
	//

	CLyrPlaylistAutoSwitch	m_autoSwitchToLyrShow;

};

inline byte OpaquePercentToAlpha(int nOpaquePercent)
{
	if (nOpaquePercent < 10)
		nOpaquePercent = 10;
	else if (nOpaquePercent > 100)
		nOpaquePercent = 100;
	return (byte)(nOpaquePercent * 255 / 100);
}

#endif // !defined(AFX_MPSKINWND_H__C466015C_6B75_4118_98E2_AEDA64F864DA__INCLUDED_)
