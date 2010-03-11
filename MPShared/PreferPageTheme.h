#ifndef _PREFER_PAGE_THEME_H_
#define _PREFER_PAGE_THEME_H_

#pragma once

#include "PreferencePageBase.h"

class CPagePfThemeRoot : public CPagePfBase
{
public:
	CPagePfThemeRoot(bool bFloatingLyrMode);

	static LPCTSTR GetPageID();

	void OnCreate();

	bool OnCustomCommand(int nId);

};



//
// Themes root
//
class CPPageThemes : public CPreferencePageBase
{
public:
	CPPageThemes();
	~CPPageThemes();

	BOOL OnInitDialog();
	void OnCommand(UINT uID, UINT nNotifyCode);
	BOOL OnNotify(WIDGET_NOTIFY_INFO *info);

protected:
	void UpdateThemesList();

protected:
	CWidgetListCtrl		m_listThemes;

};


//////////////////////////////////////////////////////////////////////////
// 
class CPPageLyrDisplay : public CPreferencePageBase
{
public:
	CPPageLyrDisplay(bool bFloatingLyrics = false);
	~CPPageLyrDisplay();

	BOOL OnInitDialog();
	void OnCommand(UINT uID, UINT nNotifyCode);

	void OnShowPage();

protected:
	enum SettingType
	{
		ST_NORMAL_LYR		= 0,
		ST_FLOATING_LYR		= 1,
	};

	void SetSettingType(SettingType settingType);

	void RecreateBrush();

	enum COLOR_SET_TYPE
	{
		CS_OUTLINED_HIGH_BORDER,
		CS_OUTLINED_LOW_BORDER,
		CS_BG,
		CS_SIZE,
		CS_NONE,
	};

	struct COLOR_SET
	{
		CWidgetColorButton	button;
		MLColor		clr;
		COLORREF	clrDefault;
		DWORD		dwButtonID;
		CRect		rc;
		tstring		strProfileKey;
	};

	COLOR_SET		m_ColorSet[CS_SIZE];
	int				m_nColorSetCount;

	void CreateLOBXlightBmp(bool bHilight);

#ifdef _WIN32
	//
	// Drag left mouse button to choose color.
	//
protected:
	virtual void OnLButtonUp(UINT nFlags, CPoint point);
	virtual void OnLButtonDown(UINT nFlags, CPoint point);
	virtual void OnMouseMove(UINT nFlags, CPoint point);

	void OnPaint(IMLGraphicsBase *pGraph);

	HCURSOR				m_hcursorSelecting;
	COLOR_SET_TYPE		m_SelColorType;
#endif
	EventType			m_eventType;
	tstring				m_strSectName;
	CWidgetImageButton	m_btLOBHilight;
	CWidgetImageButton	m_btLOBLowlight;

	CWidgetComboBox		m_cbLyrStyle, m_cbAlign, m_cbFadeMode;

	CWidgetComboBox		m_cbSettingType;

};

//////////////////////////////////////////////////////////////////////////
// 
class CPreferPageDispBgPic : public CPreferencePageBase
{
public:
	CPreferPageDispBgPic();

	void OnCommand(UINT uID, UINT nNotifyCode);
	BOOL OnInitDialog();
	void OnDestroy();

protected:
	CWidgetComboBox	m_cbPicPos;

};

#endif // _PREFER_PAGE_THEME_H_
