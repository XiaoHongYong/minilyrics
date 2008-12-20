// DlgCopySongLyrics.cpp: implementation of the CDlgCopySongLyrics class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "DlgCopySongLyrics.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDlgCopySongLyrics::CDlgCopySongLyrics() : CBaseDialog(IDD_COPYSONGLYRICS)
{
}

CDlgCopySongLyrics::~CDlgCopySongLyrics()
{

}

BOOL CDlgCopySongLyrics::OnInitDialog()
{
	if (!CBaseDialog::OnInitDialog())
		return FALSE;

	if (g_Profile.GetProfileInt(_T("CSL_copylyrics"), TRUE))
		CheckButton(IDC_C_COPYLYRICS, true);

	if (g_Profile.GetProfileInt(_T("CSL_renamelyrics"), TRUE))
		CheckButton(IDC_C_RENAMELYRICS, true);

	SetDlgItemText(IDC_E_TARGET, g_Profile.GetProfileString(_T("CSL_targetdir"), _T("")));

	SetDlgItemText(IDC_E_SRC, m_strSong.c_str());

	return TRUE;
}

void CDlgCopySongLyrics::OnOK()
{
	BOOL		bCopyLyrics;
	BOOL		bRenameLyrics;
	TCHAR		szTargetDir[MAX_PATH];
	TCHAR		szTargetFile[MAX_PATH];
	TCHAR		szFileName[MAX_PATH];

	GetDlgItemText(IDC_E_TARGET, szTargetDir, MAX_PATH);

	if (!IsDirExist(szTargetDir))
	{
		MessageOut(TOLOCAL(_T("Please input the correct target folder.")));
		return;
	}
	g_Profile.WriteProfileString(_T("CSL_targetdir"), szTargetDir);
	DirStringFormat(szTargetDir);

	bCopyLyrics = IsButtonChecked(IDC_C_COPYLYRICS);
	bRenameLyrics = IsButtonChecked(IDC_C_RENAMELYRICS);
	g_Profile.WriteProfileInt(_T("CSL_copylyrics"), bCopyLyrics);
	g_Profile.WriteProfileInt(_T("CSL_renamelyrics"), bRenameLyrics);

	// ¸´ÖÆMP3ÎÄ¼þ
	FileGetName(szFileName, MAX_PATH, m_strSong.c_str());
	_tcscpy(szTargetFile, szTargetDir);
	_tcscat(szTargetFile, szFileName);
	if (!SHCopyFile(m_strSong.c_str(), szTargetFile, this))
	{
		// MessageOut(CWin32Error().Description());
		return;
	}

	if (bCopyLyrics && !IsEmptyString(m_strLyrics.c_str()))
	{
		if (g_LyricData.GetLyricsSourceType() != LST_FILE)
		{
			// save it as a file
			tstring		str;
			if (g_LyricData.ToString(str, FT_LYRICS_LRC, true))
			{
				TCHAR		szLyricsFile[MAX_PATH];

				_tcscpy_safe(szLyricsFile, CountOf(szLyricsFile), szTargetFile);
				if (g_LyricData.GetLyricsFileType() == FT_LYRICS_TXT)
					FileSetExt(szLyricsFile, CountOf(szLyricsFile), _T(".txt"));
				else
					FileSetExt(szLyricsFile, CountOf(szLyricsFile), _T(".lrc"));
				SaveDataAsFile(szLyricsFile, str.c_str(), str.size());
			}
		}
		else
		{
			_tcscpy(szTargetFile, szTargetDir);
			if (bRenameLyrics)
			{
				TCHAR	szExt[64];
				FileGetName(szFileName, MAX_PATH, m_strSong.c_str());
				FileGetExt(m_strLyrics.c_str(), szExt, CountOf(szExt));
				FileSetExt(szFileName, CountOf(szFileName), szExt);
			}
			else
			{
				FileGetName(szFileName, MAX_PATH, m_strLyrics.c_str());
			}
			_tcscat(szTargetFile, szFileName);
			SHCopyFile(m_strLyrics.c_str(), szTargetFile, this);
		}
	}

	CBaseDialog::OnOK();
}

void CDlgCopySongLyrics::OnCommand(UINT uID, UINT nNotifyCode)
{
	CBaseDialog::OnCommand(uID, nNotifyCode);

	switch (uID)
	{
	case IDC_BROWSE:
		{
			CFolderDialog	dlg;
			TCHAR			szFolder[MAX_PATH];

			GetDlgItemText(IDC_E_TARGET, szFolder, MAX_PATH);
			dlg.SetInitFolder(szFolder);
			dlg.SetTitle(TOLOCAL(_T("Target folder:")));
			if (dlg.DoBrowse(this))
				SetDlgItemText(IDC_E_TARGET, dlg.GetFolder());
		}
		break;
//	case IDC_C_COPYLYRICS:
//		break;
//	case IDC_C_RENAMELYRICS:
//		break;
	default:
		break;
	}
}
