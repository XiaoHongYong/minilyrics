#include "stdafx.h"
#include "MPlayerApp.h"
#include "PreferPageAdvanced.h"

#define SZ_YES			_T("Yes")
#define SZ_NO			_T("No")
#define SZ_ENABLED		_T("Enabled")
#define SZ_DISABLED		_T("Disabled")
#define SZ_EDIT			_T("Edit...")
#define SZ_RESET		_T("Reset setting of selection")

#define MENUID_START		3000

class CPfItemInt : public CPreferItem
{
public:
	CPfItemInt(LPCTSTR szName, EventType etNotify, LPCTSTR szSectName, LPCTSTR szValueName, int nDefault) : CPreferItem(szName)
	{
		eventType = etNotify;
		strSection = szSectName;
		strSettingName = szValueName;
		nDefaultValue = nDefault;
	}

	virtual tstring GetValue() {
		return CStrPrintf(_T("%d"), GetIntValue()).c_str();
	}

	int GetIntValue()
	{
		return g_Profile.GetProfileInt(strSection.c_str(), strSettingName.c_str(), nDefaultValue);
	}

	virtual void SetIntValue(int nValue)
	{
		CMPlayerSettings::SetSettings(eventType, strSection.c_str(),
			strSettingName.c_str(), nValue, eventType != ET_INVALID);
	}

	virtual void GetOptions(V_Str &vString, int &nRadio) {
		vString.push_back(SZ_EDIT);
	}

	virtual void SetOption(int nIndex) {
		ASSERT(nIndex == 0);
	}

	virtual bool IsDefault() {
		return GetIntValue() == nDefaultValue;
	}

	virtual void Reset() {
		SetIntValue(nDefaultValue);
	}

	EventType	eventType;
	tstring		strSection;
	tstring		strSettingName;
	int			nDefaultValue;

};


class CPfItemBool : public CPfItemInt
{
public:
	CPfItemBool(LPCTSTR szName, EventType etNotify, LPCTSTR szSectName, LPCTSTR szValueName, bool bDefault) 
		: CPfItemInt(szName, etNotify, szSectName, szValueName, bDefault)
	{
	}

	virtual tstring GetValue() {
		if (GetIntValue())
			return SZ_YES;
		else
			return SZ_NO;
	}

	virtual void GetOptions(V_Str &vString, int &nRadio) {
		vString.push_back(SZ_YES);
		vString.push_back(SZ_NO);

		if (GetIntValue())
			nRadio = 0;
		else
			nRadio = 1;
	}

	virtual void SetOption(int nIndex) {
		SetIntValue(nIndex == 0);
	}

};

#ifdef _WIN32_DESKTOP
class CPfItemWndOpacity : public CPreferItem
{
public:
	CPfItemWndOpacity() : CPreferItem(_T("Window opaque percent(10 - 100)")) { }

	virtual tstring GetValue() {
		if (IsEnabled())
			return CStrPrintf(_T("%d"), GetOpacity()).c_str();
		else
			return SZ_DISABLED;
	}

	bool IsEnabled()
		{ return !g_Profile.GetProfileBool(SZ_SECT_UI, _T("WndOpacity"), false); }
	int GetOpacity() {
		int		nOpaque = g_Profile.GetProfileInt(SZ_SECT_UI, _T("WndOpaquePercent"), 70);
		if (nOpaque < 10)
			nOpaque = 10;
		else if (nOpaque > 100)
			nOpaque = 100;
		return nOpaque / 10 * 10;
	}

	virtual void GetOptions(V_Str &vString, int &nRadio) {
		CStrPrintf	str;
		vString.push_back(SZ_DISABLED);
		for (int i = 10; i <= 100; i += 10)
		{
			str.printf(_T("%d"), i);
			vString.push_back(str.c_str());
		}

		if (!IsEnabled())
			nRadio = 0;
		else
			nRadio = GetOpacity() / 10;
	}

	virtual void SetOption(int nIndex) {
		if (nIndex == 0)
			g_Profile.WriteProfileInt(SZ_SECT_UI, _T("WndOpacity"), true);
		else
		{
			if (!IsEnabled())
				g_Profile.WriteProfileInt(SZ_SECT_UI, _T("WndOpacity"), false);
			g_Profile.WriteProfileInt(SZ_SECT_UI, _T("WndOpaquePercent"), nIndex * 10);
		}

		g_mpApp.GetSkinFactory()->AllUpdateTransparent();
	}

	virtual bool IsDefault() {
		return !IsEnabled();
	}

	virtual void Reset() {
		SetOption(0);
	}

};

#endif // #ifdef _WIN32_DESKTOP


CPreferPageAdvanced::CPreferPageAdvanced() : CPreferencePageBase(IDD_PAGE_ADVANCED, _T("Advanced"))
{
	m_PageID = PAGE_ADVANCED;
}

CPreferPageAdvanced::~CPreferPageAdvanced()
{
	for (size_t i = 0; i < m_vPreferItems.size(); i++)
	{
		delete m_vPreferItems[i];
	}
	m_vPreferItems.clear();
}

BOOL CPreferPageAdvanced::OnInitDialog()
{
	CPreferencePageBase::OnInitDialog();

	m_listItems.Attach(this, IDC_LIST_SETTINGS);
	m_listItems.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_listItems.InsertColumn(0, _T("Option"), LVCFMT_LEFT, 200);
	m_listItems.InsertColumn(1, _T("Value"), LVCFMT_LEFT, 200);

#ifdef _WIN32_DESKTOP
	m_vPreferItems.push_back(new CPfItemWndOpacity());
#endif // #ifdef _WIN32_DESKTOP

#ifndef _MPLAYER
	// 自动隐藏迷你歌词如果没有歌词
	m_vPreferItems.push_back(new CPfItemBool(_T("Auto hide MiniLyrics if no lyrics is opened"), ET_NULL, SZ_SECT_UI, _T("AutoHideIfNoLyrics"), false));
	m_vPreferItems.push_back(new CPfItemBool(_T("Auto hide minilyrics when playing movie"), ET_NULL, SZ_SECT_UI, _T("AutoHideOnPlayMovie"), true));
	m_vPreferItems.push_back(new CPfItemBool(_T("Hide toolbar"), ET_UI_SETTINGS_CHANGED, SZ_SECT_UI, _T("HideToolbar"), false));
	m_vPreferItems.push_back(new CPfItemBool(_T("Activate, minimize and restore with player window"),
		ET_NULL, SZ_SECT_UI, _T("ActivateWithWinamp"), true));
#endif
	m_vPreferItems.push_back(new CPfItemBool(_T("Auto adjust window width with lyrics"), ET_NULL, SZ_SECT_UI, _T("AutoAdjustWndWidth"), false));
	m_vPreferItems.push_back(new CPfItemBool(_T("Auto adjust window height with lyrics"), ET_NULL, SZ_SECT_UI, _T("AutoAdjustWndHeight"), true));
	// m_vPreferItems.push_back(new CPfItemBool(_T("Hide the link of rating lyrics"), ET_NULL, SZ_SECT_UI, _T("HideRateLink"), true));
	m_vPreferItems.push_back(new CPfItemBool(_T("Allow to adjust the vertical position of lyrics"), ET_LYRICS_DISPLAY_SETTINGS, SZ_SECT_LYR_DISPLAY, _T("EnableAdjustVertAlign"), false));

	for (size_t i = 0; i < m_vPreferItems.size(); i++)
	{
		CPreferItem	*item = m_vPreferItems[i];
		int	n = m_listItems.InsertItem(i, item->m_strName.c_str());
		m_listItems.SetItemText(n, 1, item->GetValue().c_str());
	}

	return TRUE;
}

void CPreferPageAdvanced::OnDestroy()
{

	CPreferencePageBase::OnDestroy();
}

BOOL CPreferPageAdvanced::OnNotify(WIDGET_NOTIFY_INFO *info)
{
	if (info->nCtrlID == IDC_LIST_SETTINGS)
	{
		if (info->nCode == WN_GEN_DBL_CLICK)
		{
			POINT	pt;
			GetCursorPos(&pt);
			ShowCustomizeMenu(pt.x, pt.y);
		}
	}

	return true;
}

void CPreferPageAdvanced::OnCommand(UINT uID, UINT nNotifyCode)
{
	CBaseDialog::OnCommand(uID, nNotifyCode);

	switch (uID)
	{
	case IDC_CUSTOMIZE:
		{
			CRect		rc;

			GetDlgItemRect(IDC_CUSTOMIZE, &rc);

			ShowCustomizeMenu(rc.left, rc.top);
		}
		break;
	default:
		if (uID >= m_nMenuIdStart && uID <= m_nMenuIdEnd)
		{
			CPreferItem	*item = GetCurItem();
			if (!item)
				break;

			if (uID == m_nMenuIdEnd)
				item->Reset();
			else
				item->SetOption(uID - m_nMenuIdStart);

			int		nSel = m_listItems.GetNextSelectedItem();
			m_listItems.SetItemText(nSel, 1, item->GetValue().c_str());
		}

		break;
	}
}

CPreferItem *CPreferPageAdvanced::GetCurItem()
{
	int			nSel = 0;

	nSel = m_listItems.GetNextSelectedItem();
	if (nSel < 0 || nSel >= m_vPreferItems.size())
		return NULL;

	return m_vPreferItems[nSel];
}

void CPreferPageAdvanced::ShowCustomizeMenu(int x, int y)
{
	CMLMenu		menu;
	V_Str		vOptions;
	int			nRadio = -1;
	CPreferItem	*item = GetCurItem();
	if (!item)
		return;

	m_nMenuIdStart = MENUID_START;
	m_nMenuIdEnd = MENUID_START;

	menu.CreatePopupMenu();

	item->GetOptions(vOptions, nRadio);
	for (size_t i = 0; i < vOptions.size(); i++)
	{
		menu.AppendItem(m_nMenuIdEnd++, vOptions[i].c_str());
	}
	menu.AppendSeperator();

	menu.AppendItem(m_nMenuIdEnd, SZ_RESET);
	if (item->IsDefault())
		menu.EnableItem(m_nMenuIdEnd, false);

	if (nRadio != -1)
		menu.CheckRadioItem(m_nMenuIdStart + nRadio, m_nMenuIdStart, m_nMenuIdEnd - 1);

	menu.TrackPopupMenu(x, y, this);
}
