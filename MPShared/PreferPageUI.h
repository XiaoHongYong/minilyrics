#ifndef _PREFER_PAGE_UI_H_
#define _PREFER_PAGE_UI_H_

#pragma once

#include "PreferencePageBase.h"


//
// UI Root
//
class CPPageUI : public CPreferencePageBase
{
public:
	CPPageUI();
	~CPPageUI();

	BOOL OnInitDialog();
	void OnCommand(UINT uID, UINT nNotifyCode);
	void OnDestroy();

protected:
	void UpdateLanguageInfo();

	CMLMenu			m_menuLang;

	// 每一个菜单项对应的文件
	CLanguageTool::V_TRANSFILEINFO	m_vTransFiles;

};


//
// Skins
//
class CPreferPageSkin : public CPreferencePageBase
{
public:
	CPreferPageSkin();
	~CPreferPageSkin();

	BOOL OnInitDialog();
	void OnCommand(UINT uID, UINT nNotifyCode);
	void OnTimer(UINT nIDEvent);

protected:
	BOOL UpdateSkinComment(LPCTSTR szSkin);
	void UpdateSkinList();

protected:
	CHyperLinkCtrl		m_hlDlSkin;
	CWidgetListBox		m_listSkin;
	tstring				m_strEmbeddedSkinName;

};


#endif // _PREFER_PAGE_UI_H_
