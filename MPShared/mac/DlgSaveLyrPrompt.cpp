#include "stdafx.h"
#include "MPlayerApp.h"
#include "DlgSaveLyrPrompt.h"

int SaveLyrDialogBox(CBaseWnd *pWnd)
{
	static bool bInModal = FALSE;
	if (!bInModal)
	{
		bInModal = TRUE;
		
		tstring		strMessage = CStrPrintf(_TLT("The lyrics of the %s file have changed."), 
											FileGetName(g_LyricData.GetSongFileName())).c_str();
		strMessage += _T("\r\n\r\n");
		strMessage += _TLT("Do you want to save the changes?");

		int nRet = pWnd->MessageOut(strMessage.c_str(), MB_OKCANCEL, SZ_APP_NAME);
		
		bInModal = FALSE;
		return nRet;
	}
	
	return IDCANCEL;
}

