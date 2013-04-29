// MPSkinWnd.h: interface for the CMPSkinWnd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPSKINWND_H__C466015C_6B75_4118_98E2_AEDA64F864DA__INCLUDED_)
#define AFX_MPSKINWND_H__C466015C_6B75_4118_98E2_AEDA64F864DA__INCLUDED_

#pragma once

class ISkinCmdHandler;


typedef map<CUIObject *, IEventHandler *>		MAP_EVENT_HANDLER;


class CMPSkinWnd : public CSkinWnd, public IEventHandler
{
public:
	CMPSkinWnd();
	virtual ~CMPSkinWnd();

	virtual void OnPreCreate(bool &bTopmost, bool &bVisible);

	virtual bool OnCreate();
	virtual void OnEvent(const IEvent *pEvent) { };

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


public:
	// IUICheckStatus interface
	virtual bool GetChecked(UINT nID, bool &bChecked);
	virtual bool GetRadioChecked(vector<UINT> &vIDs, UINT &nIDChecked);

	virtual void PostCustomCommandMsg(int nId);
	virtual void PostShortcutKeyCmd(int nId);

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
