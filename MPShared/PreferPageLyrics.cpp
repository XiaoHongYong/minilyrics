#include "stdafx.h"
#include "MPlayerApp.h"
#include "PreferPageLyrics.h"
#include "LyricsDownloadMgr.h"
#include "SkinListCtrl.h"

#ifdef _WIN32
#include "win32/LyricsOutPluginMgr.h"
#endif

#ifdef _LINUX_GTK2
#include "gtk2/LyricsOutPluginMgr.h"
#endif


class CPagePfLyrDownload : public CPagePfBase
{
public:
	CPagePfLyrDownload() : CPagePfBase(GetPageID(), PAGE_LYR_DOWNLOAD, _T("CMD_LYR_DOWNLOAD"))
	{
	}

	static LPCTSTR GetPageID() { return _T("CID_C_LYR_DOWNLOAD"); }

	void OnCreate()
	{
		CPagePfBase::OnCreate();

		GET_ID_BY_NAME(CID_DOWN_DIR);
		GET_ID_BY_NAME(CID_C_SAVE_IN_SONG_DIR);
		GET_ID_BY_NAME(CID_C_SAVE_IN_DIR);
		GET_ID_BY_NAME(CID_C_SAVE_IN_MP3);
		GET_ID_BY_NAME(CID_C_ID3V2_SYLT);
		GET_ID_BY_NAME(CID_C_ID3V2_USLT);
		GET_ID_BY_NAME(CID_C_LYR3V2);

		AddOptBool(ET_NULL, SZ_SECT_LYR_DL, _T("DownSaveId3v2Sylt"), true, _T("CID_C_ID3V2_SYLT"));
		AddOptBool(ET_NULL, SZ_SECT_LYR_DL, _T("DownSaveId3v2Uslt"), true, _T("CID_C_ID3V2_USLT"));
		AddOptBool(ET_NULL, SZ_SECT_LYR_DL, _T("DownSaveLyrics3v2"), false, _T("CID_C_LYR3V2"));

		{
			// downloaded lyrics file name
			OptRadioInt		opt;
			opt.Set(ET_NULL, SZ_SECT_LYR_DL, _T("DownSaveName"), DOWN_SAVE_NAME_KEEP);
			opt.AddCtrlValue(GetIDByName(_T("CID_C_AS_SONG_NAME")), DOWN_SAVE_NAME_AS_SONG_NAME);
			opt.AddCtrlValue(GetIDByName(_T("CID_C_KEEP_FILENAME")), DOWN_SAVE_NAME_KEEP);
			AddOptRadioInt(opt);
		}

		//
		// 设置下载路径的显示
		tstring strDownSaveDir = CMLProfile::GetDir(SZ_SECT_LYR_DL, _T("LyricsDownPath"), _T(""));
		SetUIObjectText(CID_DOWN_DIR, strDownSaveDir.c_str());

		// 需要保存的歌词目录
		DOWN_SAVE_DIR		DownSaveDir;
		DownSaveDir = (DOWN_SAVE_DIR)g_Profile.GetProfileInt(SZ_SECT_LYR_DL, _T("DownSaveInSongDir"), DOWN_SAVE_IN_CUSTOM_DIR);
		if (DownSaveDir == DOWN_SAVE_IN_SONG_DIR)
		{
			EnableUIObject(CID_DOWN_DIR, false, false);
			CheckButton(CID_C_SAVE_IN_SONG_DIR, true);
		}
		else
		{
			EnableUIObject(CID_DOWN_DIR, true, false);
			CheckButton(CID_C_SAVE_IN_DIR, true);
		}

		if (g_Profile.GetProfileBool(SZ_SECT_LYR_DL, _T("DownSaveEmbeded"), false))
		{
			CheckButton(_T("CID_C_SAVE_IN_MP3"), true);

			EnableUIObject(CID_C_ID3V2_SYLT, true);
			EnableUIObject(CID_C_ID3V2_USLT, true);
			EnableUIObject(CID_C_LYR3V2, true);
		}

		InitCheckButtons();
	}

	void OnDestroy()
	{

		CPagePfBase::OnDestroy();
	}

	bool OnCustomCommand(int nId)
	{
		if (nId == CID_C_SAVE_IN_SONG_DIR
			|| nId == CID_C_SAVE_IN_DIR)
		{
			DOWN_SAVE_DIR		DownSaveDir;
			if (nId == CID_C_SAVE_IN_SONG_DIR)
			{
				DownSaveDir = DOWN_SAVE_IN_SONG_DIR;
				CheckButton(CID_C_SAVE_IN_DIR, false);
			}
			else
			{
				DownSaveDir = DOWN_SAVE_IN_CUSTOM_DIR;
				CheckButton(CID_C_SAVE_IN_SONG_DIR, false);
			}

			EnableUIObject(CID_DOWN_DIR, DownSaveDir == DOWN_SAVE_IN_CUSTOM_DIR);

			g_Profile.WriteProfileInt(SZ_SECT_LYR_DL, _T("DownSaveInSongDir"), DownSaveDir);
		}
		else if (nId == CID_C_SAVE_IN_MP3)
		{
			bool		bCheck;
			bCheck = IsButtonChecked(nId);

			g_Profile.WriteProfileInt(SZ_SECT_LYR_DL, _T("DownSaveEmbeded"), bCheck);
			EnableUIObject(CID_C_ID3V2_SYLT, bCheck);
			EnableUIObject(CID_C_ID3V2_USLT, bCheck);
			EnableUIObject(CID_C_LYR3V2, bCheck);
		}
		else if (nId == CID_DOWN_DIR)
		{
			CFolderDialog		dlg;
			tstring				strDownSaveDir;

			// 取得原来的Skin目录
			strDownSaveDir = CMLProfile::GetDir(SZ_SECT_LYR_DL, _T("LyricsDownPath"), _T(""));

			dlg.SetInitFolder(strDownSaveDir.c_str());
			if (dlg.DoBrowse(m_pSkinWnd))
			{
				if (_tcsicmp(strDownSaveDir.c_str(), dlg.GetFolder()) != 0)
				{
					strDownSaveDir = dlg.GetFolder();
					DirStringFormat(strDownSaveDir);

					CMLProfile::WriteDir(SZ_SECT_LYR_DL, _T("LyricsDownPath"), strDownSaveDir.c_str());
					g_LyricsDownloader.SetDefSavePath(strDownSaveDir.c_str());

					SetUIObjectText(nId, strDownSaveDir.c_str());
				}
			}
		}
		else
			return CPagePfBase::OnCustomCommand(nId);

		return true;
	}

protected:
	int				CID_DOWN_DIR, CID_C_SAVE_IN_SONG_DIR, CID_C_SAVE_IN_DIR, CID_C_SAVE_IN_MP3;
	int				CID_C_ID3V2_SYLT, CID_C_ID3V2_USLT, CID_C_LYR3V2;

};

//////////////////////////////////////////////////////////////////////////

class CPagePfLyrOutputPlugin : public CPagePfBase
{
public:
	CPagePfLyrOutputPlugin() : CPagePfBase(GetPageID(), PAGE_LYR_OUTPUT_PLUGINS, _T("CMD_LYR_OUTPUT_PLUGIN"))
	{
		m_pListPlugins = NULL;
	}

	static LPCTSTR GetPageID() { return _T("CID_C_LYR_OUTPUT_PLUGIN"); }

	void OnCreate()
	{
		CPagePfBase::OnCreate();

		GET_ID_BY_NAME(CID_LO_PLUGIN_LIST);

		m_pSkinWnd->SetUIObjectProperty(_T("CID_GET_PLUG"), SZ_PN_LINK, GetStrName(SN_HTTP_DLPLUGIN));

		m_pSkinWnd->RegisterUIObjNotifyHandler(CID_LO_PLUGIN_LIST, this);

		m_pListPlugins = (CSkinListCtrl *)GetUIObjectById(CID_LO_PLUGIN_LIST, CSkinListCtrl::ClassName());
		if (!m_pListPlugins)
			return;

		m_pListPlugins->AddColumn(_T("Plug-ins"), 100);

		vector<tstring>		vPlugins;
		g_lyrOutPlguinMgr.GetLoadedPlugins(vPlugins);
		for (int i = 0; i < (int)vPlugins.size(); i++)
		{
			m_pListPlugins->InsertItem(i, vPlugins[i].c_str());
		}

		EnablePluginBtns();
	}

	bool OnCustomCommand(int nId)
	{
		if (!m_pListPlugins)
			return CPagePfBase::OnCustomCommand(nId);

		if (nId == GetIDByName(_T("CID_UNINST_PLUG")))
		{
			int		n = m_pListPlugins->GetNextSelectedItem();
			if (n != -1)
				g_lyrOutPlguinMgr.UninstPlugin(n, m_pSkinWnd);
		}
		else if (nId == GetIDByName(_T("CID_CONFIGURE")))
		{
			int		n = m_pListPlugins->GetNextSelectedItem();
			if (n != -1)
				g_lyrOutPlguinMgr.ConfigurePlugin(n, m_pSkinWnd);
		}
		else if (nId == GetIDByName(_T("CID_ABOUT")))
		{
			int		n = m_pListPlugins->GetNextSelectedItem();
			if (n != -1)
				g_lyrOutPlguinMgr.AboutPlugin(n, m_pSkinWnd);
		}
		else
			return CPagePfBase::OnCustomCommand(nId);

		return true;
	}

	void OnUIObjNotify(IUIObjNotify *pNotify)
	{
		if (pNotify->nID == CID_LO_PLUGIN_LIST && pNotify->IsKindOf(CSkinListCtrl::ClassName()))
		{
			CSkinListCtrlEventNotify	*pListCtrlNotify = (CSkinListCtrlEventNotify*)pNotify;
			if (pListCtrlNotify->cmd == CSkinListCtrlEventNotify::C_SEL_CHANGED)
				EnablePluginBtns();
		}
	}

protected:
	void EnablePluginBtns()
	{
		ASSERT(m_pListPlugins);
		bool	bEnable = (m_pListPlugins->GetNextSelectedItem() != -1);

		EnableUIObject(_T("CID_CONFIGURE"), bEnable);
		EnableUIObject(_T("CID_ABOUT"), bEnable);
		EnableUIObject(_T("CID_UNINST_PLUG"), bEnable);
	}

	CSkinListCtrl	*m_pListPlugins;
	int				CID_LO_PLUGIN_LIST;

};

//////////////////////////////////////////////////////////////////////////

CPagePfLyricsRoot::CPagePfLyricsRoot() : CPagePfBase(GetPageID(), PAGE_UNKNOWN, _T("CMD_ROOT_LYRICS"))
{
	AddPage(new CPagePfLyrDownload());
	AddPage(new CPagePfLyrOutputPlugin());
}

LPCTSTR CPagePfLyricsRoot::GetPageID()
{
	return _T("CID_C_LYR_ROOT");
}

void CPagePfLyricsRoot::OnCreate()
{
	CPagePfBase::OnCreate();

	CheckToolbarDefaultPage(_T("CID_TOOLBAR_LYR"));
}
/*
bool CPagePfLyricsRoot::OnCustomCommand(int nId)
{
	if (nId == GetIDByName(_T("CMD_LYR_DOWNLOAD")))
		SwitchToPage(CPagePfLyrDownload::GetPageID());
	else if (nId == GetIDByName(_T("CMD_LYR_OUTPUT_PLUGIN")))
		SwitchToPage(CPagePfLyrOutputPlugin::GetPageID());
	else
		return CPagePfBase::OnCustomCommand(nId);

	return true;
}*/

//////////////////////////////////////////////////////////////////////////

CPreferPageLyricsDownload::CPreferPageLyricsDownload() : CPreferencePageBase(IDD_PAGE_LYR_DOWNLOAD, _T("Lyrics Download"))
{
	m_PageID = PAGE_LYR_DOWNLOAD;
}

CPreferPageLyricsDownload::~CPreferPageLyricsDownload()
{
}

BOOL CPreferPageLyricsDownload::OnInitDialog()
{
	tstring				strDownSaveDir;

	CPreferencePageBase::OnInitDialog();

	m_LangDlgHelper.RemoveDlgItem(IDC_DOWN_DIR);

	AddOptBool(ET_NULL, SZ_SECT_LYR_DL, _T("DownSaveId3v2Sylt"), true, IDC_EMBEDDED_ID3V2_SYLT);

	AddOptBool(ET_NULL, SZ_SECT_LYR_DL, _T("DownSaveId3v2Uslt"), true, IDC_EMBEDDED_ID3V2_USLT);

	AddOptBool(ET_NULL, SZ_SECT_LYR_DL, _T("DownSaveLyrics3v2"), false, IDC_EMBEDDED_LYRICS3V2);

	{
		// downloaded lyrics file name
		OptRadioInt		opt;
		opt.Set(ET_NULL, SZ_SECT_LYR_DL, _T("DownSaveName"), DOWN_SAVE_NAME_KEEP);
		opt.AddCtrlValue(IDC_DL_SAVE_AS_SONG_NAME, DOWN_SAVE_NAME_AS_SONG_NAME);
		opt.AddCtrlValue(IDC_DL_KEEP_NAME, DOWN_SAVE_NAME_KEEP);
		AddOptRadioInt(opt);
	}

	//
	// 设置下载路径的显示
	strDownSaveDir = CMLProfile::GetDir(SZ_SECT_LYR_DL, _T("LyricsDownPath"), _T(""));
	SetDlgItemText(IDC_DOWN_DIR, strDownSaveDir.c_str());

	// 需要保存的歌词目录
	DOWN_SAVE_DIR		DownSaveDir;
	DownSaveDir = (DOWN_SAVE_DIR)g_Profile.GetProfileInt(SZ_SECT_LYR_DL, _T("DownSaveInSongDir"), DOWN_SAVE_IN_CUSTOM_DIR);
	if (DownSaveDir == DOWN_SAVE_IN_SONG_DIR)
	{
		EnableDlgItem(IDC_DOWN_DIR, FALSE);
		CheckButton(IDC_DOWNLOAD_LYRICS_IN_SONG_DIRECOTRY, true);
	}
	else
	{
		EnableDlgItem(IDC_DOWN_DIR, TRUE);
		CheckButton(IDC_DL_SAVE_IN_DIR, true);
	}

	if (g_Profile.GetProfileBool(SZ_SECT_LYR_DL, _T("DownSaveEmbeded"), false))
	{
		EnableDlgItem(IDC_DOWN_DIR, FALSE);
		CheckButton(IDC_DL_SAVE_TO_MP3, true);

		EnableDlgItem(IDC_EMBEDDED_ID3V2_SYLT, TRUE);
		EnableDlgItem(IDC_EMBEDDED_ID3V2_USLT, TRUE);
		EnableDlgItem(IDC_EMBEDDED_LYRICS3V2, TRUE);
	}

	InitCheckButtons();

	return TRUE;
}

void CPreferPageLyricsDownload::OnCommand(UINT uID, UINT nNotifyCode)
{
	CBaseDialog::OnCommand(uID, nNotifyCode);

	// from menu or accerator
	switch (uID)
	{
	case IDC_DOWNLOAD_LYRICS_IN_SONG_DIRECOTRY:
	case IDC_DL_SAVE_IN_DIR:
		{
			DOWN_SAVE_DIR		DownSaveDir;
			if (IsButtonChecked(IDC_DOWNLOAD_LYRICS_IN_SONG_DIRECOTRY))
				DownSaveDir = DOWN_SAVE_IN_SONG_DIR;
			else
				DownSaveDir = DOWN_SAVE_IN_CUSTOM_DIR;

			EnableDlgItem(IDC_DOWN_DIR, DownSaveDir == DOWN_SAVE_IN_CUSTOM_DIR);

			g_Profile.WriteProfileInt(SZ_SECT_LYR_DL, _T("DownSaveInSongDir"), DownSaveDir);
		}
		break;
	case IDC_DL_SAVE_TO_MP3:
		{
			bool		bCheck;
			bCheck = IsButtonChecked(IDC_DL_SAVE_TO_MP3);

			g_Profile.WriteProfileInt(SZ_SECT_LYR_DL, _T("DownSaveEmbeded"), bCheck);
			EnableDlgItem(IDC_EMBEDDED_ID3V2_SYLT, bCheck);
			EnableDlgItem(IDC_EMBEDDED_ID3V2_USLT, bCheck);
			EnableDlgItem(IDC_EMBEDDED_LYRICS3V2, bCheck);
		}
		break;
	case IDC_DOWN_DIR:
		{
			CFolderDialog		dlg;
			tstring				strDownSaveDir;

			// 取得原来的Skin目录
			strDownSaveDir = CMLProfile::GetDir(SZ_SECT_LYR_DL, _T("LyricsDownPath"), _T(""));

			dlg.SetInitFolder(strDownSaveDir.c_str());
			if (dlg.DoBrowse(this))
			{
				if (_tcsicmp(strDownSaveDir.c_str(), dlg.GetFolder()) != 0)
				{
					strDownSaveDir = dlg.GetFolder();
					DirStringFormat(strDownSaveDir);

					CMLProfile::WriteDir(SZ_SECT_LYR_DL, _T("LyricsDownPath"), strDownSaveDir.c_str());
					g_LyricsDownloader.SetDefSavePath(strDownSaveDir.c_str());

					SetDlgItemText(IDC_DOWN_DIR, strDownSaveDir.c_str());
				}
			}
		}
		break;
	default:
		ProcessCommand(uID, nNotifyCode);
		break;
	}
}
