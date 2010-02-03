// MPSkinFactory.h: interface for the CMPSkinFactory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPSKINFACTORY_H__0C65D114_F3BF_47A3_A647_37E9AB35B4ED__INCLUDED_)
#define AFX_MPSKINFACTORY_H__0C65D114_F3BF_47A3_A647_37E9AB35B4ED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef _MPLAYER
#define SZ_DEFSKIN		_T("ZikiPlayer")
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

	virtual CSkinWnd *NewSkinWnd(LPCTSTR szSkinWndName, bool bMainWnd);

	virtual bool LoadMenu(CSkinWnd *pWnd, CMLMenu **ppMenu, LPCTSTR szMenu);

	virtual CSkinMenu *LoadPresetMenu(CSkinWnd *pWnd, LPCTSTR szMenu);

	virtual void OnLanguageChanged();

	virtual void OnSkinWndActivate(CSkinWnd *pWnd);

	virtual void OnMainSkinWndDestory(CSkinWnd *pSkinWnd);

	void TopmostAll(bool bTopmost);
	void MinizeAll(bool bSilently = false);
	void RestoreAll();

	virtual int GetIDByNameEx(LPCTSTR szId, tstring &strToolTip);

	int OpenSimpliestSkin();

	virtual void AdjustHue(float hue, float saturation = 0.5, float luminance = 0.5);

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
	void SetClickThrough(bool bClickThrough);
	bool GetClickThrough() const { return m_bClickThrough; }

protected:
	bool				m_bClickThrough;

#endif

protected:
	virtual int ExeDynamicCmd(const CDynamicCmds::DynamicCmd &cmd, CSkinWnd *pSkinWnd);

	virtual CUIObject * NewUIObject(CSkinWnd *pSkin, LPCTSTR szClassName);

	SXNode *GetMenuOfMenus(SXNode *pNodeMenus, LPCTSTR szMenuName);

};

#endif // !defined(AFX_MPSKINFACTORY_H__0C65D114_F3BF_47A3_A647_37E9AB35B4ED__INCLUDED_)
