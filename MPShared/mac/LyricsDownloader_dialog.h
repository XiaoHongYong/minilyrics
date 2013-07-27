#pragma once

namespace LyricsDownloader
{

#define SZ_SECT_LYR_DOWNLOADER		_T("LyrDownloader")

class CEmbeddedOptDialog : public CBaseDialog
{
public:
	CEmbeddedOptDialog(UINT nIDTemplate) : CBaseDialog(nIDTemplate) { }

	void GetEmbeddedOptCheck(CBaseDialog *pDlg, VecString &vEmbeddedLyrNames)
	{
		uint32	lst = 0;
		if (pDlg->IsButtonChecked(IDC_C_EDL_M4A))
		{
			lst |= LST_M4A_LYRICS;
			vEmbeddedLyrNames.push_back(SZ_SONG_M4A_LYRICS);
		}

		if (pDlg->IsButtonChecked(IDC_C_EDL_ID3V2_SYLY))
		{
			lst |= LST_ID3V2_SYLT;
			vEmbeddedLyrNames.push_back(SZ_SONG_ID3V2_SYLT);
		}

		if (pDlg->IsButtonChecked(IDC_C_EDL_ID3V2_USLY))
		{
			lst |= LST_ID3V2_USLT;
			vEmbeddedLyrNames.push_back(SZ_SONG_ID3V2_USLT);
		}

		if (pDlg->IsButtonChecked(IDC_C_EDL_LYR3V2))
		{
			lst |= LST_LYRICS3V2;
			vEmbeddedLyrNames.push_back(SZ_SONG_LYRICS3V2);
		}

		g_Profile.WriteInt(SZ_SECT_LYR_DOWNLOADER, _T("EmbeddedLyr"), lst);
	}

	void LoadEmbeddedOptCheck(CBaseDialog *pDlg)
	{
		uint32 lst = g_Profile.GetInt(SZ_SECT_LYR_DOWNLOADER, _T("EmbeddedLyr"), 0);

		if (IsFlagSet(lst, LST_M4A_LYRICS))
			pDlg->CheckButton(IDC_C_EDL_M4A, true);

		if (IsFlagSet(lst, LST_ID3V2_SYLT))
			pDlg->CheckButton(IDC_C_EDL_ID3V2_SYLY, true);

		if (IsFlagSet(lst, LST_ID3V2_USLT))
			pDlg->CheckButton(IDC_C_EDL_ID3V2_USLY, true);

		if (IsFlagSet(lst, LST_LYRICS3V2))
			pDlg->CheckButton(IDC_C_EDL_LYR3V2, true);
	}

	bool LoadBtnCheckStatus(int btnId, LPCTSTR szValueName, bool bDefault)
	{
		bool bVal = g_Profile.GetBool(SZ_SECT_LYR_DOWNLOADER, szValueName, bDefault);
		CheckButton(btnId, bVal);
		return bVal;
	}

	bool SaveBtnCheckStatus(int btnId, LPCTSTR szValueName)
	{
		bool bVal = IsButtonChecked(btnId);
		g_Profile.WriteInt(SZ_SECT_LYR_DOWNLOADER, szValueName, bVal);
		return bVal;
	}

	void EnableEmbeddedLyr(bool bEnable)
	{
		EnableDlgItem(IDC_C_EDL_M4A, bEnable);
		EnableDlgItem(IDC_C_EDL_ID3V2_SYLY, bEnable);
		EnableDlgItem(IDC_C_EDL_ID3V2_USLY, bEnable);
		EnableDlgItem(IDC_C_EDL_LYR3V2, bEnable);
	}

	void OnCommand(UINT uID, UINT nNotifyCode)
	{
		if (uID == IDC_C_SAVE_IN_SONG_FILE || uID == IDC_C_REMOVE_EMBEDDED_LYR)
		{
			EnableEmbeddedLyr(IsButtonChecked(uID));
		}
		else
			CBaseDialog::OnCommand(uID, nNotifyCode);
	}

};

class CDlgAdvDownloadPrompt : public CEmbeddedOptDialog
{
public:
	CDlgAdvDownloadPrompt() : CEmbeddedOptDialog(IDD_LD_ADV_DOWNLOAD_LYR) {
	}
	
	BOOL OnInitDialog()
	{
		CBaseDialog::OnInitDialog();

		LoadBtnCheckStatus(IDC_C_USE_LOCAL_ONLY, _T("OnlyUseLocalLyr"), false);

		LoadBtnCheckStatus(IDC_C_SAVE_IN_SONG_DIR, _T("SaveInSongDir"), false);
		LoadBtnCheckStatus(IDC_C_SAVE_IN_DIR, _T("SaveInDir"), false);
		if (!LoadBtnCheckStatus(IDC_C_AS_SONG_NAME, _T("SaveAsSongName"), false))
			CheckButton(IDC_C_KEEP_NAME, true);

		if (!LoadBtnCheckStatus(IDC_C_SAVE_IN_SONG_FILE, _T("SaveInSongFile"), false))
			EnableEmbeddedLyr(false);

		LoadEmbeddedOptCheck(this);

		SetDlgItemText(IDC_BR_DIR, g_LyricsDownloader.GetDefSavePath());

		return TRUE;
	}

	virtual void OnOK()
	{
		m_bOnlyUseLocalLyrics = SaveBtnCheckStatus(IDC_C_USE_LOCAL_ONLY, _T("OnlyUseLocalLyr"));

		SaveBtnCheckStatus(IDC_C_SAVE_IN_SONG_DIR, _T("SaveInSongDir"));
		SaveBtnCheckStatus(IDC_C_SAVE_IN_DIR, _T("SaveInDir"));
		SaveBtnCheckStatus(IDC_C_AS_SONG_NAME, _T("SaveAsSongName"));

		if (IsButtonChecked(IDC_C_SAVE_IN_SONG_DIR))
			m_downSaveDir = DOWN_SAVE_IN_SONG_DIR;
		else if (IsButtonChecked(IDC_C_SAVE_IN_DIR))
			m_downSaveDir = DOWN_SAVE_IN_CUSTOM_DIR;
		else
			m_downSaveDir = DOWN_SAVE_NO_FILE;

		if (IsButtonChecked(IDC_C_AS_SONG_NAME))
			m_downSaveName = DOWN_SAVE_NAME_AS_SONG_NAME;
		else
			m_downSaveName = DOWN_SAVE_NAME_KEEP;

		GetEmbeddedOptCheck(this, m_vSaveEmbeddedLyrNames);

		bool bSaveInSongFile = SaveBtnCheckStatus(IDC_C_SAVE_IN_SONG_FILE, _T("SaveInSongFile"));
		if (!bSaveInSongFile)
			m_vSaveEmbeddedLyrNames.clear();

		if (m_downSaveDir == DOWN_SAVE_NO_FILE
			&& m_vSaveEmbeddedLyrNames.empty())
			return;

		CBaseDialog::OnOK();
	}

	void OnCommand(UINT uID, UINT nNotifyCode)
	{
		if (uID == IDC_C_SAVE_IN_SONG_DIR)
		{
			if (IsButtonChecked(IDC_C_SAVE_IN_SONG_DIR))
				CheckButton(IDC_C_SAVE_IN_DIR, false);
		}
		else if (uID == IDC_C_SAVE_IN_DIR)
		{
			if (IsButtonChecked(IDC_C_SAVE_IN_DIR))
				CheckButton(IDC_C_SAVE_IN_SONG_DIR, false);
		}
		else if (uID == IDC_BR_DIR)
		{
			CFolderDialog dlg;
			dlg.SetInitFolder(g_LyricsDownloader.GetDefSavePath());
			if (dlg.DoBrowse(this))
			{
				g_LyricsDownloader.SetDefSavePath(dlg.GetFolder());
				SetDlgItemText(IDC_BR_DIR, g_LyricsDownloader.GetDefSavePath());
			}
		}

		CEmbeddedOptDialog::OnCommand(uID, nNotifyCode);
	}
	
public:
	bool			m_bOnlyUseLocalLyrics;
	DOWN_SAVE_DIR	m_downSaveDir;
	DOWN_SAVE_NAME	m_downSaveName;
	VecString		m_vSaveEmbeddedLyrNames;

};


class CDlgAdvRemovePrompt : public CEmbeddedOptDialog
{
public:
	CDlgAdvRemovePrompt() : CEmbeddedOptDialog(IDD_LD_ADV_REMOVE_LYR) {
	}

	BOOL OnInitDialog()
	{
		CBaseDialog::OnInitDialog();

		LoadBtnCheckStatus(IDC_C_REMOVE_LYR_FILE, _T("RemoveLyrFile"), false);

		if (!LoadBtnCheckStatus(IDC_C_REMOVE_EMBEDDED_LYR, _T("RemoveEmbeddedLyr"), false))
			EnableEmbeddedLyr(false);

		LoadEmbeddedOptCheck(this);

		return TRUE;
	}

	virtual void OnOK()
	{
		m_bRemoveAssociateLyrFile = SaveBtnCheckStatus(IDC_C_REMOVE_LYR_FILE, _T("RemoveLyrFile"));

		GetEmbeddedOptCheck(this, m_vEmbeddedLyrNames);

		bool bRemoveEmbeddedLyr = SaveBtnCheckStatus(IDC_C_REMOVE_EMBEDDED_LYR, _T("RemoveEmbeddedLyr"));
		if (!bRemoveEmbeddedLyr)
			m_vEmbeddedLyrNames.clear();

		if (!m_bRemoveAssociateLyrFile && m_vEmbeddedLyrNames.empty())
			return;

		CBaseDialog::OnOK();
	}

public:
	bool		m_bRemoveAssociateLyrFile;
	VecString	m_vEmbeddedLyrNames;

};


}
