#include "stdafx.h"
#include "MPlayerApp.h"
#include "PreferPageLyrics.h"
#include "DownloadMgr.h"

#ifdef _WIN32
#include "win32/LyricsOutPluginMgr.h"
#endif

#ifdef _LINUX_GTK2
#include "gtk2/LyricsOutPluginMgr.h"
#endif

#ifdef _MAC_OSX
#include "mac/LyricsOutPluginMgr.h"
#endif


class CPagePfLyrDownload : public CPagePfBase
{
	UIOBJECT_CLASS_NAME_DECLARE(CPagePfBase)
public:
	CPagePfLyrDownload() : CPagePfBase(PAGE_LYR_DOWNLOAD, _T("CMD_LYR_DOWNLOAD"))
	{
		CID_DOWN_DIR = 0;
		CID_C_SAVE_IN_SONG_DIR = 0;
		CID_C_SAVE_IN_DIR = 0;
		CID_C_SAVE_IN_SONG_FILE = 0;
		CID_C_ID3V2_SYLT = 0;
		CID_C_ID3V2_USLT = 0;
		CID_C_LYR3V2 = 0;
		CID_C_M4A_LYRICS = 0;
	}

	void OnInitialUpdate()
	{
		CPagePfBase::OnInitialUpdate();

		GET_ID_BY_NAME(CID_DOWN_DIR);
		GET_ID_BY_NAME(CID_C_SAVE_IN_SONG_DIR);
		GET_ID_BY_NAME(CID_C_SAVE_IN_DIR);
		GET_ID_BY_NAME2(CID_C_SAVE_IN_SONG_FILE, CID_C_M4A_LYRICS);
		GET_ID_BY_NAME(CID_C_ID3V2_SYLT);
		GET_ID_BY_NAME(CID_C_ID3V2_USLT);
		GET_ID_BY_NAME(CID_C_LYR3V2);

		AddOptBool(ET_NULL, SZ_SECT_LYR_DL, _T("DownSaveId3v2Sylt"), true, _T("CID_C_ID3V2_SYLT"));
		AddOptBool(ET_NULL, SZ_SECT_LYR_DL, _T("DownSaveId3v2Uslt"), true, _T("CID_C_ID3V2_USLT"));
		AddOptBool(ET_NULL, SZ_SECT_LYR_DL, _T("DownSaveLyrics3v2"), false, _T("CID_C_LYR3V2"));
		AddOptBool(ET_NULL, SZ_SECT_LYR_DL, _T("DownSaveLyricsM4a"), false, _T("CID_C_M4A_LYRICS"));

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
		tstring strDownSaveDir = g_LyricsDownloader.GetDefSavePath();
		SetUIObjectText(CID_DOWN_DIR, strDownSaveDir.c_str());

		// 需要保存的歌词目录
		DOWN_SAVE_DIR		DownSaveDir;
		DownSaveDir = (DOWN_SAVE_DIR)g_Profile.GetProfileInt(SZ_SECT_LYR_DL, _T("DownSaveInSongDir"), DOWN_SAVE_IN_CUSTOM_DIR);
		if (DownSaveDir == DOWN_SAVE_IN_SONG_DIR)
		{
			EnableUIObject(CID_DOWN_DIR, false, false);
			CheckButton(CID_C_SAVE_IN_SONG_DIR, true);
		}
		else if (DownSaveDir == DOWN_SAVE_IN_CUSTOM_DIR)
		{
			EnableUIObject(CID_DOWN_DIR, true, false);
			CheckButton(CID_C_SAVE_IN_DIR, true);
		}
		else
			EnableUIObject(CID_DOWN_DIR, false, false);

		if (g_Profile.GetProfileBool(SZ_SECT_LYR_DL, _T("DownSaveEmbeded"), false))
			CheckButton(_T("CID_C_SAVE_IN_SONG_FILE"), true);
		else
		{
			EnableUIObject(CID_C_M4A_LYRICS, false, false);
			EnableUIObject(CID_C_ID3V2_SYLT, false, false);
			EnableUIObject(CID_C_ID3V2_USLT, false, false);
			EnableUIObject(CID_C_LYR3V2, false, false);
		}

		InitCheckButtons();
	}

	bool OnCustomCommand(int nId)
	{
		if (nId == CID_C_SAVE_IN_SONG_DIR
			|| nId == CID_C_SAVE_IN_DIR)
		{
			DOWN_SAVE_DIR		DownSaveDir;
			if (!IsButtonChecked(nId))
				DownSaveDir = DOWN_SAVE_NO_FILE;
			else
			{
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
			}

			EnableUIObject(CID_DOWN_DIR, DownSaveDir == DOWN_SAVE_IN_CUSTOM_DIR);

			g_Profile.WriteProfileInt(SZ_SECT_LYR_DL, _T("DownSaveInSongDir"), DownSaveDir);
		}
		else if (nId == CID_C_SAVE_IN_SONG_FILE)
		{
			bool		bCheck;
			bCheck = IsButtonChecked(nId);

			g_Profile.WriteProfileInt(SZ_SECT_LYR_DL, _T("DownSaveEmbeded"), bCheck);
			EnableUIObject(CID_C_M4A_LYRICS, bCheck);
			EnableUIObject(CID_C_ID3V2_SYLT, bCheck);
			EnableUIObject(CID_C_ID3V2_USLT, bCheck);
			EnableUIObject(CID_C_LYR3V2, bCheck);
		}
		else if (nId == CID_DOWN_DIR)
		{
			CFolderDialog		dlg;
			tstring				strDownSaveDir;

			strDownSaveDir = g_LyricsDownloader.GetDefSavePath();

			dlg.SetInitFolder(strDownSaveDir.c_str());
			if (dlg.DoBrowse(m_pSkin))
			{
				if (_tcsicmp(strDownSaveDir.c_str(), dlg.GetFolder()) != 0)
				{
					strDownSaveDir = dlg.GetFolder();
					DirStringFormat(strDownSaveDir);

					if (!CanWriteInDir(strDownSaveDir.c_str()))
						m_pSkin->MessageOut(_T("Can't save lyrics in the selected folder."));
					else
					{
						CMLProfile::WriteDir(SZ_SECT_LYR_DL, _T("LyricsDownPath"), strDownSaveDir.c_str());
						g_LyricsDownloader.SetDefSavePath(strDownSaveDir.c_str());
					}

					SetUIObjectText(nId, strDownSaveDir.c_str());
				}
			}
		}
		else
			return CPagePfBase::OnCustomCommand(nId);

		return true;
	}

protected:
	int				CID_DOWN_DIR, CID_C_SAVE_IN_SONG_DIR, CID_C_SAVE_IN_DIR;
	int				CID_C_SAVE_IN_SONG_FILE, CID_C_M4A_LYRICS;
	int				CID_C_ID3V2_SYLT, CID_C_ID3V2_USLT, CID_C_LYR3V2;

};

UIOBJECT_CLASS_NAME_IMP(CPagePfLyrDownload, _T("PreferPage.LyrDownload"))

//////////////////////////////////////////////////////////////////////////

class CPagePfLyrSaveOpt : public CPagePfBase
{
	UIOBJECT_CLASS_NAME_DECLARE(CPagePfBase)
public:
	CPagePfLyrSaveOpt() : CPagePfBase(PAGE_LYR_SAVE_OPTIONS, _T("CMD_LYR_SAVE_OPT"))
	{
	}

	void OnInitialUpdate()
	{
		CPagePfBase::OnInitialUpdate();

		AddOptBool(ET_NULL, SZ_SECT_UI, _T("RemoveExtraBlankLines"), true, _T("CID_C_REMOVE_EXTRA_BLANK_LINES"));
		AddOptBool(ET_NULL, SZ_SECT_UI, _T("AppendTimeStampsInTxtLyr"), true, _T("CID_C_APPEND_OFFSET_FOR_IOS"));

		InitCheckButtons();
	}

};

UIOBJECT_CLASS_NAME_IMP(CPagePfLyrSaveOpt, _T("PreferPage.LyrSaveOptions"))

//////////////////////////////////////////////////////////////////////////

class CPagePfLyrOutputPlugin : public CPagePfBase
{
	UIOBJECT_CLASS_NAME_DECLARE(CPagePfBase)
public:
	CPagePfLyrOutputPlugin() : CPagePfBase(PAGE_LYR_OUTPUT_PLUGINS, _T("CMD_LYR_OUTPUT_PLUGIN"))
	{
		m_pListPlugins = NULL;
		CID_LO_PLUGIN_LIST = 0;
	}

	void OnInitialUpdate()
	{
		CPagePfBase::OnInitialUpdate();

		GET_ID_BY_NAME(CID_LO_PLUGIN_LIST);

		m_pSkin->SetUIObjectProperty(_T("CID_GET_PLUG"), SZ_PN_LINK, GetStrName(SN_HTTP_DLPLUGIN));

		m_pSkin->RegisterUIObjNotifyHandler(CID_LO_PLUGIN_LIST, this);

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
				g_lyrOutPlguinMgr.UninstPlugin(n, m_pSkin);
		}
		else if (nId == GetIDByName(_T("CID_CONFIGURE")))
		{
			int		n = m_pListPlugins->GetNextSelectedItem();
			if (n != -1)
				g_lyrOutPlguinMgr.ConfigurePlugin(n, m_pSkin);
		}
		else if (nId == GetIDByName(_T("CID_ABOUT")))
		{
			int		n = m_pListPlugins->GetNextSelectedItem();
			if (n != -1)
				g_lyrOutPlguinMgr.AboutPlugin(n, m_pSkin);
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

UIOBJECT_CLASS_NAME_IMP(CPagePfLyrOutputPlugin, _T("PreferPage.LyrOutputPlugin"))

//////////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CPagePfLyricsRoot, _T("PreferPage.LyricsRoot"))

CPagePfLyricsRoot::CPagePfLyricsRoot() : CPagePfBase(PAGE_UNKNOWN, _T("CMD_ROOT_LYRICS"))
{
}

void CPagePfLyricsRoot::OnInitialUpdate()
{
	CPagePfBase::OnInitialUpdate();

	CheckToolbarDefaultPage(_T("CID_TOOLBAR_LYR"));
}

void RegisterPfLyricsPages(CSkinFactory *pSkinFactory)
{
	AddUIObjNewer2(pSkinFactory, CPagePfLyrDownload);
	AddUIObjNewer2(pSkinFactory, CPagePfLyrSaveOpt);
	AddUIObjNewer2(pSkinFactory, CPagePfLyrOutputPlugin);
	AddUIObjNewer2(pSkinFactory, CPagePfLyricsRoot);
}

