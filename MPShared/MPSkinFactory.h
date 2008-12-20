// MPSkinFactory.h: interface for the CMPSkinFactory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPSKINFACTORY_H__0C65D114_F3BF_47A3_A647_37E9AB35B4ED__INCLUDED_)
#define AFX_MPSKINFACTORY_H__0C65D114_F3BF_47A3_A647_37E9AB35B4ED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef _MPLAYER
#define SZ_DEFSKIN		_T("WMP11")
#else
#define SZ_DEFSKIN		_T("Fantasy")
#endif

class CSkinMenu;

class CMPSkinFactory : public CSkinFactory  
{
public:
	CMPSkinFactory();
	virtual ~CMPSkinFactory();

	virtual int LoadDefaultSkin();

	virtual void Quit();

	virtual CUIObject * CreateUIObject(CSkin *pSkin, LPCTSTR szClassName);

	virtual CSkin *NewSkinWnd(LPCTSTR szSkinWndName, bool bMainWnd);

	virtual bool LoadMenu(CSkin *pWnd, CMLMenu **ppMenu, LPCTSTR szMenu);

	virtual CSkinMenu *LoadPresetMenu(CSkin *pWnd, LPCTSTR szMenu);

	virtual void OnLanguageChanged();

	virtual void OnSkinWndActivate(CSkin *pWnd);

	virtual void OnMainSkinWndDestory(CSkin *pSkinWnd);

	virtual void SaveOpenedSkinWndNames();

	void TopmostAll(bool bTopmost);
	void MinizeAll(bool bSilently = false);
	void RestoreAll();

	int OpenSimpliestSkin();

#ifndef _MPLAYER
	//
	// Auto close to player window, and track to move with it.
	//

	void BeforeTrackMoveWith(CBaseWnd *pWndChain[], int nCount, CBaseWnd *pWndToTrack);
	void TrackMoveWith(CBaseWnd *pWnd, int x, int y);

	void AddWndCloseto(CBaseWnd *pWnd, LPCTSTR szWndName, LPCTSTR szClass);

	// ISkinWndDragHost
	virtual void GetWndDragAutoCloseTo(vector<CBaseWnd *> &vWnd);

	typedef vector<CWndDrag::WndCloseTo>		V_WNDCLOSETO;

	CWndDrag			m_WndCloseToPlayers;
#endif
#ifdef _WIN32_DESKTOP
	void AllUpdateTransparent();
	void SetClickThrough(bool bClickThrough) { m_bClickThrough = bClickThrough; }
	bool GetClickThrough() const { return m_bClickThrough; }

protected:
	bool				m_bClickThrough;

#endif

protected:
	virtual int ExeDynamicCmd(const CDynamicCmds::DynamicCmd &cmd, CSkin *pSkinWnd, CUIObject *pUIObj);

	SXNode *GetMenuOfMenus(SXNode *pNodeMenus, LPCTSTR szMenuName);

};

#endif // !defined(AFX_MPSKINFACTORY_H__0C65D114_F3BF_47A3_A647_37E9AB35B4ED__INCLUDED_)
