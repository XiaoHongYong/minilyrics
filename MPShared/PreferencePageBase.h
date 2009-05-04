// PreferencePageBase.h: interface for the CPreferencePageBase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PREFERENCEPAGEBASE_H__DE19C916_8F60_4132_B485_4D82DC5491B8__INCLUDED_)
#define AFX_PREFERENCEPAGEBASE_H__DE19C916_8F60_4132_B485_4D82DC5491B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EventsDispatcher.h"

enum PreferPageID
{
	PAGE_UNKNOWN,

	// User interface
	PAGE_UI_ROOT,
	PAGE_LOCALIZATION,
	PAGE_SKINS,
	PAGE_UI_ADVANCED,

	// Themes
	PAGE_THEMES,
	PAGE_LYR_DISPLAY,
	PAGE_LYR_BG,

	// System
	PAGE_HOTKEYS,
	PAGE_TRAY_ICON,
	PAGE_INET,
	PAGE_ASSOCIATION,
	PAGE_STARTUP,

	// Lyrics
	PAGE_LYRICS,
	PAGE_LYR_DOWNLOAD,
	PAGE_LYR_OUTPUT_PLUGINS,
};

class CPreferencePageBase : public CBaseDialog  
{
public:
	CPreferencePageBase(UINT nIDTemplate, LPCTSTR szCaption);
	virtual ~CPreferencePageBase();

	BOOL OnInitDialog();

	void OnSize(int cx, int cy);

	virtual void OnHidePage() { }
	virtual void OnShowPage() { }

public:
	struct OptBase
	{
		EventType	eventType;
		tstring		strSection;
		tstring		strSettingName;
		tstring		strDefaultValue;
		int			nDefaultValue;

		void Set(EventType evtType, LPCTSTR szSection, LPCTSTR szSettingName, LPCTSTR szDefValue)
		{
			eventType = evtType;
			strSection = szSection;
			strSettingName = szSettingName;
			strDefaultValue = szDefValue;
		}

		void Set(EventType evtType, LPCTSTR szSection, LPCTSTR szSettingName, int nDefValue)
		{
			eventType = evtType;
			strSection = szSection;
			strSettingName = szSettingName;
			nDefaultValue = nDefValue;
		}
	};
	struct OptBool : public OptBase
	{
		int			nCtrl;
	};
	typedef	list<OptBool>			VOptBool;

	typedef OptBool					OptBoolStr;
	typedef	list<OptBoolStr>		VOptBoolStr;

	struct OptRadioStr : public OptBase
	{
		vector<int>	vCtrls;
		vector<tstring>	vValues;

		void AddCtrlValue(int nCtrl, LPCTSTR szValue)
		{
			vCtrls.push_back(nCtrl);
			vValues.push_back(szValue);
		}
	};
	typedef	list<OptRadioStr>		VOptRadioStr;

	struct OptRadioInt : public OptBase
	{
		vector<int>	vCtrls;
		vector<int>	vValues;

		void AddCtrlValue(int nCtrl, int nValue)
		{
			vCtrls.push_back(nCtrl);
			vValues.push_back(nValue);
		}
	};
	typedef	list<OptRadioInt>		VOptRadioInt;

	struct OptComboStr: public OptBase
	{
		int				nIDWidgetCombo;
		vector<tstring> vValues;

		void AddItemValue(LPCTSTR szValue)
		{
			vValues.push_back(szValue);
		}
	};
	typedef	list<OptComboStr>		VOptComboStr;

	// helper functions for preference pages
	void AddOptComboStr(OptComboStr &optComboStr) { m_vOptComboStr.push_back(optComboStr); }
	void AddOptRadioStr(OptRadioStr &optRadioStr) { m_vOptRadioStr.push_back(optRadioStr); }
	void AddOptRadioInt(OptRadioInt &optRadioInt) { m_vOptRadioInt.push_back(optRadioInt); }
	void AddOptBool(OptBool &optBool) { m_vOptBool.push_back(optBool); }
	void AddOptBoolStr(OptBoolStr &optBoolStr) { m_vOptBoolStr.push_back(optBoolStr); }
	void AddOptBool(EventType evtType, LPCTSTR szSection, LPCTSTR szSettingName, bool bDefValue, int nIDCtrl);
	void AddOptBoolStr(EventType evtType, LPCTSTR szSection, LPCTSTR szSettingName, LPCTSTR szDefValue, int nIDCtrl);

	void InitCheckButtons();

	bool ProcessCommand(UINT uID, UINT nNotifyCode);

protected:
	VOptComboStr		m_vOptComboStr;
	VOptRadioStr		m_vOptRadioStr;
	VOptRadioInt		m_vOptRadioInt;
	VOptBoolStr			m_vOptBoolStr;
	VOptBool			m_vOptBool;

	virtual void ClearOptions();

public:
	//
	// Interface for CPreferenceDlg
	//
	LPCTSTR GetCaption() const { return m_strCaption.c_str(); }
	PreferPageID GetPageID() const { return m_PageID; }
	CPreferencePageBase* GetParentPage() { return m_pParentPage; }
	TreeItemIter *GetTreeItemIter() { return &m_treeItemIter; }

	void SetParentPage(CPreferencePageBase *pParentPage) { m_pParentPage = pParentPage; }
	void SetTreeItemIter(TreeItemIter treeItemIter) { m_treeItemIter = treeItemIter; }

protected:
	tstring				m_strCaption;
	CPreferencePageBase	*m_pParentPage;
	TreeItemIter		m_treeItemIter;
	CMLWndHelper		m_wndHelper;
	PreferPageID		m_PageID;

};

#endif // !defined(AFX_PREFERENCEPAGEBASE_H__DE19C916_8F60_4132_B485_4D82DC5491B8__INCLUDED_)
