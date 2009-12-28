#include "stdafx.h"
#include "MPlayerApp.h"
#include "PreferPageAdvanced.h"
#include "LyricsDownloadMgr.h"

#define TIMER_ID_SEARCH		1
#define DURATION_SEARCH		500

#define SZ_YES			TOLOCAL(_T("Yes"))
#define SZ_NO			TOLOCAL(_T("No"))
#define SZ_ENABLED		TOLOCAL(_T("Enabled"))
#define SZ_DISABLED		TOLOCAL(_T("Disabled"))
#define SZ_EDIT			TOLOCAL(_T("Edit..."))
#define SZ_RESET		TOLOCAL(_T("Reset Selected Settings"))

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

class CPfItemBoolEnableAutoDlLyr : public CPfItemBool
{
public:
	CPfItemBoolEnableAutoDlLyr() 
		: CPfItemBool(_T("Auto download lyrics"), ET_NULL, SZ_SECT_LYR_DL,
			_T("EnableAuoDownload"), true)
	{
	}

	virtual void SetIntValue(int nValue)
	{
		g_LyricsDownloader.m_bAutoDownload = Tobool(nValue);
		CPfItemBool::SetIntValue(nValue);
	}

};

class CPfItemLyrSearchFolder : public CPreferItem
{
public:
	CPfItemLyrSearchFolder(CBaseWnd *pWndParent)
		: CPreferItem(_T("Lyrics search folder"))
	{
		m_pWndParent = pWndParent;
	}

	virtual tstring GetValue() {
		tstring		strValue;

		for (int i = 0; i < g_LyricSearch.GetSearchFolerCount(); i++)
		{
			CLyricsLocalSearch::SearchFolder	*psf;

			psf = g_LyricSearch.GetFolder(i);
			strValue += psf->strFolder;
			strValue += _T("; ");
		}

		return strValue;
	}

	virtual void GetOptions(V_Str &vString, int &nRadio) {
		vString.push_back(TOLOCAL(_T("Add folder...")));

		if (g_LyricSearch.GetSearchFolerCount() > 0)
			vString.push_back(_T(""));

		for (int i = 0; i < g_LyricSearch.GetSearchFolerCount(); i++)
		{
			CLyricsLocalSearch::SearchFolder	*psf;

			psf = g_LyricSearch.GetFolder(i);
			vString.push_back(tstring(TOLOCAL(_T("Remove folder:"))) + _T(" ") + psf->strFolder);
		}
	}

	virtual void SetOption(int nIndex) {
		if (nIndex == 0)
		{
			// Add new folder:
			CFolderDialog		dlg;
			TCHAR	szPath[MAX_PATH];

			// 取得上次选择过的文件夹的位置
			g_Profile.GetProfileString(_T("LastSelectFolder"), _T(""), szPath, MAX_PATH);
			dlg.SetInitFolder(szPath);

			if (dlg.DoBrowse(m_pWndParent))
			{
				// 保存上次选择过的文件夹的位置
				_tcscpy_safe(szPath, CountOf(szPath), dlg.GetFolder());
				g_Profile.WriteProfileString(_T("LastSelectFolder"), szPath);

				if (! g_LyricSearch.IsFolderAdded(szPath))
				{
					g_LyricSearch.SetFolder(szPath, TRUE);

					// update lyric
					g_mpApp.ResearchLyrics();
					g_LyricSearch.SaveLyricFolderCfg();
				}
			}
		}
		else
		{
			// Remove search folder
			nIndex--;
			if (nIndex >= g_LyricSearch.GetSearchFolerCount())
			{
				ASSERT(0);
				return;
			}
			g_LyricSearch.RemoveFolder(nIndex);
			g_LyricSearch.SaveLyricFolderCfg();

			g_mpApp.ResearchLyrics();
		}
	}

	virtual bool IsDefault() {
		return g_LyricSearch.GetSearchFolerCount() == 0;
	}

	virtual void Reset() {
		for (int i = 0; i < g_LyricSearch.GetSearchFolerCount(); i++)
			g_LyricSearch.RemoveFolder(i);
		g_LyricSearch.SaveLyricFolderCfg();
	}

	CBaseWnd	*m_pWndParent;

};

class CPfItemLyrExternalEditor : public CPreferItem
{
public:
	CPfItemLyrExternalEditor(CBaseWnd *pWndParent)
		: CPreferItem(_T("External Lyrics Editor"))
	{
		m_pWndParent = pWndParent;
	}

	virtual tstring GetValue() {
		tstring		strEditor;
		GetNotepadEditor(strEditor);
		return CMLProfile::GetDir(SZ_SECT_UI, _T("LyricsEditor"), strEditor.c_str()).c_str();
	}

	virtual void GetOptions(V_Str &vString, int &nRadio) {
		vString.push_back(TOLOCAL(SZ_EDIT));
	}

	virtual void SetOption(int nIndex) {
		ASSERT(nIndex == 0);
		tstring		strEditor;

		GetNotepadEditor(strEditor);

		CFileOpenDlg	dlg(TOLOCAL(_T("Browse lyrics editor program")), strEditor.c_str(), _T("Executable file(*.exe)\0*.exe\0\0"), 1);

		if (dlg.DoModal(m_pWndParent) == IDOK)
		{
			CMLProfile::WriteDir(SZ_SECT_UI, _T("LyricsEditor"), dlg.GetOpenFile());
		}
	}

	virtual bool IsDefault() {
		tstring		strEditor;
		GetNotepadEditor(strEditor);
		return _tcscmp(strEditor.c_str(), GetValue().c_str()) == 0;
	}

	virtual void Reset() {
		tstring		strEditor;

		GetNotepadEditor(strEditor);
		CMLProfile::WriteDir(SZ_SECT_UI, _T("LyricsEditor"), strEditor.c_str());
	}

	CBaseWnd	*m_pWndParent;

};



#ifdef _WIN32_DESKTOP
class CPfItemWndOpacity : public CPreferItem
{
public:
	CPfItemWndOpacity() : CPreferItem(_T("Window opaque percent(10 - 100)")) { }

	virtual tstring GetValue() {
		return CStrPrintf(_T("%d"), GetOpacity()).c_str();
	}

	int GetOpacity() {
		int		nOpaque = g_Profile.GetProfileInt(SZ_SECT_UI, _T("WindowOpaquePercent"), 100);
		if (nOpaque < 10)
			nOpaque = 10;
		else if (nOpaque > 100)
			nOpaque = 100;
		return nOpaque / 10 * 10;
	}

	virtual void GetOptions(V_Str &vString, int &nRadio) {
		CStrPrintf	str;
		for (int i = 10; i <= 100; i += 10)
		{
			str.printf(_T("%d"), i);
			vString.push_back(str.c_str());
		}

		nRadio = GetOpacity() / 10 - 1;
	}

	virtual void SetOption(int nIndex) {
		g_Profile.WriteProfileInt(SZ_SECT_UI, _T("WindowOpaquePercent"), (nIndex + 1) * 10);

		g_mpApp.GetSkinFactory()->AllUpdateTransparent();
	}

	virtual bool IsDefault() {
		return GetOpacity() == 100;
	}

	virtual void Reset() {
		SetOption(9);
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
	m_listItems.InsertColumn(0, TOLOCAL(_T("Option")), LVCFMT_LEFT, 300);
	m_listItems.InsertColumn(1, TOLOCAL(_T("Value")), LVCFMT_LEFT, 150);

#ifdef _WIN32
	// 创建ImageList
	m_imgList.LoadBitmap(IDB_ADV_PREFERENCE, 16, RGB(255, 255, 255));

	m_listItems.SetImageList(m_imgList.GetHandle(), LVSIL_SMALL);
#endif

#ifdef _WIN32_DESKTOP
	m_vPreferItems.push_back(new CPfItemWndOpacity());
#endif // #ifdef _WIN32_DESKTOP

#ifndef _MPLAYER
	// 自动隐藏迷你歌词如果没有歌词
	m_vPreferItems.push_back(new CPfItemBool(_T("Activate, minimize and restore with player window"),
		ET_NULL, SZ_SECT_UI, _T("ActivateWithWinamp"), true));
	m_vPreferItems.push_back(new CPfItemBool(_T("Hide toolbar"), ET_UI_SETTINGS_CHANGED, SZ_SECT_UI, _T("HideToolbar"), false));
	m_vPreferItems.push_back(new CPfItemBool(_T("Auto hide MiniLyrics if no lyrics is opened"), ET_NULL, SZ_SECT_UI, _T("AutoHideIfNoLyrics"), false));
	m_vPreferItems.push_back(new CPfItemBool(_T("Auto hide minilyrics when playing movie"), ET_NULL, SZ_SECT_UI, _T("AutoHideOnPlayMovie"), true));
#endif
	m_vPreferItems.push_back(new CPfItemBool(_T("Auto adjust window width with lyrics"), ET_NULL, SZ_SECT_UI, _T("AutoAdjustWndWidth"), false));
	m_vPreferItems.push_back(new CPfItemBool(_T("Auto adjust window height with lyrics"), ET_NULL, SZ_SECT_UI, _T("AutoAdjustWndHeight"), true));
	// m_vPreferItems.push_back(new CPfItemBool(_T("Hide the link of rating lyrics"), ET_NULL, SZ_SECT_UI, _T("HideRateLink"), true));
	m_vPreferItems.push_back(new CPfItemBool(_T("Allow to adjust the vertical position of lyrics"), ET_LYRICS_DISPLAY_SETTINGS, SZ_SECT_LYR_DISPLAY, _T("EnableAdjustVertAlign"), false));

	//
	// Lyrics
	//
	// m_vPreferItems.push_back(new CPfItemBool(_T(""), ));
	m_vPreferItems.push_back(new CPfItemBoolEnableAutoDlLyr());
	m_vPreferItems.push_back(new CPfItemBool(_T("Only download synchronized lyrics with .lrc extension"), ET_NULL, SZ_SECT_LYR_DL, _T("OnlyDlSyncLyr"), false));
	m_vPreferItems.push_back(new CPfItemBool(_T("Pop up lyrics searching dialog when lyrics are found"), ET_NULL, SZ_SECT_LYR_DL, _T("DownLrcUserSelect"), true));
	m_vPreferItems.push_back(new CPfItemBool(_T("Save lyrics in the subfolder in abc order"), ET_NULL, SZ_SECT_LYR_DL, _T("DownSaveByABC"), false));
	m_vPreferItems.push_back(new CPfItemLyrSearchFolder(this));
	m_vPreferItems.push_back(new CPfItemLyrExternalEditor(this));

	ListAllItems();

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
		else if (info->nCode == WN_LV_SELECTED || info->nCode == WN_LV_UNSELECTED)
		{
			EnableDlgItem(IDC_CUSTOMIZE, m_listItems.GetNextSelectedItem() != -1);
		}
	}

	return true;
}

void CPreferPageAdvanced::OnTimer(UINT nIDEvent)
{
	if (nIDEvent != TIMER_ID_SEARCH)
		return;

	KillTimer(TIMER_ID_SEARCH);

	tstring		strKey;

	GetDlgItemText(IDC_E_SEARCH, strKey);
	if (_tcsicmp(strKey.c_str(), m_strLastSearch.c_str()) == 0)
		return;

	m_strLastSearch = strKey;

	V_Str		vKeyWords;
	StrSplit(strKey.c_str(), _T(' '), vKeyWords);
	TrimStr(vKeyWords, ' ');
	if (vKeyWords.empty() || (vKeyWords.size() == 1 && vKeyWords[0].empty()))
	{
		ListAllItems();
		return;
	}

	m_listItems.DeleteAllItems();
	m_vFilteredItems.clear();

	// List results.
	for (size_t i = 0; i < m_vPreferItems.size(); i++)
	{
		CPreferItem	*item = m_vPreferItems[i];
		bool		bMatch = true;

		for (size_t k = 0; k < vKeyWords.size(); k++)
		{
			if (!stristr(TOLOCAL(item->m_strName.c_str()), vKeyWords[k].c_str()))
			{
				bMatch = false;
				break;
			}
		}

		if (bMatch)
		{
			m_vFilteredItems.push_back(item);
			int	n = m_listItems.InsertItem(i, TOLOCAL(item->m_strName.c_str()), item->IsDefault() ? IMG_UNMODIFIED : IMG_MODIFIED);
			m_listItems.SetItemText(n, 1, item->GetValue().c_str());
		}
	}
}

void CPreferPageAdvanced::OnCommand(UINT uID, UINT nNotifyCode)
{
	CBaseDialog::OnCommand(uID, nNotifyCode);

	switch (uID)
	{
	case IDC_E_SEARCH:
		KillTimer(TIMER_ID_SEARCH);
		SetTimer(TIMER_ID_SEARCH, DURATION_SEARCH);
		break;
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

#ifdef _WIN32
			m_listItems.SetItem(nSel, 0, LVIF_IMAGE, _T(""), item->IsDefault() ? IMG_UNMODIFIED : IMG_MODIFIED, 0, 0, 0);
#endif

		}

		break;
	}
}

CPreferItem *CPreferPageAdvanced::GetCurItem()
{
	int			nSel = 0;

	nSel = m_listItems.GetNextSelectedItem();
	if (nSel < 0 || nSel >= m_vFilteredItems.size())
		return NULL;

	return m_vFilteredItems[nSel];
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
		if (vOptions[i].empty())
			menu.AppendSeperator();
		else
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

void CPreferPageAdvanced::ListAllItems()
{
	m_listItems.DeleteAllItems();

	m_vFilteredItems = m_vPreferItems;

	for (size_t i = 0; i < m_vPreferItems.size(); i++)
	{
		CPreferItem	*item = m_vPreferItems[i];
		int	n = m_listItems.InsertItem(i, TOLOCAL(item->m_strName.c_str()), item->IsDefault() ? IMG_UNMODIFIED : IMG_MODIFIED);
		m_listItems.SetItemText(n, 1, item->GetValue().c_str());
	}
}
