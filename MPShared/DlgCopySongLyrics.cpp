// DlgCopySongLyrics.cpp: implementation of the CDlgCopySongLyrics class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "DlgCopySongLyrics.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
/*

IDD_COPYSONGLYRICS DIALOG  0, 0, 279, 111
STYLE DS_SETFONT | DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
CAPTION "Copy song file and lyrics to..."
FONT 8, "MS Shell Dlg"
BEGIN
    LTEXT           "Source file:",IDC_STATIC,7,11,49,8
    EDITTEXT        IDC_E_SRC,70,9,202,14,ES_AUTOHSCROLL | ES_READONLY
    LTEXT           "Target folder:",IDC_STATIC,7,30,51,8
    EDITTEXT        IDC_E_TARGET,70,28,187,14,ES_AUTOHSCROLL
    PUSHBUTTON      "...",IDC_BROWSE,260,28,12,14
    CONTROL         "copy lyrics",IDC_C_COPYLYRICS,"Button",BS_AUTOCHECKBOX | WS_TABSTOP,7,49,113,10
    CONTROL         "rename lyrics as the same name as the song",IDC_C_RENAMELYRICS,
                    "Button",BS_AUTOCHECKBOX | WS_TABSTOP,7,68,251,10
    DEFPUSHBUTTON   "OK",IDOK,75,90,50,14
    PUSHBUTTON      "Cancel",IDCANCEL,154,90,50,14
END
*/

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

	if (g_Profile.GetInt(_T("CSL_copylyrics"), TRUE))
		CheckButton(IDC_C_COPYLYRICS, true);

	if (g_Profile.GetInt(_T("CSL_renamelyrics"), TRUE))
		CheckButton(IDC_C_RENAMELYRICS, true);

	SetDlgItemText(IDC_E_TARGET, g_Profile.GetString(_T("CSL_targetdir"), _T("")));

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
	g_Profile.WriteString(_T("CSL_targetdir"), szTargetDir);
	DirStringFormat(szTargetDir);

	bCopyLyrics = IsButtonChecked(IDC_C_COPYLYRICS);
	bRenameLyrics = IsButtonChecked(IDC_C_RENAMELYRICS);
	g_Profile.WriteInt(_T("CSL_copylyrics"), bCopyLyrics);
	g_Profile.WriteInt(_T("CSL_renamelyrics"), bRenameLyrics);

	// ¸´ÖÆMP3ÎÄ¼þ
	_tcscpy_safe(szTargetFile, CountOf(szTargetFile), szTargetDir);
	_tcscat_safe(szTargetFile, CountOf(szTargetFile), FileGetName(m_strSong.c_str()));
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
			_tcscpy_safe(szTargetFile, CountOf(szTargetFile), szTargetDir);
			if (bRenameLyrics)
			{
				_tcscpy_safe(szFileName, MAX_PATH, FileGetName(m_strSong.c_str()));
				FileSetExt(szFileName, CountOf(szFileName), FileGetExt(m_strLyrics.c_str()));
			}
			else
			{
				_tcscpy_safe(szFileName, MAX_PATH, FileGetName(m_strLyrics.c_str()));
			}
			_tcscat_safe(szTargetFile, CountOf(szTargetFile), szFileName);
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
