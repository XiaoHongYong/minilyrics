#include "stdafx.h"
#include "MPlayerApp.h"
#include "PreferPageAdvanced.h"
#include "DownloadMgr.h"

#ifdef _WIN32
#include "win32/HotkeyCtrlEx.h"
#endif

#ifdef _LINUX_GTK2
#include "gtk2/HotkeyCtrlEx.h"
#endif

#ifdef _MAC_OSX

void FormatHotkeyText(tstring &strText, UINT nVirtKey, UINT fsModifiers)
{
	strText = CStrPrintf(_T("%d"), nVirtKey).c_str();
}

#endif

#define DURATION_SEARCH		500

#define SZ_YES			TOLOCAL(_T("Yes"))
#define SZ_NO			TOLOCAL(_T("No"))
#define SZ_ENABLED		TOLOCAL(_T("Enabled"))
#define SZ_DISABLED		TOLOCAL(_T("Disabled"))
#define SZ_EDIT			TOLOCAL(_T("Edit..."))
#define SZ_BROWSE		TOLOCAL(_T("Browse..."))
#define SZ_RESET		TOLOCAL(_T("Reset Selected Settings"))

#define MENUID_START		3000

tstring ConnectToLocalStr(LPCTSTR szStr1, LPCTSTR szStr2)
{
	tstring		str = TOLOCAL(szStr1);

	str += _T(" ");
	str += TOLOCAL(szStr2);

	return str;
}

class CPfItemRestoreLyrChgSavePrompt : public CPreferItem
{
public:
	CPfItemRestoreLyrChgSavePrompt(CBaseWnd *pWnd) : CPreferItem(ConnectToLocalStr(_T("UI:"), _T("Restore saving prompt setting for lyrics changed")).c_str())
	{
		m_pWnd = pWnd;
	}

	virtual tstring GetValue() {
		return _T("");
	}

	virtual void GetOptions(V_Str &vString, int &nRadio) {
		vString.push_back(TOLOCAL_T("Restore"));
	}

	virtual void SetOption(int nIndex) {
		g_Profile.WriteProfileInt(_T("SaveLyrPrompt"), IDCANCEL);
		m_pWnd->MessageOut(TOLOCAL_T("Setting was restored successfully."), MB_OK | MB_ICONINFORMATION);
	}

	virtual bool IsDefault() {
		return true;
	}

	virtual void Reset() {
		
	}

	CBaseWnd		*m_pWnd;

};

class CPfItemInt : public CPreferItem
{
public:
	CPfItemInt(LPCTSTR szName, EventType etNotify, LPCTSTR szSectName, LPCTSTR szValueName, int nDefault) : CPreferItem(szName)
	{
		eventType = etNotify;
		strSection = szSectName;
		strValueName = szValueName;
		nDefaultValue = nDefault;
	}

	virtual tstring GetValue() {
		return CStrPrintf(_T("%d"), GetIntValue()).c_str();
	}

	virtual int GetIntValue()
	{
		return g_Profile.GetProfileInt(strSection.c_str(), strValueName.c_str(), nDefaultValue);
	}

	virtual void SetIntValue(int nValue)
	{
		CMPlayerSettings::SetSettings(eventType, strSection.c_str(),
			strValueName.c_str(), nValue, eventType != ET_INVALID);
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
	tstring		strValueName;
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
		: CPfItemBool(ConnectToLocalStr(_T("Lyrics Download:"), _T("Auto download lyrics")).c_str(), ET_NULL, SZ_SECT_LYR_DL,
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
		: CPreferItem(ConnectToLocalStr(_T("Lyrics:"), _T("Lyrics search folder")).c_str())
	{
		m_pWndParent = pWndParent;
	}

	virtual tstring GetValue() {
		tstring		strValue;

		for (int i = 0; i < g_LyricSearch.GetSearchFolerCount(); i++)
		{
			strValue += g_LyricSearch.GetFolder(i);
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
			vString.push_back(tstring(TOLOCAL(_T("Remove folder:"))) + _T(" ") + g_LyricSearch.GetFolder(i));
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

				if (g_LyricSearch.SetFolder(szPath))
				{
					// update lyric
					CMPlayerAppBase::GetInstance()->DispatchResearchLyrics();
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

			CMPlayerAppBase::GetInstance()->DispatchResearchLyrics();
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
		: CPreferItem(ConnectToLocalStr(_T("Lyrics:"), _T("External Lyrics Editor")).c_str())
	{
		m_pWndParent = pWndParent;
	}

	virtual tstring GetValue() {
		tstring		strEditor;
		GetNotepadEditor(strEditor);
		return CMLProfile::GetDir(SZ_SECT_UI, _T("LyricsEditor"), strEditor.c_str()).c_str();
	}

	virtual void GetOptions(V_Str &vString, int &nRadio) {
		vString.push_back(TOLOCAL(SZ_BROWSE));
	}

	virtual void SetOption(int nIndex) {
		ASSERT(nIndex == 0);
		tstring		strEditor;

		GetNotepadEditor(strEditor);

		CFileOpenDlg	dlg(TOLOCAL(_T("Browse external lyrics editor program")), strEditor.c_str(), _T("Executable file(*.exe)\0*.exe\0\0"), 1);

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
class CPfItemWndOpacity : public CPfItemInt
{
public:
	CPfItemWndOpacity() : CPfItemInt(ConnectToLocalStr(_T("UI:"), _T("Window opaque percent (10 - 100)")).c_str(),
		ET_NULL, SZ_SECT_UI, _T("WindowOpaquePercent"), 100) { }

	int GetIntValue() {
		int		nOpaque = CPfItemInt::GetIntValue();
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

		nRadio = GetIntValue() / 10 - 1;
	}

	virtual void SetOption(int nIndex) {
		SetIntValue((nIndex + 1) * 10);
	}

	virtual void SetIntValue(int nValue)
	{
		CPfItemInt::SetIntValue(nValue);
		CMPlayerAppBase::GetMPSkinFactory()->AllUpdateTransparent();
	}

};

class CPfItemTrayIconPlayerCtrl : public CPfItemBool
{
public:
	CPfItemTrayIconPlayerCtrl(LPCTSTR szName, int nPlayerCtrlIndex) 
		: CPfItemBool((tstring(TOLOCAL(_T("System tray icon player control"))) + _T(": ") + TOLOCAL(szName)).c_str(),
		ET_NULL, SZ_SECT_UI,
		_T(""), false)
	{
		TCHAR	szKey[128];

		wsprintf(szKey, _T("TrayIcon%d"), nPlayerCtrlIndex);
		strValueName = szKey;

		this->nPlayerCtrlIndex = nPlayerCtrlIndex;
	}

	virtual void SetIntValue(int nValue)
	{
		g_Profile.WriteProfileInt(strValueName.c_str(), nValue);

		g_SysTrayIconCmd[nPlayerCtrlIndex].bEnable = Tobool(nValue);

		CMPlayerAppBase::GetMainWnd()->UpdatePlayerSysTrayIcon();
	}

	int			nPlayerCtrlIndex;

};

LPCTSTR		g_szSysTrayOpt[] = { _T("&Taskbar only"), _T("S&ystem tray only"), _T("T&askbar and system tray"), _T("&None") };

class CPfItemSystemTrayIcon : public CPfItemInt
{
public:
	CPfItemSystemTrayIcon() 
		: CPfItemInt(_T("Show $Product$ in System tray and taskbar"), 
		ET_UI_SETTINGS_CHANGED, SZ_SECT_UI, _T("ShowIconOn"), SHOW_ICON_ON_TASKBAR)
	{
	}

	virtual tstring GetValue() {
		int	n = GetIntValue();
		if (n < 0 || n >= CountOf(g_szSysTrayOpt))
			n = SHOW_ICON_ON_TASKBAR;
		return RemovePrefixOfAcckey(TOLOCAL(g_szSysTrayOpt[n]));
	}

	virtual void GetOptions(V_Str &vString, int &nRadio) {
		for (int i = 0; i < CountOf(g_szSysTrayOpt); i++)
			vString.push_back(TOLOCAL(g_szSysTrayOpt[i]));

		nRadio = GetIntValue();
	}

	virtual void SetOption(int nIndex) {
		SetIntValue(nIndex);
	}

};

#endif // #ifdef _WIN32_DESKTOP

class CPfItemLyrLineSpacing : public CPfItemInt
{
public:
	CPfItemLyrLineSpacing() : CPfItemInt(ConnectToLocalStr(_T("Lyrics Display:"), _T("Lyrics line spacing")).c_str(),
		ET_LYRICS_DISPLAY_SETTINGS, SZ_SECT_LYR_DISPLAY, _T("LineSpacing"), 2) { }

	virtual void GetOptions(V_Str &vString, int &nRadio) {
		CStrPrintf	str;
		for (int i = 0; i <= 10; i++)
		{
			str.printf(_T("%d"), i);
			vString.push_back(str.c_str());
		}

		nRadio = GetIntValue();
	}

	virtual void SetOption(int nIndex) {
		SetIntValue(nIndex);
	}

};

class CPfItemLyrDelayTime : public CPfItemInt
{
public:
	CPfItemLyrDelayTime() : CPfItemInt(ConnectToLocalStr(_T("Lyrics Display:"), _T("Global lyrics displaying delay time (seconds)")).c_str(),
		ET_NULL, SZ_SECT_LYR_DISPLAY, _T("LyrDelayTime"), 0) { }

	virtual void GetOptions(V_Str &vString, int &nRadio) {
		CStrPrintf	str;
		int			nMax = 20;
		float		fDelayTime = -5.0;
		for (int i = 0; i <= nMax; i++)
		{
			str.printf(_T("%.1f"), fDelayTime + i * 0.5);
			vString.push_back(str.c_str());
		}

		nRadio = GetIntValue() / 500 + nMax / 2;
		if (nRadio >= nMax)
			nRadio = nMax;
		else if (nRadio < 0)
			nRadio = 0;
	}

	virtual void SetOption(int nIndex) {
		int nGlobalOffsetTime = -5000 + 500 * nIndex;
		SetIntValue(nGlobalOffsetTime);
	}

	virtual void SetIntValue(int nValue)
	{
		g_LyricData.m_nGlobalOffsetTime = nValue;
		CMPlayerSettings::SetSettings(eventType, strSection.c_str(),
			strValueName.c_str(), nValue, eventType != ET_INVALID);
	}

};

#ifdef _WIN32
class CDlgShortcutKey : public CBaseDialog
{
public:
	CDlgShortcutKey(LPCTSTR szCaption) : CBaseDialog(IDD_SET_HOTKEY)
	{
		m_strCaption = szCaption;
		m_nVirtualKey = 0;
		m_fsModifiers = 0;
	}
	BOOL OnInitDialog()
	{
		CBaseDialog::OnInitDialog();

		m_hotkey.Init(this, IDC_HOTKEY);
		SetWindowText(m_strCaption.c_str());

		return TRUE;
	}

	void OnOK()
	{
		m_hotkey.GetHotkey(m_nVirtualKey, m_fsModifiers);

		CBaseDialog::OnOK();
	}

	UINT				m_nVirtualKey, m_fsModifiers;

protected:
	CHotkeyCtrlEx		m_hotkey;
	tstring				m_strCaption;

};

#else // #ifdef _WIN32

class CDlgShortcutKey
{
public:
	CDlgShortcutKey(LPCTSTR szCaption)
	{
		m_strCaption = szCaption;
		m_nVirtualKey = 0;
		m_fsModifiers = 0;
	}
	
	int DoModal(CBaseWnd *pWndParent)
	{
		return IDCANCEL;
	}
	
	UINT				m_nVirtualKey, m_fsModifiers;
	
protected:
	tstring				m_strCaption;
	
};
#endif // #ifdef _WIN32


static tstring GetCmdIDDescription(int nCmd)
{
	tstring strTooltip;
	CMPlayerAppBase::GetMPSkinFactory()->GetTooltip(nCmd, strTooltip);
	if (strTooltip.empty())
		return _T("");

	MPHotKeySection		*pSect = g_vHotkeySections;
	for (; pSect->szName != NULL; pSect++)
	{
		for (int k = 0; pSect->vHotkeys[k] != 0; k++)
		{
			if (pSect->vHotkeys[k] == nCmd)
			{
				return tstring(TOLOCAL(pSect->szName)) + _T(" ") + TOLOCAL(strTooltip.c_str());
			}
		}
	}

	return strTooltip;
}

class CPfItemBoolEnableGlobalHotkey : public CPfItemBool
{
public:
	CPfItemBoolEnableGlobalHotkey()
		: CPfItemBool(RemovePrefixOfAcckey(ConnectToLocalStr(_T("Shortcut:"), _T("&Enable Global Hotkeys")).c_str()).c_str(),
			ET_NULL, SZ_APP_NAME, _T("EnableGlobalHotkey"), true)
	{
	}

	virtual void SetIntValue(int nValue)
	{
		CMPlayerAppBase::GetHotkey().EnableGlobalHotkey(Tobool(nValue));
		CPfItemBool::SetIntValue(nValue);
	}

};

class CPfItemShortcut : public CPreferItem
{
public:
	struct Hotkey {
		bool		bGlobal;
		tstring		strHotkey;
	};
	typedef vector<Hotkey>	VecHotkeys;

	CPfItemShortcut(CPagePfAdvanced *pWndParent, LPCTSTR szName, int nCmd)
		: CPreferItem(szName)
	{
		m_nCmd = nCmd;
		m_pWndParent = pWndParent;
	}

	virtual tstring GetValue() {
		tstring		strValue;
		VecHotkeys	vHotkeys;

		EnumAll(vHotkeys);

		for (size_t i = 0; i < vHotkeys.size(); i++)
		{
			if (!strValue.empty())
				strValue.append(_T("; "));
			if (vHotkeys[i].bGlobal)
			{
				strValue += TOLOCAL(_T("Global:"));
				strValue += _T(" ");
			}
			strValue += vHotkeys[i].strHotkey;
		}

		return strValue;
	}

	void EnumAll(VecHotkeys &vHotkeys)
	{
		int			nIndex = -1;
		CMPHotkey::CmdAccKey	*pCmdKey;
		while (1)
		{
			nIndex = CMPlayerAppBase::GetHotkey().GetByCmd(m_nCmd, nIndex);
			if (nIndex == -1)
				break;

			pCmdKey = CMPlayerAppBase::GetHotkey().Get(nIndex);
			if (pCmdKey)
			{
				Hotkey		hotkey;
				FormatHotkeyText(hotkey.strHotkey, pCmdKey->button, pCmdKey->fsModifiers);
				hotkey.bGlobal = pCmdKey->bGlobal;
				vHotkeys.push_back(hotkey);
			}
		}
	}

	virtual void GetOptions(V_Str &vString, int &nRadio) {
		vString.push_back(TOLOCAL(_T("Add Shortcut Key...")));
		vString.push_back(TOLOCAL(_T("Add Global Hotkey...")));

		VecHotkeys		vHotkeys;

		EnumAll(vHotkeys);

		if (vHotkeys.size())
			vString.push_back(_T(""));

		for (size_t i = 0; i < vHotkeys.size(); i++)
		{
			vString.push_back(
				tstring(TOLOCAL(vHotkeys[i].bGlobal ? _T("Remove Global Hotkey:") : _T("Remove Shortcut Key:")))
				+ _T(" ") + vHotkeys[i].strHotkey);
		}
	}

	virtual void SetOption(int nIndex) {
		if (nIndex == 0 || nIndex == 1)
		{
			// Add new shortcut key:
			CDlgShortcutKey		dlg(m_strName.c_str());
			bool				bGloal = (nIndex == 1);

			if (dlg.DoModal(m_pWndParent->GetSkinWnd()) == IDOK)
			{
				// Is this hotkey already being used?
				int			nUsedCmdIndex;
				nUsedCmdIndex = CMPlayerAppBase::GetHotkey().GetByKey(dlg.m_nVirtualKey, dlg.m_fsModifiers);
				if (nUsedCmdIndex != -1)
				{
					tstring		strCmd;
					tstring		strMsg;

					// Same one is self?
					if (CMPlayerAppBase::GetHotkey().Get(nUsedCmdIndex)->cmd == m_nCmd)
						return;

					strCmd = GetCmdIDDescription(CMPlayerAppBase::GetHotkey().Get(nUsedCmdIndex)->cmd);

					strMsg = CStrPrintf(TOLOCAL(_T("This hotkey is currently used by action: %s.")), TOLOCAL(strCmd.c_str())).c_str();
					strMsg += _T("\r\n");
					strMsg += TOLOCAL(_T("Do you want to replace it?"));
					if (m_pWndParent->GetSkinWnd()->MessageOut(strMsg.c_str(), MB_ICONINFORMATION | MB_YESNO) != IDYES)
						return;

					CMPlayerAppBase::GetHotkey().Remove(nUsedCmdIndex);

					m_pWndParent->UpdateValues();
				}

				if (bGloal)
					CMPlayerAppBase::GetHotkey().EnableGlobalHotkey(true);

				CMPlayerAppBase::GetHotkey().Add(m_nCmd, bGloal, dlg.m_nVirtualKey, dlg.m_fsModifiers);
			}
		}
		else
		{
			// Remove shortcut key
			int			nShortcutIndex = -1;
			for (int i = 2; i <= nIndex; i++)
			{
				nShortcutIndex = CMPlayerAppBase::GetHotkey().GetByCmd(m_nCmd, nShortcutIndex);
				if (nShortcutIndex == -1)
					break;

				if (i == nIndex)
					CMPlayerAppBase::GetHotkey().Remove(nShortcutIndex);
			}
		}
	}

	virtual bool IsDefault() {
		return CMPlayerAppBase::GetHotkey().IsDefaultKey(m_nCmd);
	}

	virtual void Reset() {
		return CMPlayerAppBase::GetHotkey().RestoreDefaultKey(m_nCmd);
	}

	CPagePfAdvanced		*m_pWndParent;
	int					m_nCmd;

};

//////////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CPagePfAdvanced, _T("PreferPage.Advanced"))

CPagePfAdvanced::CPagePfAdvanced() : CPagePfBase(PAGE_ADVANCED, _T("CMD_ROOT_ADVANCED"))
{
	m_nMenuIdEnd = m_nMenuIdStart = 0;
	m_bIgnoreSettingListNotify = false;
	CID_LIST_SETTINGS = 0;
	CID_E_ADV_SEARCH = 0;
	m_nTimerIdSearch = 0;
}

CPagePfAdvanced::~CPagePfAdvanced()
{
	for (size_t i = 0; i < m_vPreferItems.size(); i++)
	{
		delete m_vPreferItems[i];
	}
	m_vPreferItems.clear();
}

void CPagePfAdvanced::OnInitialUpdate()
{
	CPagePfBase::OnInitialUpdate();

	GET_ID_BY_NAME(CID_LIST_SETTINGS);
	GET_ID_BY_NAME(CID_E_ADV_SEARCH);

	m_pListItems = (CSkinListCtrl *)GetUIObjectById(CID_LIST_SETTINGS, CSkinListCtrl::ClassName());
	ASSERT(m_pListItems);
	if (!m_pListItems)
		return;

	m_pSkin->RegisterUIObjNotifyHandler(CID_LIST_SETTINGS, this);

	m_pListItems->AddColumn(TOLOCAL(_T("Option")), 450);
	m_pListItems->AddColumn(TOLOCAL(_T("Value")), 150);

#ifdef _WIN32_DESKTOP
	m_vPreferItems.push_back(new CPfItemWndOpacity());
#endif

#ifndef _MPLAYER
	// 自动隐藏迷你歌词如果没有歌词
	m_vPreferItems.push_back(new CPfItemBool(ConnectToLocalStr(_T("UI:"), _T("Activate, minimize and restore with player window")).c_str(),
		ET_NULL, SZ_SECT_UI, _T("ActivateWithWinamp"), true));
	m_vPreferItems.push_back(new CPfItemBool(ConnectToLocalStr(_T("UI:"), _T("Hide toolbar")).c_str(), ET_UI_SETTINGS_CHANGED, SZ_SECT_UI, _T("HideToolbar"), false));
	m_vPreferItems.push_back(new CPfItemBool(ConnectToLocalStr(_T("UI:"), _T("Auto hide MiniLyrics if no lyrics is opened")).c_str(), ET_NULL, SZ_SECT_UI, _T("AutoHideIfNoLyrics"), false));
	m_vPreferItems.push_back(new CPfItemBool(ConnectToLocalStr(_T("UI:"), _T("Auto hide minilyrics when playing movie")).c_str(), ET_NULL, SZ_SECT_UI, _T("AutoHideOnPlayMovie"), true));
#endif
	m_vPreferItems.push_back(new CPfItemBool(ConnectToLocalStr(_T("UI:"), _T("Auto adjust window width with lyrics")).c_str(), ET_NULL, SZ_SECT_UI, _T("AutoAdjustWndWidth"), false));
	m_vPreferItems.push_back(new CPfItemBool(ConnectToLocalStr(_T("UI:"), _T("Auto adjust window height with lyrics")).c_str(), ET_NULL, SZ_SECT_UI, _T("AutoAdjustWndHeight"), true));
	m_vPreferItems.push_back(new CPfItemRestoreLyrChgSavePrompt(m_pSkin));
	// m_vPreferItems.push_back(new CPfItemBool(_T("Hide the link of rating lyrics"), ET_NULL, SZ_SECT_UI, _T("HideRateLink"), true));
	m_vPreferItems.push_back(new CPfItemBool(ConnectToLocalStr(_T("Lyrics Display:"), _T("Allow to adjust the vertical position of lyrics")).c_str(),
		ET_LYRICS_DISPLAY_SETTINGS, SZ_SECT_LYR_DISPLAY, 
		_T("EnableAdjustVertAlign"), false));
	m_vPreferItems.push_back(new CPfItemLyrLineSpacing());
	m_vPreferItems.push_back(new CPfItemLyrDelayTime());

	//
	// Lyrics
	//
	// m_vPreferItems.push_back(new CPfItemBool(_T(""), ));
	m_vPreferItems.push_back(new CPfItemBoolEnableAutoDlLyr());
	m_vPreferItems.push_back(new CPfItemBool(ConnectToLocalStr(_T("Lyrics Download:"), _T("Only download synchronized lyrics with .lrc extension")).c_str(), ET_NULL, SZ_SECT_LYR_DL, _T("OnlyDlSyncLyr"), false));
	m_vPreferItems.push_back(new CPfItemBool(ConnectToLocalStr(_T("Lyrics Download:"), _T("Try to download synchronized lyrics (.lrc) even if having local unsynchronized (.txt) lyrics")).c_str(), ET_NULL, SZ_SECT_LYR_DL, _T("DownLrcEvenIfHasTxt"), true));
	m_vPreferItems.push_back(new CPfItemBool(ConnectToLocalStr(_T("Lyrics Download:"), _T("Pop up lyrics searching dialog when lyrics are found")).c_str(), ET_NULL, SZ_SECT_LYR_DL, _T("DownLrcUserSelect"), false));
	m_vPreferItems.push_back(new CPfItemBool(ConnectToLocalStr(_T("Lyrics Download:"), _T("Save downloaded lyrics in the a, b, c... folder of downloaded folder")).c_str(), ET_NULL, SZ_SECT_LYR_DL, _T("DownSaveByABC"), false));
	m_vPreferItems.push_back(new CPfItemLyrSearchFolder(m_pSkin));
	m_vPreferItems.push_back(new CPfItemLyrExternalEditor(m_pSkin));

#ifdef _WIN32
	//
	// Tray Icon controls
	//
	for (int i = 0; i < MAX_PLAYER_TRAY_ICON_CMD; i++)
	{
		m_vPreferItems.push_back(new CPfItemTrayIconPlayerCtrl(g_SysTrayIconCmd[i].szCmd, i));
	}
	m_vPreferItems.push_back(new CPfItemSystemTrayIcon());
#endif

	m_vPreferItems.push_back(new CPfItemBoolEnableGlobalHotkey());

	//
	// Shortcut keys
	//
	MPHotKeySection		*pSect = g_vHotkeySections;
	for (; pSect->szName != NULL; pSect++)
	{
		for (int k = 0; pSect->vHotkeys[k] != 0; k++)
		{
			tstring strTooltip;
			CMPlayerAppBase::GetMPSkinFactory()->GetTooltip(pSect->vHotkeys[k], strTooltip);
			if (strTooltip.empty())
				continue;

			m_vPreferItems.push_back(new CPfItemShortcut(
				this,
				(tstring(TOLOCAL(_T("Shortcut:"))) + _T(" ")
				+ TOLOCAL(pSect->szName) + _T(" ") + TOLOCAL(strTooltip.c_str())).c_str(),
				pSect->vHotkeys[k]));
		}
	}

	ListAllItems();

	CSkinEditCtrl	*pEdit = (CSkinEditCtrl *)GetUIObjectById(CID_E_ADV_SEARCH, CSkinEditCtrl::ClassName());
	ASSERT(pEdit);
	if (pEdit)
		pEdit->SetEditNotification(this);
}

void CPagePfAdvanced::OnDestroy()
{
	m_pSkin->UnregisterUIObjNotifyHandler(this);

	CPagePfBase::OnDestroy();
}

void CPagePfAdvanced::OnUIObjNotify(IUIObjNotify *pNotify)
{
	if (pNotify->nID == CID_LIST_SETTINGS && pNotify->IsKindOf(CSkinListCtrl::ClassName()))
	{
		if (m_bIgnoreSettingListNotify)
			return;

		CSkinListCtrlEventNotify	*pListCtrlNotify = (CSkinListCtrlEventNotify*)pNotify;
		if (pListCtrlNotify->cmd == CSkinListCtrlEventNotify::C_SEL_CHANGED)
		{
			EnableUIObject(GetIDByName(_T("CID_CUSTOMIZE")), m_pListItems->GetNextSelectedItem() != -1);
		}
		else if (pListCtrlNotify->cmd == CSkinListCtrlEventNotify::C_DBL_CLICK
			|| pListCtrlNotify->cmd == CSkinListCtrlEventNotify::C_RBTN_CLICK)
		{
			POINT	pt;
			MLWidget::GetCursorPos(&pt);
			ShowCustomizeMenu(pt.x, pt.y);
		}
	}
}

// Edit control notification
void CPagePfAdvanced::OnTextChanged()
{
	if (m_nTimerIdSearch != 0)
		m_pSkin->UnregisterTimerObject(this, m_nTimerIdSearch);
	m_nTimerIdSearch = m_pSkin->RegisterTimerObject(this, DURATION_SEARCH);
}

void CPagePfAdvanced::OnTimer(int nId)
{
	if (nId != m_nTimerIdSearch)
		return;

	m_pSkin->UnregisterTimerObject(this, m_nTimerIdSearch);
	m_nTimerIdSearch = 0;

	tstring		strKey;

	strKey = GetUIObjectText(CID_E_ADV_SEARCH);
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

	m_pListItems->DeleteAllItems();
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
			int	n = m_pListItems->InsertItem(i, TOLOCAL(item->m_strName.c_str()), item->IsDefault() ? IMG_UNMODIFIED : IMG_MODIFIED, 0, false);
			m_pListItems->SetItemText(n, 1, item->GetValue().c_str(), false);
		}
	}

	m_pListItems->Invalidate();
}

bool CPagePfAdvanced::OnCustomCommand(int nId)
{
	if (nId == GetIDByName(_T("CID_CUSTOMIZE")))
	{
		CRect		rc;

		GetUIObjectRect(nId, rc);
		m_pSkin->ClientToScreen(rc);

		ShowCustomizeMenu(rc.left, rc.top);
	}
	else
		return CPagePfBase::OnCustomCommand(nId);

	return true;
}

bool CPagePfAdvanced::OnCommand(int nId)
{
	if ((int)nId >= m_nMenuIdStart && (int)nId <= m_nMenuIdEnd)
	{
		if (nId == m_nMenuIdEnd)
		{
			// Reset selected settings
			int			nSel = -1;
			while (1)
			{
				nSel = m_pListItems->GetNextSelectedItem(nSel);
				if (nSel >= 0 && nSel < (int)m_vFilteredItems.size())
				{
					CPreferItem	*item = m_vFilteredItems[nSel];
					item->Reset();
					m_pListItems->SetItemText(nSel, 1, item->GetValue().c_str(), false);
					m_pListItems->SetItemImageIndex(nSel, item->IsDefault() ? IMG_UNMODIFIED : IMG_MODIFIED, false);
				}
				else
					break;
			}
		}
		else
		{
			// Set option.
			CPreferItem	*item = GetCurItem();
			if (!item)
				return true;

			item->SetOption(nId - m_nMenuIdStart);

			int		nSel = m_pListItems->GetNextSelectedItem();
			m_pListItems->SetItemText(nSel, 1, item->GetValue().c_str(), false);

			m_pListItems->SetItemImageIndex(nSel, item->IsDefault() ? IMG_UNMODIFIED : IMG_MODIFIED, false);
		}

		m_pListItems->Invalidate();

		return true;
	}

	return false;
}

void CPagePfAdvanced::UpdateValues()
{
	for (size_t i = 0; i < m_vFilteredItems.size(); i++)
	{
		CPreferItem	*item = m_vFilteredItems[i];
		m_pListItems->SetItemText(i, 1, item->GetValue().c_str(), false);
		m_pListItems->SetItemImageIndex(i, item->IsDefault() ? IMG_UNMODIFIED : IMG_MODIFIED, false);
	}

	m_pListItems->Invalidate();
}

CPreferItem *CPagePfAdvanced::GetCurItem()
{
	int			nSel = 0;

	nSel = m_pListItems->GetNextSelectedItem();
	if (nSel < 0 || nSel >= (int)m_vFilteredItems.size())
		return NULL;

	return m_vFilteredItems[nSel];
}

void CPagePfAdvanced::ShowCustomizeMenu(int x, int y)
{
	CMLMenu		menu;
	V_Str		vOptions;
	int			nRadio = -1;
	bool		bEnabeReset = false;

	if (m_pListItems->GetSelectedCount() == 0)
		return;

	m_nMenuIdStart = MENUID_START;
	m_nMenuIdEnd = MENUID_START;

	menu.CreatePopupMenu();

	if (m_pListItems->GetSelectedCount() == 1)
	{
		CPreferItem	*item = GetCurItem();
		if (!item)
			return;

		item->GetOptions(vOptions, nRadio);
		for (size_t i = 0; i < vOptions.size(); i++)
		{
			if (vOptions[i].empty())
				menu.AppendSeperator();
			else
				menu.AppendItem(m_nMenuIdEnd++, vOptions[i].c_str());
		}
		menu.AppendSeperator();
		if (!item->IsDefault())
			bEnabeReset = true;
	}
	else
	{
		int			nSel = -1;

		while (1)
		{
			nSel = m_pListItems->GetNextSelectedItem(nSel);
			if (nSel >= 0 && nSel < (int)m_vFilteredItems.size())
			{
				CPreferItem	*item = m_vFilteredItems[nSel];
				if (!item->IsDefault())
					bEnabeReset = true;
			}
			else
				break;
		}
	}

	menu.AppendItem(m_nMenuIdEnd, SZ_RESET);
	if (!bEnabeReset)
		menu.EnableItem(m_nMenuIdEnd, bEnabeReset);

	if (nRadio != -1)
		menu.CheckRadioItem(m_nMenuIdStart + nRadio, m_nMenuIdStart, m_nMenuIdEnd - 1);

	menu.TrackPopupMenu(x, y, m_pSkin);
}

void CPagePfAdvanced::ListAllItems()
{
	m_pListItems->DeleteAllItems();

	m_vFilteredItems = m_vPreferItems;

	for (size_t i = 0; i < m_vPreferItems.size(); i++)
	{
		CPreferItem	*item = m_vPreferItems[i];
		int	n = m_pListItems->InsertItem(i, TOLOCAL(item->m_strName.c_str()), item->IsDefault() ? IMG_UNMODIFIED : IMG_MODIFIED, 0, false);
		m_pListItems->SetItemText(n, 1, item->GetValue().c_str(), false);
	}

	m_pListItems->Invalidate();
}
