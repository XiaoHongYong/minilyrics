#include "stdafx.h"
#include "MPlayerApp.h"
#include "DlgSaveEmbeddedLyrics.h"
#include "AutoProcessEmbeddedLyrics.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDlgSaveEmbeddedLyrics::CDlgSaveEmbeddedLyrics() : CBaseDialog(IDD_SAVE_EMBEDDED_LYR)
{
	m_bId3v2Sync = m_bId3v2Unsyc = m_bLyrics3v2 = false;
}

CDlgSaveEmbeddedLyrics::~CDlgSaveEmbeddedLyrics()
{

}

BOOL CDlgSaveEmbeddedLyrics::OnInitDialog()
{
	if (!CBaseDialog::OnInitDialog())
		return FALSE;

	// Get current media and lyrics information
	m_strMediaUrl = g_Player.GetSrcMedia();

	tstring		str;
	g_LyricData.ToString(str, FT_LYRICS_LRC, TRUE);
	TStringToProperFileData(str.c_str(), m_bufLyrics);

	V_LRCSEARCHRESULT vLyrics;
	if (g_LyricSearch.SearchEmbeddedLyrics(m_strMediaUrl.c_str(), vLyrics) == ERR_OK)
	{
		for (V_LRCSEARCHRESULT::iterator it = vLyrics.begin(); it != vLyrics.end(); ++it)
		{
			LrcSearchResult &item = *it;
			if (StrIBeginWith(item.strUrl.c_str(), SZ_SONG_ID3V2_SYLT))
				m_bId3v2Sync = true;
			else if (StrIBeginWith(item.strUrl.c_str(), SZ_SONG_ID3V2_USLT))
				m_bId3v2Unsyc = true;
			else if (StrIBeginWith(item.strUrl.c_str(), SZ_SONG_LYRICS3V2))
				m_bLyrics3v2 = true;
		}
	}

	if (!m_bId3v2Sync && !m_bId3v2Unsyc && !m_bLyrics3v2)
	{
		m_bId3v2Unsyc = g_Profile.GetProfileBool(_T("SaveId3v2Unsyc"), true);
		m_bId3v2Sync = g_Profile.GetProfileBool(_T("SaveId3v2Sync"), false);
		m_bLyrics3v2 = g_Profile.GetProfileBool(_T("SaveLyr3v2Sync"), false);
	}

	if (m_bId3v2Unsyc)
		CheckButton(IDC_C_ID3V2_UNSYNC, true);

	if (m_bId3v2Sync)
		CheckButton(IDC_C_ID3V2_SYNC, true);

	if (m_bLyrics3v2)
		CheckButton(IDC_C_LYRICS3V2, true);

	return TRUE;
}

void CDlgSaveEmbeddedLyrics::OnOK()
{
	bool bId3v2Sync = false, bId3v2Unsyc = true, bLyrics3v2 = false;

	bId3v2Sync = IsButtonChecked(IDC_C_ID3V2_SYNC);
	bId3v2Unsyc = IsButtonChecked(IDC_C_ID3V2_UNSYNC);
	bLyrics3v2 = IsButtonChecked(IDC_C_LYRICS3V2);

	if (m_bId3v2Sync != bId3v2Sync)
		g_Profile.WriteProfileInt(_T("SaveId3v2Sync"), bId3v2Sync);
	if (m_bId3v2Unsyc != bId3v2Unsyc)
		g_Profile.WriteProfileInt(_T("SaveId3v2Unsyc"), bId3v2Unsyc);
	if (m_bLyrics3v2 != bLyrics3v2)
		g_Profile.WriteProfileInt(_T("SaveLyr3v2Sync"), bLyrics3v2);

	UINT uEmbeddedLyrType = 0;
	if (bId3v2Sync) uEmbeddedLyrType |= LST_ID3V2_SYLT;
	if (bId3v2Unsyc) uEmbeddedLyrType |= LST_ID3V2_USLT;
	if (bLyrics3v2) uEmbeddedLyrType |= LST_LYRICS3V2;

	int nRet = g_autoProcessEmbeddedLyrics.SaveEmbeddedLyrics(m_strMediaUrl.c_str(), _T(""), &m_bufLyrics, uEmbeddedLyrType);
	if (nRet != ERR_OK)
	{
		tstring strMsg = TOLOCAL_T("Failed to Save embedded lyrics, MiniLyrics will auto try again later.");
		strMsg += SZ_RETURN;
		strMsg += SZ_RETURN;
		strMsg += ERROR2STR_LOCAL(nRet);
		MessageOut(strMsg.c_str());
	}

	CBaseDialog::OnOK();
}
