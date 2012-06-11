#include "stdafx.h"
#include "MPlayerAppBase.h"
#include "SkinListCtrlEx.h"
#include "../Skin/SkinEditCtrl.h"
#include "MPlaylistSearchObj.h"
#include "PlayListFile.h"


#define SZ_RECENT_PL			_T("RecentPL")
#define COUNT_RECENT_PL_MAX		10


void SplitKeywords(LPCTSTR szKeywords, V_Str &vKeywords)
{
	LPCTSTR		pStart, p;
	tstring		str;

	vKeywords.clear();

	pStart = p = szKeywords;
	while (*p)
	{
		while (IsAlpha(*p) || IsDigit(*p) || (unsigned int)(*p) >= 128)
			p++;

		if (p == pStart)
			p++;

		if (*pStart != ' ' && *pStart != '\t')
		{
			vKeywords.push_back(tstring());
			vKeywords.back().append(pStart, p);
		}

		pStart = p;
	}
}


static uint32 SearchWithKeywordEx(V_Str &vKeywords, LPCTSTR szText, LC_ITEMEX::VecTextColor &vItemClrs)
{
	LPCTSTR		pStart, p, pLastUnmatch;
	tstring		str;
	V_Str		vLeftKeywords = vKeywords;

	pLastUnmatch = pStart = p = szText;
	while (*p)
	{
		while (IsAlpha(*p) || IsDigit(*p))
			p++;

		if (p == pStart)
			p++;

		if (*pStart != ' ' && *pStart != '\t')
		{
			for (V_Str::size_type i = 0; i < vLeftKeywords.size(); i++)
			{
				if (int(p - pStart) == vLeftKeywords[i].size()
					&& _tcsnicmp(pStart, vLeftKeywords[i].c_str(), vLeftKeywords[i].size()) == 0)
				{
					vLeftKeywords.erase(vLeftKeywords.begin() + i);

					// Set unmatched color
					if (pLastUnmatch != pStart)
						vItemClrs.Add(int(pStart - pLastUnmatch), CSkinListCtrlEx::CN_TEXT);

					// Set matched color
					vItemClrs.Add(int(p - pStart), CSkinListCtrlEx::CN_CUSTOMIZED_START);

					pLastUnmatch = p;
					break;
				}
			}
			if (vLeftKeywords.empty())
				return 1;
		}

		pStart = p;
	}

	if (vLeftKeywords.empty())
		return 1;

	return 0;
}


static uint32 SearchWithKeyword(LPCTSTR szKeyword, LPCTSTR szText, LC_ITEMEX::VecTextColor &vItemClrs)
{
	LPCTSTR		pKey, pDst, pDstStart;

	vItemClrs.clear();

	pDst = szText;

	while (*pDst)
	{
		pDstStart = pDst;
		pKey = szKeyword;

		while (*pDst && CharLwr(*pKey) == CharLwr(*pDst))
		{
			pKey++;
			pDst++;
		}

		if (*pKey == 0)
		{
			if (int(pDstStart - szText) > 0)
				vItemClrs.Add(int(pDstStart - szText), CSkinListCtrlEx::CN_TEXT);
			vItemClrs.Add(int(pDst - pDstStart), CSkinListCtrlEx::CN_CUSTOMIZED_START);
			return uint32(-1);
		}

		if (*pDst)
			pDst++;
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CMPlaylistSearchObj, _T("PlaylistSearchObj"))

CMPlaylistSearchObj::CMPlaylistSearchObj()
{
	m_pCtrlEdit = NULL;
	m_pCtrlPlaylist = NULL;
	m_pCtrlPlaylistList = NULL;
	m_nIDBeginSearchTimer = 0;
}


CMPlaylistSearchObj::~CMPlaylistSearchObj()
{
	m_pSkin->UnregisterUIObjNotifyHandler(this);
}


void CMPlaylistSearchObj::OnCreate()
{
	CSkinDataObj::OnCreate();

	m_pCtrlEdit = (CSkinEditCtrl *)m_pSkin->GetUIObjectById(
		m_pSkin->GetSkinFactory()->GetIDByName(m_strIDEditor.c_str()),
		CSkinEditCtrl::ClassName());
	if (m_pCtrlEdit)
		m_pCtrlEdit->SetEditNotification(this);

	m_pCtrlPlaylist = (CSkinListCtrlEx *)m_pSkin->GetUIObjectById(
		m_pSkin->GetSkinFactory()->GetIDByName(m_strIDPlaylist.c_str()),
		CSkinListCtrlEx::ClassName());

	m_pCtrlPlaylistList = (CSkinListCtrl *)m_pSkin->GetUIObjectById(
		m_pSkin->GetSkinFactory()->GetIDByName(m_strIDPlaylistList.c_str()),
		CSkinListCtrl::ClassName());

//	MLColor		clrKeyword(RGB(0, 128, 255));
//	m_pCtrlPlaylist->SetColor(CSkinListCtrl::CN_CUSTOMIZED_START, clrKeyword);

	if (!m_pCtrlEdit || !m_pCtrlPlaylist || !m_pCtrlPlaylistList)
		return;

	m_pSkin->RegisterUIObjNotifyHandler(m_pCtrlPlaylistList->GetID(), this);
	m_pSkin->RegisterUIObjNotifyHandler(m_pCtrlPlaylist->GetID(), this);

	m_pCtrlPlaylistList->AddColumn(_T("Playlist"), 200);
	m_pCtrlPlaylist->AddColumn(_T("Results"), 200);

	m_pCtrlPlaylistList->InsertItem(PN_NOWPLAYING, _TLT("Now Playing"));
	m_vRecentPL.push_back(_T("NowPlaying"));

	m_pCtrlPlaylistList->InsertItem(PN_ALL_MEDIALIB, _TLT("Entire Media Library"));
	m_vRecentPL.push_back(_T("Entire Media Library"));

	// Load recent playlist...
	for (int i = PN_CUSTOMIZED; i < COUNT_RECENT_PL_MAX; i++)
	{
		tstring			strPlaylist;

		strPlaylist = g_Profile.GetProfileString(CStrPrintf(_T("%s%d"), SZ_RECENT_PL, i).c_str(), _T(""));
		if (strPlaylist.empty())
			break;
		if (!IsFileExist(strPlaylist.c_str()))
			continue;

		AddInRecentPlaylist(FileGetTitle(strPlaylist.c_str()).c_str(), strPlaylist.c_str(), -1);
	}
	m_pCtrlPlaylistList->Invalidate();
	m_pCtrlPlaylistList->SetItemSelectionState(0, true);
}


void CMPlaylistSearchObj::OnTimer(int nId)
{
	tstring			strKeyword;

	if (m_nIDBeginSearchTimer)
	{
		m_pSkin->UnregisterTimerObject(this, m_nIDBeginSearchTimer);
		m_nIDBeginSearchTimer =  0;
	}

	if (!m_pCtrlEdit || !m_pCtrlPlaylist)
		return;

	strKeyword = m_pCtrlEdit->GetText();

	TrimStr(strKeyword);
	LowerString((LPTSTR)strKeyword.c_str());
	DoSearch(strKeyword.c_str());
}


void CMPlaylistSearchObj::DoSearch(LPCTSTR szKeyword)
{
	MLRESULT	nRet;
	tstring		str;

	// clear old results
	if (m_PlaylistResults)
		m_PlaylistResults.Release();
	m_pCtrlPlaylist->DeleteAllItems(false);

	ASSERT(m_Playlist);
	if (!m_Playlist)
		g_Player.GetCurrentPlaylist(&m_Playlist);

	if (IsEmptyString(szKeyword))
	{
		m_PlaylistResults = m_Playlist;
		UpdatePlaylist();

		return;
	}

	if (g_Player.NewPlaylist(&m_PlaylistResults) != ERR_OK)
		return;

	//
	// Search playlist with keyword: szKeyword
	//

	LC_ITEMEX::VecTextColor		vItemClrs;
	uint32			nMatchValue;
	V_Str			vKeywords;

	SplitKeywords(szKeyword, vKeywords);

	for (uint32 i = 0; i < m_Playlist->GetCount(); i++)
	{
		CMPAutoPtr<IMedia>	media;
		nRet = m_Playlist->GetItem(i, &media);
		if (nRet == ERR_OK)
		{
			CPlayer::FormatMediaTitle(media, str);

			nMatchValue = SearchWithKeyword(szKeyword, str.c_str(), vItemClrs);
			if (nMatchValue > 0)
			{
				m_pCtrlPlaylist->InsertItem(-1, str.c_str(), vItemClrs, 0, 0, false);
				m_PlaylistResults->InsertItem(-1, media);
			}
			else
			{
				nMatchValue = SearchWithKeywordEx(vKeywords, str.c_str(), vItemClrs);
				if (nMatchValue > 0)
				{
					m_pCtrlPlaylist->InsertItem(-1, str.c_str(), vItemClrs, 0, 0, false);
					m_PlaylistResults->InsertItem(-1, media);
				}
			}
		}
	}

	m_pCtrlPlaylist->Invalidate();
}


BOOL CMPlaylistSearchObj::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CSkinDataObj::SetProperty(szProperty, szValue))
		return TRUE;

	if (IsPropertyName(szProperty, _T("KeywordEdit")))
		m_strIDEditor = szValue;
	else if (IsPropertyName(szProperty, _T("Playlist")))
		m_strIDPlaylist = szValue;
	else if (IsPropertyName(szProperty, _T("PlaylistList")))
		m_strIDPlaylistList = szValue;
	else
		return FALSE;

	return TRUE;
}

#ifdef _SKIN_EDITOR_
void CMPlaylistSearchObj::EnumProperties(CUIObjProperties &listProperties)
{
	CSkinDataObj::EnumProperties(listProperties);

}
#endif // _SKIN_EDITOR_


void CMPlaylistSearchObj::OnTextChanged()
{
	ASSERT(m_pCtrlEdit);
	if (!m_pCtrlEdit)
		return;

	if (m_nIDBeginSearchTimer != 0)
		m_pSkin->UnregisterTimerObject(this, m_nIDBeginSearchTimer);

	m_nIDBeginSearchTimer = m_pSkin->RegisterTimerObject(this, TD_BEGIN_SEARCH);
}


void CMPlaylistSearchObj::OnSpecialKey(SpecialKey key)
{
	if (!m_pCtrlEdit || !m_pCtrlPlaylist || !m_pCtrlPlaylistList)
		return;

	if (key == IEditNotification::SK_ENTER)
		UseResultAsNowPlaying();
}


void CMPlaylistSearchObj::OnUIObjNotify(IUIObjNotify *pNotify)
{
	if (!m_pCtrlEdit || !m_pCtrlPlaylist || !m_pCtrlPlaylistList)
		return;

	if (pNotify->nID == m_pCtrlPlaylistList->GetID())
	{
		//
		// Process Playlist List command
		//
		if (pNotify->IsKindOf(CSkinListCtrl::ClassName()))
		{
			CSkinListCtrlEventNotify	*pSkinListCtrlNotify = (CSkinListCtrlEventNotify *)pNotify;

			int nSel = m_pCtrlPlaylistList->GetNextSelectedItem();
			if (nSel < 0 || nSel >= (int)m_vRecentPL.size())
				return;

			switch (pSkinListCtrlNotify->cmd)
			{
			case CSkinListCtrlEventNotify::C_SEL_CHANGED:
			case CSkinListCtrlEventNotify::C_DBL_CLICK:
			case CSkinListCtrlEventNotify::C_ENTER:
				{
					// Choose as current search source.
					if (m_Playlist)
						m_Playlist.Release();
					if (nSel == PN_NOWPLAYING)
						g_Player.GetCurrentPlaylist(&m_Playlist);
					else if (nSel == PN_ALL_MEDIALIB)
					{
						CMPAutoPtr<IMediaLibrary>		mediaLib;
						if (g_Player.GetMediaLibrary(&mediaLib) == ERR_OK)
							mediaLib->GetAll(&m_Playlist, MLOB_NONE, -1);
					}
					else
					{
						if (g_Player.NewPlaylist(&m_Playlist) != ERR_OK)
							return;
						::LoadPlaylist(g_Player.GetIMPlayer(), m_Playlist, m_vRecentPL[nSel].c_str());
					}
					m_PlaylistResults = m_Playlist;
					if (pSkinListCtrlNotify->cmd != CSkinListCtrlEventNotify::C_SEL_CHANGED)
					{
						// Open selected playlist
						g_Player.SetCurrentPlaylist(m_Playlist);
						g_Player.Play();
					}
					UpdatePlaylist();
				}
				break;
			case CSkinListCtrlEventNotify::C_KEY_DELETE:
				// Erase recent playlist
				if (nSel != 0)
				{
					m_vRecentPL.erase(m_vRecentPL.begin() + nSel);
					m_pCtrlPlaylistList->DeleteItem(nSel);
				}
				break;
			default:
				break;
			}
		}
	}
	else if (pNotify->nID == m_pCtrlPlaylist->GetID())
	{
		//
		// Process Playlist command
		//
		if (pNotify->IsKindOf(CSkinListCtrl::ClassName()))
		{
			CSkinListCtrlEventNotify	*pSkinListCtrlNotify = (CSkinListCtrlEventNotify *)pNotify;

			int nSel = m_pCtrlPlaylist->GetNextSelectedItem();
			if (nSel == -1)
				return;

			switch (pSkinListCtrlNotify->cmd)
			{
			case CSkinListCtrlEventNotify::C_DBL_CLICK:
			case CSkinListCtrlEventNotify::C_ENTER:
				{
					// Use current result as now playing
					UseResultAsNowPlaying();
				}
				break;
			case CSkinListCtrlEventNotify::C_KEY_DELETE:
				{
					// Erase selected items in result list
					vector<int>		vSelItems;
					int				nItem = -1, count;

					while ((nItem = m_pCtrlPlaylist->GetNextSelectedItem(nItem)) != -1)
						vSelItems.push_back(nItem);

					// sort selected items descending
					sort(vSelItems.begin(), vSelItems.end());

					count = m_PlaylistResults->GetCount();
					if (count == 0)
						return;

					// remove selected items descending
					for (int n = 0; n < (long)vSelItems.size(); n++)
					{
						nItem = vSelItems[n];
						ASSERT(nItem < count && nItem >= 0);
						if (m_PlaylistResults->RemoveItem(nItem)!= ERR_OK)
							break;
						m_pCtrlPlaylist->DeleteItem(nItem);
					}
				}
				break;
			default:
				break;
			}
		}
	}
}


void CMPlaylistSearchObj::UpdatePlaylist()
{
	ASSERT(m_pCtrlPlaylist);

	m_pCtrlPlaylist->DeleteAllItems(false);

	for (uint32 i = 0; i < m_PlaylistResults->GetCount(); i++)
	{
		CMPAutoPtr<IMedia>	media;
		tstring				str;
		if (m_PlaylistResults->GetItem(i, &media) == ERR_OK)
		{
			CPlayer::FormatMediaTitle(media, str);
			m_pCtrlPlaylist->InsertItem(-1, str.c_str(), 0, 0, false);
		}
	}

	m_pCtrlPlaylist->Invalidate();
}

void CMPlaylistSearchObj::AddInRecentPlaylist(LPCTSTR szName, LPCTSTR szFile, int nPos)
{
	V_Str::size_type i;

	for (i = PN_CUSTOMIZED; i < m_vRecentPL.size(); i++)
	{
		tstring			str;
		m_pCtrlPlaylistList->GetItemText(i, 0, str);
		if (_tcsicmp(str.c_str(), szName) == 0)
		{
			m_vRecentPL[i] = szFile;
			m_pCtrlPlaylistList->SetItemText(i, 0, szName);
			return;
		}
	}

	if (i == m_vRecentPL.size())
	{
		if (nPos == -1)
		{
			m_vRecentPL.push_back(szFile);
			m_pCtrlPlaylistList->InsertItem(nPos, szName);
		}
		else
		{
			m_vRecentPL.insert(m_vRecentPL.begin() + nPos, szFile);
			m_pCtrlPlaylistList->InsertItem(nPos, szName);
		}
	}
}

void CMPlaylistSearchObj::UseResultAsNowPlaying()
{
	ASSERT(m_pCtrlPlaylist);

	if (!m_pCtrlPlaylist || !m_PlaylistResults || m_PlaylistResults->GetCount() == 0)
		return;

	tstring			strKeyword;
	tstring			strPlaylistFile;
	int				i;
	int				nSel;

	nSel = m_pCtrlPlaylist->GetNextSelectedItem();
	if (nSel == -1)
		nSel = 0;

	g_Player.SetCurrentPlaylist(m_PlaylistResults);
	g_Player.SetCurrentMediaInPlaylist(nSel);
	g_Player.Play();

	strKeyword = m_pCtrlEdit->GetText();
	TrimStr(strKeyword);

	strPlaylistFile = GetAppDataDir();
	strPlaylistFile += FileNameFilterInvalidChars(strKeyword.c_str());
	strPlaylistFile += _T(".m3u");
	SavePlaylistAsM3u(m_PlaylistResults, strPlaylistFile.c_str());

	// insert in PL list or replace it
	AddInRecentPlaylist(strKeyword.c_str(), strPlaylistFile.c_str(), PN_CUSTOMIZED);

	// Only show about COUNT_RECENT_PL_MAX results
	if (m_vRecentPL.size() > COUNT_RECENT_PL_MAX)
		m_vRecentPL.resize(COUNT_RECENT_PL_MAX);
	for (i = COUNT_RECENT_PL_MAX; i < m_pCtrlPlaylistList->GetItemCount(); i++)
		m_pCtrlPlaylistList->DeleteItem(i, true);
	m_pCtrlPlaylistList->Invalidate();

	// Save recent playlist...
	for (i = PN_CUSTOMIZED; i < (int)m_vRecentPL.size(); i++)
		g_Profile.WriteProfileString(CStrPrintf(_T("%s%d"), SZ_RECENT_PL, i).c_str(), m_vRecentPL[i].c_str());
}

#ifdef _CPPUNIT_TEST

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

IMPLEMENT_CPPUNIT_TEST_REG(MPlaylistSearchObj)

class CTestCasePlaylistSearchObj : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(CTestCasePlaylistSearchObj);
	CPPUNIT_TEST(TestSplitKeywords);
	CPPUNIT_TEST_SUITE_END();

protected:

public:
	void setUp()
	{
	}
	void tearDown() 
	{
	}

protected:
	void TestSplitKeywords()
	{
		{
			LPCTSTR	szKeyWords = _T("");
			LPCTSTR	vOutput[] = { _T("") };
			VerifySplitResult(szKeyWords, vOutput, 0);
		}

		{
			LPCTSTR	szKeyWords = _T("a");
			LPCTSTR	vOutput[] = { _T("a") };
			VerifySplitResult(szKeyWords, vOutput, CountOf(vOutput));
		}

		{
			LPCTSTR	szKeyWords = _T("abcdefg");
			LPCTSTR	vOutput[] = { _T("abcdefg") };
			VerifySplitResult(szKeyWords, vOutput, CountOf(vOutput));
		}

		{
			LPCTSTR	szKeyWords = _T("a, b  cc;d-!ee?ff");
			LPCTSTR	vOutput[] = { _T("a"), _T(","), _T("b"), _T("cc"), _T(";"), _T("d"), _T("-"), 
				_T("!"), _T("ee"), _T("?"), _T("ff") };
			VerifySplitResult(szKeyWords, vOutput, CountOf(vOutput));
		}
	}

	void VerifySplitResult(LPCTSTR szKeywords, LPCTSTR vOutput[], int nCountOutput)
	{
		V_Str	vKeywords;

		SplitKeywords(szKeywords, vKeywords);
		if (nCountOutput == vKeywords.size())
		{
			for (int i = 0; i < nCountOutput; i++)
			{
				if (_tcscmp(vOutput[i], vKeywords[i].c_str()) != 0)
					CPPUNIT_FAIL_T(CStrPrintf(_T("SplitKeywords:%s Failed, %s should be %s, Round: %d"), szKeywords, vKeywords[i].c_str(), vOutput[i], i).c_str());
			}
		}
		else
			CPPUNIT_FAIL_T(CStrPrintf(_T("SplitKeywords:%s Failed, Output Count not match."), szKeywords).c_str());
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCasePlaylistSearchObj);


#endif // _CPPUNIT_TEST
