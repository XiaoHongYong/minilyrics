// PreferencePageBase.h: interface for the CPreferencePageBase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PREFERENCEPAGEBASE_H__DE19C916_8F60_4132_B485_4D82DC5491B8__INCLUDED_)
#define AFX_PREFERENCEPAGEBASE_H__DE19C916_8F60_4132_B485_4D82DC5491B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum PreferPageID
{
	PAGE_UNKNOWN,

	// User interface
	PAGE_UI,
	PAGE_SKINS,

	// Themes
	PAGE_THEMES,
	PAGE_LYR_DISPLAY,
	PAGE_LYR_BG,

	// System
	PAGE_INET,
	PAGE_ASSOCIATION,
	PAGE_STARTUP,

	// Lyrics
	PAGE_LYR_DOWNLOAD,
	PAGE_LYR_OUTPUT_PLUGINS,

	PAGE_ADVANCED,
};

class CPagePfBase : public CSkinPageHandler, public IUIObjNotifyHandler
{
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

public:
	CPagePfBase(LPCTSTR szIdPage, PreferPageID pfPageId, LPCTSTR szAssociateTabButtonId);

	void OnCreate();

	void OnDestroy();

	bool OnCustomCommand(int nId);

	void AddOptBool(EventType evtType, LPCTSTR szSection, LPCTSTR szSettingName, bool bDefValue, LPCTSTR szCtrlId);
	void AddOptComboStr(OptComboStr &optComboStr) { m_vOptComboStr.push_back(optComboStr); }
	void AddOptRadioInt(OptRadioInt &optRadioInt) { m_vOptRadioInt.push_back(optRadioInt); }

	void InitCheckButtons();

	void OnUIObjNotify(IUIObjNotify *pNotify);

	PreferPageID GetPfPageID() const { return m_pfPageId; }
	int GetAssociateTabButtonId() const { return m_nAssociateTabButtonId; }
	bool SetDefaultPreferPage(PreferPageID pfPageIdDefault);

	void CheckToolbarDefaultPage(LPCTSTR szToolbarId);

protected:
	void ClearOptions();

protected:
	VOptBool			m_vOptBool;
	VOptComboStr		m_vOptComboStr;
	VOptRadioInt		m_vOptRadioInt;
	tstring				m_strPageId;
	PreferPageID		m_pfPageId;
	int					m_nAssociateTabButtonId;

};

#endif // !defined(AFX_PREFERENCEPAGEBASE_H__DE19C916_8F60_4132_B485_4D82DC5491B8__INCLUDED_)
