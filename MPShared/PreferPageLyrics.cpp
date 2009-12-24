#include "stdafx.h"
#include "MPlayerApp.h"
#include "PreferPageLyrics.h"
#include "LyricsDownloadMgr.h"


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
