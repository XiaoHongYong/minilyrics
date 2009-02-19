// MPSkinWnd.h: interface for the CMPSkinWnd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPSKINWND_H__C466015C_6B75_4118_98E2_AEDA64F864DA__INCLUDED_)
#define AFX_MPSKINWND_H__C466015C_6B75_4118_98E2_AEDA64F864DA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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

	virtual bool OnCreate();
	virtual void OnDestroy();

	// IEventHandler
	virtual void OnEvent(const IEvent *pEvent);

	virtual void OnContexMenu(int xPos, int yPos);

	void OnCommand(UINT uID, UINT nNotifyCode);

	void OnCustomCommand(int nId);

	void OnUIObjNotify(IUIObjNotify *pNotify);

	void OnKeyDown(UINT nChar, UINT nFlags);

	void OnSizeModeChanged(WndSizeMode sizeMode);

	void OnVScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar);

	virtual void OnSkinLoaded();
	virtual void OnLoadWndSizePos();

	virtual void OnAddUIObj(CUIObject *pObj);

	virtual void OnRemoveUIObj(CUIObject *pObj);

	virtual BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);

	virtual void CloseSkin();

public:
	// IUICheckStatus interface
	virtual bool GetChecked(UINT nID, bool &bChecked);
	virtual bool GetRadioChecked(vector<UINT> &vIDs, UINT &nIDChecked);

#ifdef _WIN32_DESKTOP
protected:
	virtual void OnDropFiles(HDROP hDrop);

public:
	//
	// Window transparent properties for windows
	//
	void LoadTransparentSettings();

	void SetClickThrough(bool bClickThrough) { m_bClickThrough = bClickThrough; }
	bool GetClickThrough() const { return m_bClickThrough; }
#endif

	void AddCmdHandler(LPCTSTR szName);
	void UpdateVolumeCtrl();

	void SaveWndPos();

	void SetTopmost(bool bTopmost);
	CBaseWnd *GetParentOrSelf();

	LPCTSTR GetCfgSectName();

	LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);

protected:
	typedef list<ISkinCmdHandler *>	LIST_SKINCMDHANDLER;

	tstring					m_strCmdHandler;
	LIST_SKINCMDHANDLER		m_listSkinCmdHandler;

	MAP_EVENT_HANDLER		m_mapEventHandlers;

	// Use volume bar or volume button(inc,dec)
	bool					m_bEnableVolumeSwitch;
	bool					m_bUseVolumeSeek;

	//
	// Window status and properties
	//

	bool					m_bTopmost;	

	tstring					m_strLyrDisplayStylePropName;

};

#endif // !defined(AFX_MPSKINWND_H__C466015C_6B75_4118_98E2_AEDA64F864DA__INCLUDED_)
