// DlgBatchProcessEmbeddedLyrics.cpp: implementation of the CDlgBatchProcessEmbeddedLyrics class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "AutoProcessEmbeddedLyrics.h"
#include "DlgBatchProcessEmbeddedLyrics.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDlgBatchProcessEmbeddedLyrics::CDlgBatchProcessEmbeddedLyrics() : CBaseDialog(IDD_BATCH_EMBEDDED_LYR)
{

}

CDlgBatchProcessEmbeddedLyrics::~CDlgBatchProcessEmbeddedLyrics()
{

}

BOOL CDlgBatchProcessEmbeddedLyrics::OnInitDialog()
{
	if (!CBaseDialog::OnInitDialog())
		return FALSE;

	LPCTSTR		szColName[] = { _T("Action"), _T("Song File"), _T("Result"), _T("Embedded Lyrics Type"), _T("Lyrics File")};
	int			nColWidth[] = { 50, 150, 200, 200, 200};

	m_listCtrl.Attach(this, IDC_LIST);
	m_listCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	ASSERT(CountOf(szColName) == CountOf(nColWidth));
	for (int i = 0; i < CountOf(szColName); i++)
		m_listCtrl.InsertColumn(i, TOLOCAL(szColName[i]), LVCFMT_LEFT, nColWidth[i]);

	CMutexAutolock	autolock(g_autoProcessEmbeddedLyrics.m_mutex);

	CAutoProcessEmbeddedLyrics::LIST_ITEMS::iterator	it, itEnd;
	itEnd = g_autoProcessEmbeddedLyrics.m_listJobs.end();
	for (it = g_autoProcessEmbeddedLyrics.m_listJobs.begin(); it != itEnd; ++it)
	{
		CAutoProcessEmbeddedLyrics::Item	&item = *it;
		int n = m_listCtrl.InsertItem(m_listCtrl.GetItemCount(), item.m_bRemove ? _T("Remove") : _T("Add"));
		m_listCtrl.SetItemText(n, COL_SONG, FileGetName(item.m_strSongFile.c_str()));
		tstring strName = StrJoin(item.m_vLyrNames.begin(), item.m_vLyrNames.end(), _T(", "));
		m_listCtrl.SetItemText(n, COL_EMBEDDED_TYPE, strName.c_str());
	}

	return TRUE;
}

void CDlgBatchProcessEmbeddedLyrics::OnOK()
{
	CMutexAutolock	autolock(g_autoProcessEmbeddedLyrics.m_mutex);
	int		nRet, n = 0, nFailedTimes = 0;
	CAutoProcessEmbeddedLyrics::LIST_ITEMS::iterator	it, itEnd;

	itEnd = g_autoProcessEmbeddedLyrics.m_listJobs.end();
	for (it = g_autoProcessEmbeddedLyrics.m_listJobs.begin(); it != itEnd; ++it)
	{
		CAutoProcessEmbeddedLyrics::Item	&item = *it;
		if (!item.m_bDealOK)
		{
			nRet = g_autoProcessEmbeddedLyrics.DealEmbeddedLyrics(item);
			if (nRet != ERR_OK)
			{
				m_listCtrl.SetItemText(n, COL_RESULT, ERROR2STR_LOCAL(nRet));
				nFailedTimes++;
			}
			else
				item.m_bDealOK = true;
		}
		n++;
	}

	if (nFailedTimes > 0)
		MessageOut(CStrPrintf(_T("%d Lyrics were failed to process."), nFailedTimes).c_str());
	else
		CBaseDialog::OnOK();
}
