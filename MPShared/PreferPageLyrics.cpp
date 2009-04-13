#include "stdafx.h"
#include "MPlayerApp.h"
#include "PreferPageLyrics.h"
#include "LyricsDownloadMgr.h"



CPreferPageLyrics::CPreferPageLyrics() : CPreferencePageBase(IDD_PAGE_LYR, _T("Lyrics"))
{
}

CPreferPageLyrics::~CPreferPageLyrics()
{
}

BOOL CPreferPageLyrics::OnInitDialog()
{
	m_listCtrl.Attach(this, IDC_DIRLIST);
	m_listCtrl.SetExtendedStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

	CRect	rc;

	m_listCtrl.GetWindowRect(&rc);
	m_listCtrl.InsertColumn(0, _T("Lyrics search directory"), LVCFMT_LEFT, rc.Width() - 5);

	// Add initial search folders
	for (int i = g_LyricSearch.GetSearchFolerCount() - 1; i >= 0; i--)
	{
		CLyricsLocalSearch::SearchFolder	*psf;
		int 		nIndex;

		psf = g_LyricSearch.GetFolder(i);
		nIndex = m_listCtrl.InsertItem(m_listCtrl.GetItemCount(), psf->strFolder.c_str());
		m_listCtrl.SetCheck(nIndex, psf->bIncludeSub);
	}

	CPreferencePageBase::OnInitDialog();

	tstring		strEditor;
	GetNotepadEditor(strEditor);
	SetDlgItemText(IDC_BROWSE_LRC_EDITOR, CMLProfile::GetDir(SZ_SECT_UI, _T("LyricsEditor"), strEditor.c_str()).c_str());

	return TRUE;
}

void CPreferPageLyrics::OnDestroy()
{
	// save folder
	TCHAR	szPath[MAX_PATH];
	BOOL	bIncludeSub;

	for (int i = m_listCtrl.GetItemCount() - 1; i >= 0; i --)
	{
		m_listCtrl.GetItemText(i, 0, szPath, MAX_PATH);
		bIncludeSub = m_listCtrl.GetCheck(i);

		g_LyricSearch.SetFolder(szPath, bIncludeSub);
	}

	g_LyricSearch.SaveLyricFolderCfg();

	//
	// 用户有可能修改了搜索选项：搜索子目录；所以无论如何都重新搜索歌词 
	g_mpApp.ResearchLyrics();

	CPreferencePageBase::OnDestroy();
}

BOOL CPreferPageLyrics::OnNotify(WIDGET_NOTIFY_INFO *info)
{
	if (info->nCtrlID == IDC_DIRLIST && info->nCode == WN_LV_SELECTED)
	{
		// 重新搜索歌词
		// g_mpApp.ResearchLyrics();
	}

	return TRUE;
}

void CPreferPageLyrics::OnCommand(UINT uID, UINT nNotifyCode)
{
	CBaseDialog::OnCommand(uID, nNotifyCode);

	switch (uID)
	{
	case IDC_PAGE_SEARCHDIR_ADD:
		{
			CFolderDialog		dlg;
			TCHAR	szPath[MAX_PATH];

			// 取得上次选择过的文件夹的位置
			g_Profile.GetProfileString(_T("LastSelectFolder"), _T(""), szPath, MAX_PATH);
			dlg.SetInitFolder(szPath);

			if (dlg.DoBrowse(this))
			{
				// 保存上次选择过的文件夹的位置
				_tcscpy_safe(szPath, CountOf(szPath), dlg.GetFolder());
				g_Profile.WriteProfileString(_T("LastSelectFolder"), szPath);

				if (! g_LyricSearch.IsFolderAdded(szPath))
				{
					g_LyricSearch.SetFolder(szPath, TRUE);

					// add to list view
					int		nIndex;
					nIndex = m_listCtrl.InsertItem(m_listCtrl.GetItemCount(), szPath);
					m_listCtrl.SetCheck(nIndex, TRUE);

					// update lyric
					// 在WM_NOTIFY消息中,会搜索歌词的
					g_mpApp.ResearchLyrics();
				}
			}
		}
		break;
	case IDC_PAGE_SEARCHDIR_REMOVE:
		{
			int 	nPos;
			TCHAR	szPath[MAX_PATH];

			nPos = m_listCtrl.GetNextSelectedItem(-1);
			if (nPos == -1)
				break;

			m_listCtrl.GetItemText(nPos, 0, szPath, MAX_PATH);
			g_LyricSearch.RemoveFolder(szPath);

			m_listCtrl.DeleteItem(nPos);

			if (nPos >= m_listCtrl.GetItemCount())
				nPos --;

			if (nPos >= 0)
				m_listCtrl.SetItemSelectionState(nPos, TRUE);

			// update lyric
			g_mpApp.ResearchLyrics();
		}
		break;
	case IDC_BROWSE_LRC_EDITOR:
		{
			tstring		strEditor;

			GetNotepadEditor(strEditor);

			CFileOpenDlg	dlg(TOLOCAL(_T("Browse lyrics editor program")), strEditor.c_str(), _T("Executable file(*.exe)\0*.exe\0\0"), 1);

			if (dlg.DoModal(this) == IDOK)
			{
				CMLProfile::WriteDir(SZ_SECT_UI, _T("LyricsEditor"), dlg.GetOpenFile());
				SetDlgItemText(IDC_BROWSE_LRC_EDITOR, dlg.GetOpenFile());
			}
		}
		break;
	default:
		ProcessCommand(uID);
		break;
	}
}

BOOL CPreferPageLyrics::IsDirAdded(TCHAR szDir[])
{
	TCHAR	szPath[MAX_PATH];

	for (int i = m_listCtrl.GetItemCount() - 1; i >= 0; i --)
	{
		m_listCtrl.GetItemText(i, 0, szPath, MAX_PATH);

		if (_tcsicmp(szDir, szPath) == 0)
			return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////


CPreferPageLyricsDownload::CPreferPageLyricsDownload() : CPreferencePageBase(IDD_PAGE_DOWNLOAD, _T("Lyrics Download"))
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

	CheckButton(IDC_ENABLE_AUTODOWN, g_Profile.GetProfileBool(SZ_SECT_LYR_DL, _T("EnableAuoDownload"), true));

	AddOptBool(ET_NULL, SZ_SECT_LYR_DL, _T("NoDlTxtFile"), false, IDC_C_NO_DL_TXT);

	AddOptBool(ET_NULL, SZ_SECT_LYR_DL, _T("DownLrcUserSelect"), true, IDC_DOWN_LRC_LET_USER_SELECT);

	AddOptBool(ET_NULL, SZ_SECT_LYR_DL, _T("DownSaveId3v2Sylt"), true, IDC_EMBEDDED_ID3V2_SYLT);

	AddOptBool(ET_NULL, SZ_SECT_LYR_DL, _T("DownSaveId3v2Uslt"), true, IDC_EMBEDDED_ID3V2_USLT);

	AddOptBool(ET_NULL, SZ_SECT_LYR_DL, _T("DownSaveLyrics3v2"), false, IDC_EMBEDDED_LYRICS3V2);

	AddOptBool(ET_NULL, SZ_SECT_LYR_DL, _T("DownSaveByABC"), false, IDC_SAVE_IN_ABC_FOLDER);

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
		EnableDlgItem(IDC_SAVE_IN_ABC_FOLDER, FALSE);
		CheckButton(IDC_DOWNLOAD_LYRICS_IN_SONG_DIRECOTRY, true);
	}
	else
	{
		EnableDlgItem(IDC_DOWN_DIR, TRUE);
		EnableDlgItem(IDC_SAVE_IN_ABC_FOLDER, TRUE);
		CheckButton(IDC_DL_SAVE_IN_DIR, true);
	}

	if (g_Profile.GetProfileBool(SZ_SECT_LYR_DL, _T("DownSaveEmbeded"), false))
	{
		EnableDlgItem(IDC_DOWN_DIR, FALSE);
		EnableDlgItem(IDC_SAVE_IN_ABC_FOLDER, FALSE);
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
			EnableDlgItem(IDC_SAVE_IN_ABC_FOLDER, DownSaveDir == DOWN_SAVE_IN_CUSTOM_DIR);

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
	case IDC_ENABLE_AUTODOWN:
		{
			g_LyricsDownloader.m_bAutoDownload = IsButtonChecked(IDC_ENABLE_AUTODOWN);

			//
			// 自动下载
			g_Profile.WriteProfileInt(SZ_SECT_LYR_DL, _T("EnableAuoDownload"), 
				g_LyricsDownloader.m_bAutoDownload);
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
		ProcessCommand(uID);
		break;
	}
}
