// DlgSearchLyrics.cpp: implementation of the CDlgSearchLyrics class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "DownloadMgr.h"
#include "DlgSearchLyrics.h"
#include "DlgAbout.h"
#include "OnlineSearch.h"
#include "WaitingDlg.h"
#include "Helper.h"
#include "ErrorResolveDlg.h"
#include "AutoProcessEmbeddedLyrics.h"

bool IsHttpUrl(LPCTSTR szUrl)
{
	LPCTSTR		SZ_HTTP_HEADER = _T("http://");

	return _tcsnicmp(szUrl, SZ_HTTP_HEADER, _tcslen(SZ_HTTP_HEADER)) == 0;
}

// szValue format: %d,%d
template<class TCHARDEF, class _int_t>
bool ScanNIntX(TCHARDEF szValue, _int_t arr[], size_t nCount)
{
	if (IsEmptyString(szValue))
		return false;

	size_t	i;
	for (i = 0; i < nCount; i++)
	{
		szValue = ReadInt_t(szValue, arr[i]);
		if (*szValue != ',')
			break;
		szValue++;
		while (*szValue == ' ')
			szValue++;
	}

 	return i + 1 >= nCount;
}

bool ProfileLoadIntArray(LPCTSTR szValueName, int vArr[], int nCount)
{
	return ScanNIntX(g_Profile.GetProfileString(szValueName, _T("")), vArr, nCount);
}

void ProfileSaveSkinListCtrlColumnWidth(LPCTSTR szValueName, CSkinListCtrl *pListCtrl)
{
	tstring		strValue;
	CStrPrintf	str;

	for (int i = 0; i < pListCtrl->GetColumnCount(); i++)
	{
		str.printf(_T("%d"), pListCtrl->GetColumnWidth(i));
		if (i > 0)
			strValue += _T(",");
		strValue += str.c_str();
	}

	g_Profile.WriteProfileString(szValueName, strValue.c_str());
}

void UpdateLyrResultArtistTitle(V_LRCSEARCHRESULT &vResultsLocal)
{
	V_LRCSEARCHRESULT::iterator	it;

	for (it = vResultsLocal.begin(); it != vResultsLocal.end(); ++it)
	{
		LrcSearchResult		&result = *it;

		if (result.strArtist.empty() && result.strTitle.empty())
		{
			AnalysisLyricsFileNameEx(result.strArtist, result.strTitle, result.strSaveFileName.c_str());
		}
	}
}

/*
struct CLyricsResultSortCmp
{
public:
	bool operator()(const LrcSearchResult &item1, const LrcSearchResult &item2)
	{
		return _tcsicmp(item1.strTitle.c_str(), item2.strTitle.c_str()) < 0;
	}
};

template<class _Cont, class _It, class _Pr>
inline void sort_insert(_Cont &_Container, _It _First, _It _Last, _Pr _Pred)
{
	for (; _First != _Last; ++_First)
	{
		_Cont::iterator it;

		for (it = _Container.begin(); it != _Container.end(); ++it)
		{
			if (!_Pred(*it, *_First))
			{
				_Container.insert(it, *_First);
				break;
			}
		}
		if (it == _Container.end())
			_Container.push_back(*_First);
	}
}*/

class CPageRename : public CSkinPageHandler
{
public:
	CPageRename() : CSkinPageHandler(_T("CID_C_RENAME"), 
		NULL, _T("CID_AM_SHOW_RENAME"),
		NULL, _T("CID_AM_HIDE_RENAME"))
	{
	}

	void OnShow()
	{
		CSkinPageHandler::OnShow();

		m_pSkinWnd->SetCaptionText(TOLOCAL(_T("Rename lyrics file")));

		SetUIObjectText(_T("CID_E_FILENAME"), m_pParent->GetProperty(_T("FileTitle")).c_str(), false);
		m_pParent->SetProperty(_T("RenameDlgResult"), IDCANCEL);
	}

	void OnOK()
	{
		m_pParent->SetProperty(_T("FileTitle"), GetUIObjectText(_T("CID_E_FILENAME")).c_str());
		m_pParent->SetProperty(_T("RenameDlgResult"), IDOK);
		m_pParent->SwitchToLastPage();
	}

	void OnCancel()
	{
		m_pParent->SwitchToLastPage();
	}

};

//////////////////////////////////////////////////////////////////////

class CSearchWorkObj : public CWorkObjBase
{
public:
	CSearchWorkObj()
	{
		m_nCurPage = 0;
		m_nPageCount = 1;
		m_bEnableCancel = true;
	}

	virtual DWORD DoTheJob()
	{
		m_pvSearchResult->clear();

		m_strMsg = _T("");

		m_nResult = g_OnlineSearch.SearchOnline(m_session, m_strArtist.c_str(), m_strTitle.c_str(), *m_pvSearchResult, m_strMsg, m_nCurPage, m_nPageCount);

		return CWorkObjBase::DoTheJob();
	}

	virtual void CancelJob()
	{
		m_session.Close();

		CWorkObjBase::CancelJob();
	}

public:
	V_LRCSEARCHRESULT		*m_pvSearchResult;
	tstring					m_strMsg;
	int						m_nCurPage, m_nPageCount;
	int						m_nResult;
	CMLClientSession		m_session;

	tstring					m_strArtist, m_strTitle;

};

//////////////////////////////////////////////////////////////////////////

class CPageSearchLyrics : public CSkinPageHandler, public IUIObjNotifyHandler, public IEditNotification
{
public:
	enum { COL_ARTIST, COL_TITLE, COL_RATING, COL_DOWNLOADS, COL_FROM, COL_ALBUM, COL_URL};
	enum
	{
		IMG_NONE = 0,

		// Local
		IMG_L_TXT,
		IMG_L_LRC,
		IMG_L_UNKNOWN,

		// Inet
		IMG_I_TXT,
		IMG_I_LRC,
		IMG_I_UNKNOWN,

		IMG_OFFSET_INET = IMG_I_UNKNOWN - IMG_L_UNKNOWN,
	};
	enum {
		TIMER_ID_RESEARCH = 1,
		DURATION_AUTO_SEARCH	= 1000,
	};

	CPageSearchLyrics() : CSkinPageHandler(_T("CID_C_OPEN"), 
		_T("CID_AM_SHOW_OPEN"), _T("CID_AM_SHOW_OPEN"),
		_T("CID_AM_HIDE_OPEN"), NULL)
	{
		m_bAutoSearchLocal = true;
		m_nCurPage = 0;
		m_nPageCount = 0;
		EmptyStr(m_szAssociateKeyword);

		m_pLyricsList = NULL;
		m_pSearchObj = NULL;

		CID_PREV_PAGE = 0;
		CID_NEXT_PAGE = 0;
		CID_OPEN = 0;
		CID_GOOGLE = 0;
		CID_MORE = 0;
		CID_CANCEL = 0;
		CID_E_ARTIST = 0;
		CID_E_TITLE = 0;
		CID_SEARCH = 0;
		CID_L_LYRICS = 0;
	}

	~CPageSearchLyrics()
	{
		if (m_pSearchObj)
		{
			m_pSearchObj->CancelJob();
			delete m_pSearchObj;
		}
	}

	// SkinListControl notification
	void OnUIObjNotify(IUIObjNotify *pNotify)
	{
		if (pNotify->IsKindOf(CSkinListCtrl::ClassName()))
		{
			CSkinListCtrlEventNotify	*pListCtrlNotify = (CSkinListCtrlEventNotify*)pNotify;
			switch (pListCtrlNotify->cmd)
			{
			case CSkinListCtrlEventNotify::C_DBL_CLICK:
			case CSkinListCtrlEventNotify::C_ENTER:
				OnOK();
				break;
			}
		}
	}

	// Edit control notification
	void OnTextChanged()
	{
		if (m_bAutoSearchLocal)
		{
			m_pSkinWnd->KillTimer(TIMER_ID_RESEARCH);
			m_pSkinWnd->SetTimer(TIMER_ID_RESEARCH, DURATION_AUTO_SEARCH);
		}
	}

	// Edit control notification
	void OnSpecialKey(SpecialKey key)
	{
		if (key == SK_ENTER)
			OnSearch();
	}

	void OnCreate()
	{
		CSkinPageHandler::OnCreate();

		GET_ID_BY_NAME(CID_PREV_PAGE);
		GET_ID_BY_NAME(CID_NEXT_PAGE);
		GET_ID_BY_NAME(CID_OPEN);
		GET_ID_BY_NAME(CID_GOOGLE);
		GET_ID_BY_NAME(CID_MORE);
		GET_ID_BY_NAME(CID_CANCEL);
		GET_ID_BY_NAME(CID_E_ARTIST);
		GET_ID_BY_NAME(CID_E_TITLE);
		GET_ID_BY_NAME(CID_SEARCH);
		GET_ID_BY_NAME(CID_L_LYRICS);

		// Save current media info.
		m_strSongFile = g_Player.GetSrcMedia();
		m_strArtist = g_Player.GetArtist();
		m_strTitle = g_Player.GetTitle();
		CMPlayerAppBase::GetAssociateFileKeyword(m_szAssociateKeyword, CountOf(m_szAssociateKeyword));

		m_pLyricsList = (CSkinListCtrl *)m_pSkinWnd->GetUIObjectById(CID_L_LYRICS, CSkinListCtrl::ClassName());
		ASSERT(m_pLyricsList);
		if (!m_pLyricsList)
			return;

		SetUIObjectText(CID_E_ARTIST, m_strArtist.c_str(), false);
		SetUIObjectText(CID_E_TITLE, m_strTitle.c_str(), false);

		LPCTSTR		szHead[] = {_T("Artist"), _T("Title"), _T("Rating"), _T("Downloads"), _T("From"), _T("Album"), NULL};
		int			arrWidth[] = {130,        160,         90,           68,              150,       150,         0};

		ProfileLoadIntArray(_T("OpenLyrics_ListColWidth"), arrWidth, CountOf(arrWidth));
		ASSERT(CountOf(arrWidth) == CountOf(szHead));
		for (int i = 0; szHead[i] != NULL; i++)
			m_pLyricsList->AddColumn(TOLOCAL(szHead[i]), arrWidth[i]);
		if (g_Profile.GetProfileInt(_T("ShowLink"), false))
			m_pLyricsList->AddColumn(_T("URL"), 400);

		m_menu.LoadPopupMenu(IDR_MENU_OPEN_LRC_DLG, 0);
		ToLocalMenu(&m_menu);

		g_OnlineSearch.SearchCacheForCur(&m_vResultsInet);
		m_vResultsInet.Sort();

		SearchLocal();

		UpdateSearchResultToListCtrl();

		m_pSkinWnd->RegisterUIObjNotifyHandler(CID_L_LYRICS, this);

		CSkinEditCtrl *pEdit = (CSkinEditCtrl*)m_pSkinWnd->GetUIObjectById(CID_E_ARTIST, CSkinEditCtrl::ClassName());
		if (pEdit)
		{
			pEdit->SetEditNotification(this);
			pEdit = (CSkinEditCtrl*)m_pSkinWnd->GetUIObjectById(CID_E_TITLE, CSkinEditCtrl::ClassName());
			pEdit->SetEditNotification(this);
		}
	}

	void OnDestroy()
	{
		m_pSkinWnd->UnregisterUIObjNotifyHandler(this);
		ProfileSaveSkinListCtrlColumnWidth(_T("OpenLyrics_ListColWidth"), m_pLyricsList);
	}

	void OnOK()
	{
		if (m_pLyricsList->GetSelectedCount() == 0)
			return;

		LrcSearchResult	*pSel;

		pSel = GetCurSel();
		if (!pSel)
			return;

		if (IsHttpUrl(pSel->strUrl.c_str()))
		{
			// Download selected lyrics
			g_LyricsDownloader.DownloadLyrics(m_szAssociateKeyword, 
				m_strSongFile.c_str(), pSel->strUrl.c_str(), pSel->strSaveFileName.c_str());
		}
		else
		{
			// Associate lyrics file
			if (g_LyricSearch.AssociateLyrics(m_szAssociateKeyword, pSel->strUrl.c_str()))
			{
				CMPlayerAppBase::GetInstance()->DispatchResearchLyrics();
			}
		}

		CSkinPageHandler::OnOK();
	}

	void OnShow()
	{
		CSkinPageHandler::OnShow();

		CSkinWnd::CAutoRedrawLock	redrawLock(m_pSkinWnd);

		m_pSkinWnd->SetCaptionText(TOLOCAL(_T("Search lyrics")));

		if (m_pSearchObj)
		{
			OnOnlineSearchEnd();
			delete m_pSearchObj;
			m_pSearchObj = NULL;
		}
		else if (m_pParent->GetPropertyInt(_T("RenameDlgResult")) == IDOK)
		{
			//
			// Rename selected lyrics file
			//
			LrcSearchResult	*pSel;

			pSel = GetCurSel();
			if (!pSel)
				return;

			if (GetLrcSourceType(pSel->strUrl.c_str()) != LST_FILE)
				return;

			TCHAR			szFileNew[MAX_PATH];

			tstring		strFileTitleOld;
			tstring		strFileTitle = m_pParent->GetProperty(_T("FileTitle"));
			FileGetTitle(pSel->strUrl.c_str(), strFileTitleOld);

			// Same?
			if (_tcscmp(strFileTitle.c_str(), strFileTitleOld.c_str()) == 0)
				return;

			FileGetPath(szFileNew, CountOf(szFileNew), pSel->strUrl.c_str());
			_tcscat_safe(szFileNew, CountOf(szFileNew), strFileTitle.c_str());
			_tcscat_safe(szFileNew, CountOf(szFileNew), FileGetExt(pSel->strUrl.c_str()));
			if (!MoveFile(pSel->strUrl.c_str(), szFileNew))
			{
				m_pSkinWnd->MessageOut(TOLOCAL(_T("Failed to rename lyrics file.")));
			}
			else
			{
				int			nSel = m_pLyricsList->GetNextSelectedItem(-1);

				m_pLyricsList->SetItemText(nSel, COL_FROM, szFileNew);
			}
		}

		redrawLock.CancelDrawUpdate();
	}

	bool OnTimer(UINT nIDEvent)
	{
		if (nIDEvent != TIMER_ID_RESEARCH)
			return false;

		m_pSkinWnd->KillTimer(TIMER_ID_RESEARCH);

		tstring strArtist = GetUIObjectText(CID_E_ARTIST);
		tstring strTitle = GetUIObjectText(CID_E_TITLE);

		// Not changed?
		if (_tcsicmp(strArtist.c_str(), m_strArtist.c_str()) == 0
			&& _tcsicmp(strTitle.c_str(), m_strTitle.c_str()) == 0
			|| IsEmptyString(strTitle.c_str()))
			return true;

		m_strArtist = strArtist;
		m_strTitle = strTitle;

		SearchLocal();

		UpdateSearchResultToListCtrl();

		return true;
	}

	bool OnCommand(int nId)
	{
		switch (nId)
		{
		case IDC_NOT_MATCH:
			{
				if (IsEmptyString(m_szAssociateKeyword))
					break;

				g_LyricSearch.AssociateLyrics(m_szAssociateKeyword, NONE_LYRCS);
				CMPlayerAppBase::GetInstance()->DispatchResearchLyrics();
				PostDestroy();
			}
			break;
		case IDC_NO_PROMPT:
			// Don't prompt this dialog automatically.
			g_Profile.WriteProfileInt(SZ_SECT_LYR_DL, _T("DownLrcUserSelect"), 
				!g_Profile.GetProfileBool(SZ_SECT_LYR_DL, _T("DownLrcUserSelect"), false));
			break;
		case IDC_RENAME:
			{
				//
				// Rename selected lyrics file
				//
				LrcSearchResult	*pSel;

				pSel = GetCurSel();
				if (!pSel)
					break;

				if (GetLrcSourceType(pSel->strUrl.c_str()) != LST_FILE)
					break;

				tstring		strTitle;
				FileGetTitle(pSel->strUrl.c_str(), strTitle);

				m_pParent->SetProperty(_T("FileTitle"), strTitle.c_str());
				m_pParent->SwitchToPage(_T("CID_C_RENAME"));
			}
			break;
		case IDC_DEL_FILE:
			{
				LrcSearchResult	*pSel;

				pSel = GetCurSel();
				if (!pSel)
					break;

				LRC_SOURCE_TYPE lst = GetLrcSourceType(pSel->strUrl.c_str());
				bool bDeleted = false;
				if (lst == LST_FILE)
				{
					SHDeleteFile(pSel->strUrl.c_str(), m_pSkinWnd);
					if (!IsFileExist(pSel->strUrl.c_str()))
						bDeleted = true;
				}
				else
				{
					vector<string> vSyncLang, vUnsyncLang;
					int nRet = g_autoProcessEmbeddedLyrics.RemoveEmbeddedLyrics(m_strSongFile.c_str(), vSyncLang, vUnsyncLang, lst, true);
					if (nRet == ERR_OK)
						bDeleted = true;
					else
						m_pSkinWnd->MessageOut(ERROR2STR_LOCAL(nRet));
				}

				if (bDeleted)
				{
					int		nSel = m_pLyricsList->GetNextSelectedItem(-1);
					m_vResultsLocal.DeleteByOrder(nSel);
					m_pLyricsList->DeleteItem(nSel);
				}
			}
			break;
		case IDC_EXTERNAL_LYR_EDIT:
			//
			// Edit lyrics
			//
			{
				LrcSearchResult	*pSel;

				pSel = GetCurSel();
				if (!pSel)
					break;

				if (GetLrcSourceType(pSel->strUrl.c_str()) != LST_FILE)
					break;

				tstring		strEditor;
				GetNotepadEditor(strEditor);
				Execute(m_pSkinWnd,
					CMLProfile::GetDir(SZ_SECT_UI, _T("LyricsEditor"), strEditor.c_str()).c_str(),
					pSel->strUrl.c_str());
			}
			break;
		case IDC_BROWSE:
			{
				//
				// Browse lyrics file and associate.
				//
				LrcSearchResult	*pSel;
				tstring			strFile;

				pSel = GetCurSel();
				if (pSel)
				{
					if (GetLrcSourceType(pSel->strUrl.c_str()) != LST_FILE)
						strFile = pSel->strUrl;
				}

				static const TCHAR *SZ_SUPPORTED_FILE_TYPE		 = _T("All supported files (*.lrc; *.txt; *.srt)\0*.lrc;*.txt;*.srt\0Lyric File (*.lrc)\0*.LRC\0Text File (*.txt)\0*.txt\0Vob Subtitle File(*.srt)\0*.srt\0\0");
				CFileOpenDlg	dlg(TOLOCAL(_T("Open Lyrics file")), strFile.c_str(), SZ_SUPPORTED_FILE_TYPE, 1);

				if (dlg.DoModal(m_pSkinWnd) == IDOK)
				{
					strFile = dlg.GetOpenFile();
					if (g_LyricSearch.AssociateLyrics(m_szAssociateKeyword, strFile.c_str()))
					{
						CMPlayerAppBase::GetInstance()->DispatchResearchLyrics();
						CSkinPageHandler::OnOK();
					}
				}
			}
			break;
		default:
			return false;
			break;
		}

		return true;
	}

	bool OnCustomCommand(int nId)
	{
		if (nId == CID_SEARCH)
		{
			OnSearch();
		}
		else if (nId == CID_OPEN)
		{
			OnOK();
		}
		else if (nId == CID_GOOGLE)
		{
			//
			tstring		strLink = _T("http://www.google.com/cse?cx=partner-pub-6917905549616855%3All7s7l-jge2&ie=ISO-8859-1&q=");
			tstring strArtist = GetUIObjectText(CID_E_ARTIST);
			tstring strTitle = GetUIObjectText(CID_E_TITLE);
			strrep(strArtist, _T(" "), _T("+"));
			strrep(strTitle, _T(" "), _T("+"));
			strLink += strArtist + _T("+") + strTitle;
			strLink += _T("&siteurl=viewlyrics.com%2Fsearch.htm");
			OpenUrl(m_pSkinWnd, strLink.c_str());
		}
		else if (nId == CMD_SHOW_ERR_RESULT)
		{
			if (m_nErrResultToShow == ERR_NOT_FOUND)
			{
				tstring		strMsg;
				strMsg = TOLOCAL(_T("Sorry, no result returned. Please make sure you enter the correct artist name and title."));
				strMsg += _T("\r\n");
				strMsg += TOLOCAL(_T("If you have the lyrics, please upload it to our server to share it with others."));
				m_pSkinWnd->MessageOut(strMsg.c_str());
			}
			else if (m_nErrResultToShow != ERR_OK)
				ShowErrorDlg(m_pSkinWnd, m_nErrResultToShow);
		}
		else if (nId == CID_MORE)
		{
			CRect		rc;
			bool		bLocalLyr = false;
			bool		bLocalLyrFile = false;

			LrcSearchResult	*pSel;

			pSel = GetCurSel();
			if (pSel)
			{
				if (!IsHttpUrl(pSel->strUrl.c_str()))
				{
					bLocalLyr = true;
					if (GetLrcSourceType(pSel->strUrl.c_str()) == LST_FILE)
						bLocalLyrFile = true;
				}
			}

			m_menu.EnableItem(IDC_EXTERNAL_LYR_EDIT, bLocalLyrFile);
			m_menu.EnableItem(IDC_DEL_FILE, bLocalLyr);
			m_menu.EnableItem(IDC_RENAME, bLocalLyrFile);
			m_menu.CheckItem(IDC_NO_PROMPT,
				!g_Profile.GetProfileBool(SZ_SECT_LYR_DL, _T("DownLrcUserSelect"), false));

			GetUIObjectRect(CID_MORE, rc);
			m_pSkinWnd->ClientToScreen(rc);
			m_menu.TrackPopupMenu(rc.left, rc.top, m_pSkinWnd);
		}
		else if (nId == CID_CANCEL)
		{
			PostDestroy();
		}
		else if (nId == CID_NEXT_PAGE)
		{
			if (m_nCurPage >= m_nPageCount - 1)
				return true;
			m_nCurPage++;

			if (m_nCurPage < (int)m_vCachePages.size())
				m_vResultsInet = m_vCachePages[m_nCurPage];
			else
				SearchOnline();

			UpdateSearchResultToListCtrl();

			m_pLyricsList->SetItemSelectionState(0, TRUE);

			EnableUIObject(CID_NEXT_PAGE, m_nCurPage < m_nPageCount - 1);
			EnableUIObject(CID_PREV_PAGE, m_nCurPage > 0);
		}
		else if (nId == CID_PREV_PAGE)
		{
			if (m_nCurPage <= 0)
				return true;
			m_nCurPage--;

			ASSERT(m_nCurPage < (int)m_vCachePages.size());
			m_vResultsInet = m_vCachePages[m_nCurPage];

			UpdateSearchResultToListCtrl();

			m_pLyricsList->SetItemSelectionState(0, TRUE);

			EnableUIObject(CID_NEXT_PAGE, m_nCurPage < m_nPageCount - 1);
			EnableUIObject(CID_PREV_PAGE, m_nCurPage > 0);
		}
		else
			return false;

		return true;
	}

	bool OnKeyDown(UINT nChar, UINT nFlags)
	{
		if (nChar == VK_RETURN)
		{
			CUIObject *pObj = m_pSkinWnd->GetFocusUIObj();
			if (pObj)
			{
				int nFocusId = m_pSkinWnd->GetFocusUIObj()->GetID();
				if (nFocusId == CID_E_ARTIST || nFocusId == CID_E_TITLE)
				{
					m_pSkinWnd->PostCustomCommandMsg(CID_SEARCH);
					return true;
				}
				else if (nFocusId == CID_L_LYRICS)
				{
					m_pSkinWnd->PostCustomCommandMsg(CID_OPEN);
					return true;
				}
			}
		}
		return false;
	}

protected:
	void OnOnlineSearchEnd()
	{
		ASSERT(m_pSearchObj);

		if (m_pSearchObj->m_strMsg.size())
		{
			m_pSkinWnd->MessageOut(m_pSearchObj->m_strMsg.c_str());
		}

		else if (m_pSearchObj->m_nResult != ERR_OK && !m_pSearchObj->IsJobCanceled())
		{
			m_nErrResultToShow = m_pSearchObj->m_nResult;
			m_pSkinWnd->PostCustomCommandMsg(CMD_SHOW_ERR_RESULT);
		}

		m_nCurPage = m_pSearchObj->m_nCurPage;
		m_nPageCount = m_pSearchObj->m_nPageCount;

		m_vResultsInet.Sort();

		SearchLocal();

		UpdateSearchResultToListCtrl();

		if (m_pLyricsList->GetItemCount() > 0)
			m_pLyricsList->SetItemSelectionState(0, TRUE);

		if (m_nPageCount > 1)
		{
			m_vCachePages.push_back(m_vResultsInet);
			HidePreNextButtons(false);
		}
		else
		{
			HidePreNextButtons(true);
		}
	}

	void OnSearch()
	{
		m_strArtist = GetUIObjectText(CID_E_ARTIST);
		m_strTitle = GetUIObjectText(CID_E_TITLE);

		m_strArtistLastSearch = m_strArtist;
		m_strTitleLastSearch = m_strTitle;

		m_nCurPage = 0;
		m_nPageCount = 1;

		m_vCachePages.clear();

		SearchOnline();
	}

	void SearchLocal()
	{
		m_vResultsLocal.clear();

		tstring			strFullTitle;
		TCHAR			szLyricsFile[MAX_PATH];
		uint32			nTimeStart = GetTickCount();
		const int		nAcceptableCostTime = 500;

		strFullTitle = m_strArtist;
		strFullTitle += _T(" - ");
		strFullTitle += m_strTitle;

		/*	{
		//
		// Add "No suitable lyrics"
		//
		LrcSearchResult		result;
		result.strUrl = NONE_LYRCS;
		result.strSaveFileName = TOLOCAL(_T("No suitable lyrics for the song file"));
		result.nMatchValue = MATCH_VALUE_WITH_LRC - 5;
		m_vResultsLocal.AddResult(result);
		}*/

		g_LyricSearch.SearchAllMatchLyrics(m_strSongFile.c_str(), strFullTitle.c_str(), m_vResultsLocal);

		//
		// Set best match lyrics
		//
		if (g_LyricSearch.GetBestMatchLyrics(m_strSongFile.c_str(), m_strArtist.c_str(), m_strTitle.c_str(), szLyricsFile)
			&& !g_LyricSearch.IsAssociatedWithNoneLyrics(m_szAssociateKeyword))
		{
			LrcSearchResult	*pResult;
			pResult = m_vResultsLocal.SearchByUrl(szLyricsFile);
			if (pResult)
			{
				pResult->nMatchValue = MATCH_VALUE_MAX;
			}
			else
			{
				LrcSearchResult		result;
				result.strUrl = szLyricsFile;
				result.nMatchValue = MATCH_VALUE_MAX;
				m_vResultsLocal.AddResult(result);
			}
		}

		m_vResultsLocal.Sort();

		m_vResultsLocal.UpdateLyricsName();

		UpdateLyrResultArtistTitle(m_vResultsLocal);

		m_bAutoSearchLocal = (GetTickCount() - nTimeStart <= nAcceptableCostTime);
	}

	void SearchOnline()
	{
		m_vResultsInet.clear();

		if (m_strArtistLastSearch.empty() && m_strTitleLastSearch.empty())
			return;

		if (m_strTitleLastSearch.empty())
			ShowRegisterNotice(m_pSkinWnd, false);

		m_pLyricsList->DeleteAllItems();

		ASSERT(!m_pSearchObj);
		m_pSearchObj = new CSearchWorkObj();

		m_pSearchObj->m_strArtist = m_strArtistLastSearch;
		m_pSearchObj->m_strTitle = m_strTitleLastSearch;
		m_pSearchObj->m_pvSearchResult = &m_vResultsInet;
		m_pSearchObj->m_nCurPage = m_nCurPage;

		CPageWaitMessage	*pPageWait = (CPageWaitMessage *)m_pParent->GetPage(CPageWaitMessage::GetPageIdStr());
		ASSERT(pPageWait);
		if (pPageWait)
		{
			pPageWait->StartWork(TOLOCAL(_T("Searching Lyrics")), 
				TOLOCAL(_T("$Product$ is searching lyrics now, it may take a few seconds.")),
				m_pSearchObj);
		}
	}

	void UpdateSearchResultToListCtrl()
	{
		m_pLyricsList->DeleteAllItems(false);

		for (V_LRCSEARCHRESULT::iterator it = m_vResultsLocal.begin(); it != m_vResultsLocal.end(); it++)
		{
			AppendItemInResultsList(*it, false);
		}

		for (V_LRCSEARCHRESULT::iterator it = m_vResultsInet.begin(); it != m_vResultsInet.end(); it++)
		{
			AppendItemInResultsList(*it, true);
		}

		if (m_pLyricsList->GetItemCount()
			&& !g_LyricSearch.IsAssociatedWithNoneLyrics(m_szAssociateKeyword))
			m_pLyricsList->SetItemSelectionState(0, true);

		m_pLyricsList->Invalidate();
	}

	void AppendItemInResultsList(LrcSearchResult &Item, bool bInetFile)
	{
		int				nItem;
		TCHAR			szTemp[MAX_PATH];
		int				nImg;
		MLFileType		fileType;
		LRC_SOURCE_TYPE	lrcSrcType;
		tstring			strName;
		bool			bLocalFile = false;

		lrcSrcType = GetLrcSourceType(Item.strUrl.c_str());

		fileType = MLGetFileType(Item.strSaveFileName.c_str());
		if (fileType == FT_LYRICS_LRC)		nImg = IMG_L_LRC;
		else if (fileType == FT_LYRICS_TXT)	nImg = IMG_L_TXT;
		else								nImg = IMG_L_UNKNOWN;

		if (!bInetFile)
		{
			// local file...
			if (lrcSrcType == LST_ID3V2_SYLT || lrcSrcType == LST_ID3V2_LYRICS || lrcSrcType == LST_LYRICS3V2)
				nImg = IMG_L_LRC;
			else if (lrcSrcType == LST_ID3V2_USLT)
				nImg = IMG_L_TXT;
			else if (lrcSrcType == LST_NONE)
				nImg = IMG_NONE;
			else if (lrcSrcType == LST_FILE)
			{
				bLocalFile = true;
			}
		}
		else
			nImg += IMG_OFFSET_INET;

		nItem = m_pLyricsList->InsertItem(m_pLyricsList->GetItemCount(), Item.strArtist.c_str(), nImg, 0, false);
		m_pLyricsList->SetItemText(nItem, COL_TITLE, Item.strTitle.c_str(), false);

		if (Item.fRate != 0.0)
		{
			_stprintf_s(szTemp, CountOf(szTemp), _T("%.1f/5 (%d %s)"), Item.fRate, Item.nRateCount, TOLOCAL(_T("Votes")));
		}
		else
			_tcscpy_safe(szTemp, CountOf(szTemp), _T("-"));
		m_pLyricsList->SetItemText(nItem, COL_RATING, szTemp, false);

		if (bInetFile)
		{
			_itot_s(Item.nDownloads, szTemp, CountOf(szTemp), 10);
			m_pLyricsList->SetItemText(nItem, COL_DOWNLOADS, szTemp, false);
		}

		if (bLocalFile)
			m_pLyricsList->SetItemText(nItem, COL_FROM, Item.strUrl.c_str(), false);
		else
			m_pLyricsList->SetItemText(nItem, COL_FROM, Item.strUploader.c_str(), false);

		m_pLyricsList->SetItemText(nItem, COL_ALBUM, Item.strAlbum.c_str(), false);
		m_pLyricsList->SetItemText(nItem, COL_URL, Item.strUrl.c_str(), false);
	}

	LrcSearchResult *GetCurSel()
	{
		int		nPos;

		nPos = m_pLyricsList->GetNextSelectedItem(-1);
		if (nPos < 0)
			return NULL;

		if (nPos < (int)m_vResultsLocal.size())
			return m_vResultsLocal.GetByOrder(nPos);

		nPos -= m_vResultsLocal.size();
		if (nPos < (int)m_vResultsInet.size())
			return m_vResultsInet.GetByOrder(nPos);
		else
		{
			ASSERT(0 && "Invalid order");
			return NULL;
		}
	}

	void HidePreNextButtons(bool bHide)
	{
		CRect		rcBt, rcResultList = m_pLyricsList->m_rcObj;

		if (GetUIObjectRect(CID_NEXT_PAGE, rcBt))
		{
			int		nIncValue;
			if (bHide)
				nIncValue = rcBt.bottom - rcResultList.bottom - 3;
			else
				nIncValue = -(rcResultList.bottom - (rcBt.top - 8));

			m_pLyricsList->IncreaseHeight(nIncValue);
		}

		SetUIObjectVisible(CID_PREV_PAGE, !bHide, false);
		SetUIObjectVisible(CID_NEXT_PAGE, !bHide, false);

		InvalidateRect();
		// m_pSkinWnd->EnableUIObject(CID_NEXT_PAGE, m_nCurPage < m_nPageCount - 1);
		// m_pSkinWnd->EnableDlgItem(CID_PREV_PAGE, m_nCurPage > 0);
	}

protected:
	int			CID_PREV_PAGE, CID_NEXT_PAGE, CID_OPEN, CID_GOOGLE, CID_MORE, CID_CANCEL;
	int			CID_E_ARTIST, CID_E_TITLE, CID_SEARCH, CID_L_LYRICS;

	V_LRCSEARCHRESULT	m_vResultsLocal;
	V_LRCSEARCHRESULT	m_vResultsInet;

	tstring				m_strArtistLastSearch, m_strTitleLastSearch;
	int					m_nCurPage, m_nPageCount;

	// cache returned result.
	vector<V_LRCSEARCHRESULT>	m_vCachePages;

	TCHAR				m_szAssociateKeyword[MAX_PATH];
	tstring				m_strSongFile, m_strArtist, m_strTitle;

	CMLMenu				m_menu;

	//
	// Auto search local lyrics, if artist/tile changed?
	//
	bool				m_bAutoSearchLocal;

	CSkinListCtrl		*m_pLyricsList;

	CSearchWorkObj		*m_pSearchObj;
	int					m_nErrResultToShow;

};

//////////////////////////////////////////////////////////////////////////


CDlgOpenLyrics::CDlgOpenLyrics() : CMPSkinDialog(_T("OpenLyrics.xml"))
{
	m_pageHandler.AddPage(new CPageSearchLyrics());
	m_pageHandler.AddPage(new CPageWaitMessage(false));
	m_pageHandler.AddPage(new CPageRename());
}
