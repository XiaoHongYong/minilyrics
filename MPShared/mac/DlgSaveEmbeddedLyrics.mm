#import "DlgSaveEmbeddedLyricsController.h"

#import "stdafx.h"
#import "MPlayerApp.h"
#import "DlgSaveEmbeddedLyrics.h"
#import "AutoProcessEmbeddedLyrics.h"

//////////////////////////////////////////////////////////////////////

DlgSaveEmbeddedLyricsController * GetController(CDlgSaveEmbeddedLyrics *pThis)
{
	return (DlgSaveEmbeddedLyricsController *)pThis->GetControllerHandle();
}

CDlgSaveEmbeddedLyrics::CDlgSaveEmbeddedLyrics()
{
	m_nDefaultEmbededLST = g_Profile.GetInt(_T("DefaultEmbeddedLST"), LST_ID3V2_USLT);
	m_bHasEmbeddedLyrAlready = false;
}

CDlgSaveEmbeddedLyrics::~CDlgSaveEmbeddedLyrics()
{

}

int CDlgSaveEmbeddedLyrics::DoModal(CBaseWnd *pWnd)
{
	if (!g_LyricData.HasLyricsOpened())
	{
		pWnd->MessageOut(_TLT("No Lyrics file was opened."));
		return IDCANCEL;
	}
	
	m_pParentWnd = pWnd;

	// Get current media and lyrics information
	m_strMediaUrl = g_Player.GetSrcMedia();
	
	if (m_strMediaUrl.empty() || !IsFileExist(m_strMediaUrl.c_str()))
	{
		pWnd->MessageOut(_TL3("Can't locate the song file path."));
		return IDCANCEL;
	}

	tstring		str;
	g_LyricData.ToString(str, FT_LYRICS_LRC, TRUE);
	TStringToProperFileData(str.c_str(), m_bufLyrics);

	if (CTagLib::IsM4aTagSupported(m_strMediaUrl.c_str()))
	{
		// Save lyrics directly
		VecString vLyrNames;
		vLyrNames.push_back(SZ_SONG_M4A_LYRICS);
		if (DoSaveAction(vLyrNames, pWnd) == ERR_OK)
			pWnd->MessageOut(_TLT("Embedded Lyrics were saved successfully."));
		return IDOK;
	}
	else if (CTagLib::IsID3v2TagSupported(m_strMediaUrl.c_str()))
	{
		DlgSaveEmbeddedLyricsController *controller = [[DlgSaveEmbeddedLyricsController alloc] initWithWindowNibName:@"DlgSaveEmbeddedLyricsController"];
		m_pDlgController = controller;
		OnInitDialog();
		[controller doModal:this];
		
		return IDOK;
	}
	else
	{
		pWnd->MessageOut(_TLT("This file doesn't support embedded lyrics."));
		return IDCANCEL;
	}
	
	
//	NSWindow *pAbtWindow = [controller window];
//	
//	[NSApp runModalForWindow: pAbtWindow];
//	
//	[NSApp endSheet: pAbtWindow];
//	
//	[pAbtWindow orderOut: nil];
//	[controller release];
	
	return IDOK;
}

BOOL CDlgSaveEmbeddedLyrics::OnInitDialog()
{
	VecString vLyricsNames;
	CTagLib::GetEmbeddedLyrics(m_strMediaUrl.c_str(), vLyricsNames);

	m_bHasEmbeddedLyrAlready = vLyricsNames.size() > 0;

	AddEmbeddedItem(vLyricsNames, SZ_SONG_ID3V2_USLT);
	AddEmbeddedItem(vLyricsNames, SZ_SONG_ID3V2_SYLT);
	AddEmbeddedItem(vLyricsNames, SZ_SONG_LYRICS3V2);

	return TRUE;
}

void CDlgSaveEmbeddedLyrics::AddEmbeddedItem(VecString &vAlreadyHave, LPCTSTR szToAdd)
{
	LRC_SOURCE_TYPE lstToAdd = LyrSrcTypeFromName(szToAdd);
	bool bIsChecked = false;

	for (uint i = 0; i < vAlreadyHave.size(); i++)
	{
		if (StrBeginWith(vAlreadyHave[i].c_str(), szToAdd))
		{
			bIsChecked = true;
		}
	}

	if (!bIsChecked)
	{
		if (vAlreadyHave.empty())
		{
			if (IsFlagSet(m_nDefaultEmbededLST, lstToAdd))
				bIsChecked = true;
		}
	}

	[GetController(this) addItem: _TL(LyrSrcTypeToDesc(lstToAdd)) name:szToAdd isChecked:bIsChecked];
}

void CDlgSaveEmbeddedLyrics::OnOK()
{
	VecString vLyrNames;
	uint32 nEmbeddedLST = 0;
	DlgSaveEmbeddedLyricsController *controller = GetController(this);
	
	for (int i = 0; i < [controller getItemCount]; i++)
	{
		if ([controller getCheckAtIndex:i])
		{
			tstring strName = [controller getNameAtIndex:i];
			vLyrNames.push_back(strName);
			nEmbeddedLST |= LyrSrcTypeFromName(strName.c_str());
		}
	}

	if (!m_bHasEmbeddedLyrAlready)
		g_Profile.WriteInt(_T("DefaultEmbeddedLST"), nEmbeddedLST);

	DoSaveAction(vLyrNames, m_pParentWnd);
}

int CDlgSaveEmbeddedLyrics::DoSaveAction(VecString &vLyrNames, CBaseWnd *pWnd)
{
	int nRet = g_autoProcessEmbeddedLyrics.SaveEmbeddedLyrics(m_strMediaUrl.c_str(), _T(""), &m_bufLyrics, vLyrNames);
	if (nRet != ERR_OK)
	{
		tstring strMsg = _TLT("Failed to Save embedded lyrics, MiniLyrics will auto try again later.");
		strMsg += SZ_RETURN;
		strMsg += SZ_RETURN;
		strMsg += ERROR2STR_LOCAL(nRet);
		pWnd->MessageOut(strMsg.c_str());
	}

	return nRet;
}
