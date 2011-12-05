#include "stdafx.h"
#include "MPlayerApp.h"
#include "DlgSaveLyrPrompt.h"

int SaveLyrDialogBox(CBaseWnd *pWnd)
{
	static bool bInModal = FALSE;
	if (!bInModal)
	{
		bInModal = TRUE;
		
		int nRet = g_Profile.GetProfileInt(_T("SaveLyrPrompt"), IDCANCEL);
		if (nRet == IDCANCEL)
		{
			tstring		strMessage = CStrPrintf(TOLOCAL(_T("The lyrics of the %s file have changed.")), 
												FileGetName(g_LyricData.GetSongFileName())).c_str();
			strMessage += _T("\r\n\r\n");
			strMessage += TOLOCAL(_T("Do you want to save the changes?"));
			
			CDlgSaveLyrPrompt dlg(strMessage.c_str());
			nRet = dlg.DoModal(pWnd);
		}
		
		bInModal = FALSE;
		return nRet;
	}
	
	return IDCANCEL;
}

BOOL CDlgSaveLyrPrompt::OnInitDialog()
{
	if (!CBaseDialog::OnInitDialog())
		return FALSE;
	
	SetDlgItemText(IDC_MSG, m_strInfo.c_str());
	
	return TRUE;
}

void CDlgSaveLyrPrompt::OnCommand(UINT uID, UINT nNotifyCode)
{
	switch (uID)
	{
		case IDC_NO_PROMPT:
			break;
		case IDYES:
		case IDNO:
			if (IsButtonChecked(IDC_NO_PROMPT))
				g_Profile.WriteProfileInt(_T("SaveLyrPrompt"), uID);
		case IDCANCEL:
			EndDialog(m_hWnd, uID);
			break;
	}
}
