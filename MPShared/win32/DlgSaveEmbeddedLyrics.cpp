#include "stdafx.h"
#include "MPlayerApp.h"
#include "DlgSaveEmbeddedLyrics.h"
#include "AutoProcessEmbeddedLyrics.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDlgSaveEmbeddedLyrics::CDlgSaveEmbeddedLyrics() : CBaseDialog(IDD_SAVE_EMBEDDED_LYR)
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

	// Get current media and lyrics information
	m_strMediaUrl = g_Player.GetSrcMedia();

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
		return CBaseDialog::DoModal(pWnd);
	else
	{
		pWnd->MessageOut(_TLT("This file doesn't support embedded lyrics."));
		return IDCANCEL;
	}
}

BOOL CDlgSaveEmbeddedLyrics::OnInitDialog()
{
	if (!CBaseDialog::OnInitDialog())
		return FALSE;

	m_ctrlListLyrics.Attach(this, IDC_L_LYRICS);
	m_ctrlListLyrics.SetExtendedStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
	int n = m_ctrlListLyrics.InsertColumn(COL_DESC, _TLT("Embedded Lyrics Type"), LVCFMT_LEFT, 450);
	n = m_ctrlListLyrics.InsertColumn(COL_NAME, _TLT(""), LVCFMT_LEFT, 200);

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
	bool bAdded = false;

	for (uint i = 0; i < vAlreadyHave.size(); i++)
	{
		if (StrBeginWith(vAlreadyHave[i].c_str(), szToAdd))
		{
			bAdded = true;
			int n = m_ctrlListLyrics.AppendItem(_TL(LyrSrcTypeToDesc(lstToAdd)));
			m_ctrlListLyrics.SetItemText(n, COL_NAME, vAlreadyHave[i].c_str());
			m_ctrlListLyrics.SetCheck(n, true);
		}
	}

	if (!bAdded)
	{
		int n = m_ctrlListLyrics.AppendItem(_TL(LyrSrcTypeToDesc(lstToAdd)));
		m_ctrlListLyrics.SetItemText(n, COL_NAME, szToAdd);

		if (vAlreadyHave.empty())
		{
			if (IsFlagSet(m_nDefaultEmbededLST, lstToAdd))
				m_ctrlListLyrics.SetCheck(n, true);
		}
	}
}

void CDlgSaveEmbeddedLyrics::OnOK()
{
	VecString vLyrNames;
	uint32 nEmbeddedLST = 0;
	for (int i = 0; i < m_ctrlListLyrics.GetItemCount(); i++)
	{
		if (m_ctrlListLyrics.GetCheck(i))
		{
			tstring strName;
			m_ctrlListLyrics.GetItemText(i, COL_NAME, strName);
			vLyrNames.push_back(strName);
			nEmbeddedLST |= LyrSrcTypeFromName(strName.c_str());
		}
	}

	if (!m_bHasEmbeddedLyrAlready)
		g_Profile.WriteInt(_T("DefaultEmbeddedLST"), nEmbeddedLST);

	DoSaveAction(vLyrNames, this);

	CBaseDialog::OnOK();
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
