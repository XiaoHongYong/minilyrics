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

#define SZ_EX_POOL_ARTIST		_T("artist")
#define SZ_EX_POOL_TITLE		_T("title")
#define SZ_EX_POOL_ALBUM		_T("album")
#define SZ_EX_POOL_MEDIA_URL	_T("media_url")
#define SZ_EX_POOL_MEDIA_KEY	_T("media_key")

bool IsHttpUrl(LPCTSTR szUrl)
{
	LPCTSTR		SZ_HTTP_HEADER = _T("http://");

	return _tcsnicmp(szUrl, SZ_HTTP_HEADER, _tcslen(SZ_HTTP_HEADER)) == 0;
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

class CPageRename : public CSkinContainer
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
public:
	void OnSwitchTo()
	{
		CSkinContainer::OnSwitchTo();

		m_pSkin->SetCaptionText(_TLT("Rename lyrics file"));

		SetUIObjectText(_T("CID_E_FILENAME"), GetExPoolStr(_T("FileTitle")).c_str(), false);
		SetExPool(_T("RenameDlgResult"), IDCANCEL);
	}

	bool OnOK()
	{
		SetExPool(_T("FileTitle"), GetUIObjectText(_T("CID_E_FILENAME")).c_str());
		SetExPool(_T("RenameDlgResult"), IDOK);
		m_pContainer->SwitchToLastPage(0, true);
		return false;
	}

	bool OnCancel()
	{
		m_pContainer->SwitchToLastPage(0, true);
		return false;
	}

};

UIOBJECT_CLASS_NAME_IMP(CPageRename, _T("Container.RenameLyr"))

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

		ASSERT(m_strMediaKey.size() > 0);
		m_nResult = g_OnlineSearch.SearchOnline(m_session, m_strArtist.c_str(), m_strTitle.c_str(), m_strMediaKey.c_str(), *m_pvSearchResult, m_strMsg, m_nCurPage, m_nPageCount);

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

	tstring					m_strArtist, m_strTitle, m_strMediaKey;

};

//////////////////////////////////////////////////////////////////////////

class CPageSearchLyrics : public CSkinContainer, public IUIObjNotifyHandler, public IEditNotification
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
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
	int TIMER_ID_RESEARCH;
	enum {
		DURATION_AUTO_SEARCH	= 1000,

		R_CODE_RENAME_LYR = 1,
	};

	CPageSearchLyrics()
	{
		TIMER_ID_RESEARCH = 0;
		m_bAutoSearchLocal = true;
		m_nCurPage = 0;
		m_nPageCount = 0;

		m_pLyricsList = NULL;
		m_pSearchObj = NULL;

		CID_PREV_PAGE = CID_NEXT_PAGE = CID_GOOGLE = CID_MORE = CID_E_ARTIST = 0;
		CID_E_TITLE = CID_SEARCH = CID_OPEN = CID_L_LYRICS = 0;
		m_strTitle = _TLM("Search Lyrics");
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
				if (OnOK())
					m_pSkin->PostDestroy();
				break;
			default:
				break;
			}
		}
	}

	// Edit control notification
	void OnTextChanged()
	{
		if (m_bAutoSearchLocal)
		{
			m_pSkin->UnregisterTimerObject(this, TIMER_ID_RESEARCH);
			TIMER_ID_RESEARCH = m_pSkin->RegisterTimerObject(this, DURATION_AUTO_SEARCH);
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
		CSkinContainer::OnCreate();

		GET_ID_BY_NAME3(CID_PREV_PAGE, CID_GOOGLE, CID_NEXT_PAGE);
		GET_ID_BY_NAME3(CID_MORE, CID_E_ARTIST, CID_E_TITLE);
		GET_ID_BY_NAME3(CID_SEARCH, CID_OPEN, CID_L_LYRICS);

		// Save current media info.
		m_strSongFile = GetExPoolStr(SZ_EX_POOL_MEDIA_URL);
		m_strMediaKey = GetExPoolStr(SZ_EX_POOL_MEDIA_KEY);
		m_strArtist = GetExPoolStr(SZ_EX_POOL_ARTIST);
		m_strTitle = GetExPoolStr(SZ_EX_POOL_TITLE);

		m_pLyricsList = (CSkinListCtrl *)m_pSkin->GetUIObjectById(CID_L_LYRICS, CSkinListCtrl::ClassName());
		ASSERT(m_pLyricsList);
		if (!m_pLyricsList)
			return;

		SetUIObjectText(CID_E_ARTIST, m_strArtist.c_str(), false);
		SetUIObjectText(CID_E_TITLE, m_strTitle.c_str(), false);

		LPCTSTR		szHead[] = {_T("Artist"), _T("Title"), _T("Rating"), _T("Downloads"), _T("From"), _T("Album"), NULL};
		int			arrWidth[] = {130,        160,         90,           68,              150,       150,         0};

		ASSERT(CountOf(arrWidth) == CountOf(szHead));
		for (int i = 0; szHead[i] != NULL; i++)
			m_pLyricsList->AddColumn(_TL(szHead[i]), arrWidth[i]);
		if (g_Profile.GetInt(_T("ShowLink"), false))
			m_pLyricsList->AddColumn(_T("URL"), 400);
		m_pLyricsList->LoadColumnWidth(_T("OpenLyrics_ListColWidth"));

		g_OnlineSearch.SearchCacheForCur(&m_vResultsInet);
		m_vResultsInet.Sort();

		SearchLocal();

		UpdateSearchResultToListCtrl();

		m_pSkin->RegisterUIObjNotifyHandler(CID_L_LYRICS, this);

		CSkinEditCtrl *pEdit = (CSkinEditCtrl*)m_pSkin->GetUIObjectById(CID_E_ARTIST, CSkinEditCtrl::ClassName());
		if (pEdit)
		{
			pEdit->SetEditNotification(this);
			pEdit = (CSkinEditCtrl*)m_pSkin->GetUIObjectById(CID_E_TITLE, CSkinEditCtrl::ClassName());
			if (pEdit)
				pEdit->SetEditNotification(this);
		}
	}

	void OnDestroy()
	{
		m_pSkin->UnregisterUIObjNotifyHandler(this);
		m_pLyricsList->SaveColumnWidth(_T("OpenLyrics_ListColWidth"));
	}

	bool OnOK()
	{
		CUIObject *pObj = m_pSkin->GetFocusUIObj();
		if (pObj)
		{
			int nFocusId = m_pSkin->GetFocusUIObj()->GetID();
			if (nFocusId == CID_E_ARTIST || nFocusId == CID_E_TITLE)
			{
				m_pSkin->PostCustomCommandMsg(CID_SEARCH);
			}
			else if (nFocusId == CID_L_LYRICS)
			{
				m_pSkin->PostCustomCommandMsg(CID_OPEN);
			}
		}

		return false;
	}

	void OnOpen()
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
			g_LyricsDownloader.DownloadLyrics(m_strMediaKey.c_str(), 
				m_strSongFile.c_str(), pSel->strUrl.c_str(), pSel->strSaveFileName.c_str());
		}
		else
		{
			// Associate lyrics file
			if (g_LyricSearch.AssociateLyrics(m_strMediaKey.c_str(), pSel->strUrl.c_str()))
			{
				CMPlayerAppBase::GetInstance()->DispatchResearchLyrics();
			}
		}
		m_pSkin->PostDestroy();
	}

	void OnSwitchTo()
	{
		CSkinContainer::OnSwitchTo();

		CSkinWnd::CAutoRedrawLock	redrawLock(m_pSkin);

		if (m_pSearchObj)
		{
			OnOnlineSearchEnd();
			delete m_pSearchObj;
			m_pSearchObj = NULL;
		}
		else if (GetExPoolInt(_T("RenameDlgResult")) == IDOK)
		{
			//
			// Rename selected lyrics file
			//
			LrcSearchResult	*pSel;
			SetExPool(_T("RenameDlgResult"), IDCANCEL);

			pSel = GetCurSel();
			if (!pSel || LyrSrcTypeFromName(pSel->strUrl.c_str()) != LST_FILE)
				return;

			tstring strFileTitle = GetExPoolStr(_T("FileTitle"));
			tstring strFileTitleOld = FileGetTitle(pSel->strUrl.c_str());

			// Same?
			if (_tcscmp(strFileTitle.c_str(), strFileTitleOld.c_str()) == 0)
				return;

			tstring strFileNew = FileGetPath(pSel->strUrl.c_str());
			strFileNew += strFileTitle.c_str();
			strFileNew += FileGetExt(pSel->strUrl.c_str());
			if (!MoveFile(pSel->strUrl.c_str(), strFileNew.c_str()))
			{
				m_pSkin->MessageOut(_TLT("Failed to rename lyrics file."));
			}
			else
			{
				// Change lyrics association, if necessary.
				TCHAR	szAssociateLyr[MAX_PATH];
				if (g_LyricSearch.GetAssociateLyrics(m_strMediaKey.c_str(), szAssociateLyr, CountOf(szAssociateLyr))
					&& _tcscmp(szAssociateLyr, pSel->strUrl.c_str()) == 0)
					g_LyricSearch.AssociateLyrics(m_strMediaKey.c_str(), strFileNew.c_str());

				pSel->strUrl = strFileNew;

				int			nSel = m_pLyricsList->GetNextSelectedItem(-1);
				m_pLyricsList->SetItemText(nSel, COL_FROM, strFileNew.c_str());
			}
		}

		redrawLock.CancelDrawUpdate();
	}

	bool OnTimer(UINT nIDEvent)
	{
		if (nIDEvent != TIMER_ID_RESEARCH)
			return false;

		m_pSkin->UnregisterTimerObject(this, TIMER_ID_RESEARCH);

		tstring strArtist = GetUIObjectText(CID_E_ARTIST);
		tstring strTitle = GetUIObjectText(CID_E_TITLE);

		// Not changed?
		if ((_tcsicmp(strArtist.c_str(), m_strArtist.c_str()) == 0
			&& _tcsicmp(strTitle.c_str(), m_strTitle.c_str()) == 0)
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
				if (m_strMediaKey.empty())
					break;

				g_LyricSearch.AssociateLyrics(m_strMediaKey.c_str(), NONE_LYRCS);
				CMPlayerAppBase::GetInstance()->DispatchResearchLyrics();
				m_pSkin->PostDestroy();
			}
			break;
		case IDC_NO_PROMPT:
			// Don't prompt this dialog automatically.
			g_Profile.WriteInt(SZ_SECT_LYR_DL, _T("DownLrcUserSelect"), 
				!g_Profile.GetBool(SZ_SECT_LYR_DL, _T("DownLrcUserSelect"), false));
			break;
		case IDC_RENAME:
			{
				//
				// Rename selected lyrics file
				//
				LrcSearchResult	*pSel;

				pSel = GetCurSel();
				if (!pSel || LyrSrcTypeFromName(pSel->strUrl.c_str()) != LST_FILE)
					break;

				tstring		strTitle = FileGetTitle(pSel->strUrl.c_str());

				SetExPool(_T("FileTitle"), strTitle.c_str());
				m_pContainer->SwitchToPage(CPageRename::ClassName(), true, R_CODE_RENAME_LYR, true);
			}
			break;
		case IDC_DEL_FILE:
			{
				LrcSearchResult	*pSel;

				pSel = GetCurSel();
				if (!pSel)
					break;

				LRC_SOURCE_TYPE lst = LyrSrcTypeFromName(pSel->strUrl.c_str());
				bool bDeleted = false;
				if (lst == LST_FILE)
				{
					SHDeleteFile(pSel->strUrl.c_str(), m_pSkin);
					if (!IsFileExist(pSel->strUrl.c_str()))
						bDeleted = true;
				}
				else
				{
					VecString vLyrNames;
					vLyrNames.push_back(pSel->strUrl);
					int nRet = g_autoProcessEmbeddedLyrics.RemoveEmbeddedLyrics(m_strSongFile.c_str(), vLyrNames, true);
					if (nRet == ERR_OK)
						bDeleted = true;
					else
						m_pSkin->MessageOut(ERROR2STR_LOCAL(nRet));
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

				if (LyrSrcTypeFromName(pSel->strUrl.c_str()) != LST_FILE)
					break;

				tstring		strEditor;
				GetNotepadEditor(strEditor);
				Execute(m_pSkin,
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
					if (LyrSrcTypeFromName(pSel->strUrl.c_str()) != LST_FILE)
						strFile = pSel->strUrl;
				}

				static const TCHAR *SZ_SUPPORTED_FILE_TYPE		 = _T("All supported files (*.lrc; *.txt; *.srt)\0*.lrc;*.txt;*.srt\0Lyrics File (*.lrc)\0*.LRC\0Text File (*.txt)\0*.txt\0Vob Subtitle File(*.srt)\0*.srt\0\0");
				CFileOpenDlg	dlg(_TLT("Open Lyrics file"), strFile.c_str(), SZ_SUPPORTED_FILE_TYPE, 1);

				if (dlg.DoModal(m_pSkin) == IDOK)
				{
					strFile = dlg.GetOpenFile();
					if (g_LyricSearch.AssociateLyrics(m_strMediaKey.c_str(), strFile.c_str()))
						CMPlayerAppBase::GetInstance()->DispatchResearchLyrics();

					m_pSkin->PostDestroy();
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
			OnSearch();
		else if (nId == CID_OPEN)
			OnOpen();
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
			OpenUrl(m_pSkin, strLink.c_str());
		}
		else if (nId == CMD_SHOW_ERR_RESULT)
		{
			if (m_nErrResultToShow == ERR_NOT_FOUND)
			{
				tstring		strMsg;
				strMsg = _TLT("Sorry, no result returned. Please make sure you enter the correct artist name and title.");
				strMsg += _T("\r\n");
				strMsg += _TLT("If you have the lyrics, please upload it to our server to share it with others.");
				m_pSkin->MessageOut(strMsg.c_str());
			}
			else if (m_nErrResultToShow != ERR_OK)
				ShowInetErrorDlg(m_pSkin, m_nErrResultToShow);
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
					if (LyrSrcTypeFromName(pSel->strUrl.c_str()) == LST_FILE)
						bLocalLyrFile = true;
				}
			}

			CSkinMenu m_menu;
			m_menu.LoadMenu(IDR_MENU_OPEN_LRC_DLG);

			m_menu.EnableItem(IDC_EXTERNAL_LYR_EDIT, bLocalLyrFile);
			m_menu.EnableItem(IDC_DEL_FILE, bLocalLyr);
			m_menu.EnableItem(IDC_RENAME, bLocalLyrFile);
			m_menu.CheckItem(IDC_NO_PROMPT,
				!g_Profile.GetBool(SZ_SECT_LYR_DL, _T("DownLrcUserSelect"), false));

			GetUIObjectRect(CID_MORE, rc);
			m_pSkin->ClientToScreen(rc);
			m_menu.TrackPopupMenu(rc.left, rc.top, m_pSkin);
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

protected:
	void OnOnlineSearchEnd()
	{
		ASSERT(m_pSearchObj);

		if (m_pSearchObj->m_strMsg.size())
		{
			m_pSkin->MessageOut(m_pSearchObj->m_strMsg.c_str());
		}

		else if (m_pSearchObj->m_nResult != ERR_OK && !m_pSearchObj->IsJobCanceled())
		{
			m_nErrResultToShow = m_pSearchObj->m_nResult;
			m_pSkin->PostCustomCommandMsg(CMD_SHOW_ERR_RESULT);
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

		uint32			nTimeStart = GetTickCount();
		const int		nAcceptableCostTime = 500;

		g_LyricSearch.SearchAllMatchLyrics(m_strSongFile.c_str(), m_strArtist.c_str(), m_strTitle.c_str(), m_vResultsLocal);

		//
		// Set best match lyrics
		//
		tstring strLyricsFile;
		if (g_LyricSearch.GetBestMatchLyrics(m_strSongFile.c_str(), m_strArtist.c_str(), m_strTitle.c_str(), strLyricsFile)
			&& !g_LyricSearch.IsAssociatedWithNoneLyrics(m_strMediaKey.c_str()))
		{
			LrcSearchResult	*pResult;
			pResult = m_vResultsLocal.SearchByUrl(strLyricsFile.c_str());
			if (pResult)
			{
				pResult->nMatchValue = MATCH_VALUE_MAX;
			}
			else
			{
				LrcSearchResult		result;
				result.strUrl = strLyricsFile;
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

		m_pLyricsList->DeleteAllItems();

		ASSERT(!m_pSearchObj);
		m_pSearchObj = new CSearchWorkObj();

		m_pSearchObj->m_strArtist = m_strArtistLastSearch;
		m_pSearchObj->m_strTitle = m_strTitleLastSearch;
		m_pSearchObj->m_strMediaKey = m_strMediaKey;
		m_pSearchObj->m_pvSearchResult = &m_vResultsInet;
		m_pSearchObj->m_nCurPage = m_nCurPage;

		CPageWaitMessage	*pPageWait = (CPageWaitMessage *)m_pContainer->GetChildByClass(CPageWaitMessage::ClassName());
		ASSERT(pPageWait);
		if (pPageWait)
		{
			pPageWait->StartWork(_TLT("Searching lyrics"), 
				_TLT("$Product$ is searching lyrics now, it may take a few seconds."),
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

		if (m_pLyricsList->GetRowCount() > 0
			&& !g_LyricSearch.IsAssociatedWithNoneLyrics(m_strMediaKey.c_str()))
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

		lrcSrcType = LyrSrcTypeFromName(Item.strUrl.c_str());

		fileType = MLGetFileType(Item.strSaveFileName.c_str());
		if (fileType == FT_LYRICS_LRC)		nImg = IMG_L_LRC;
		else if (fileType == FT_LYRICS_TXT)	nImg = IMG_L_TXT;
		else								nImg = IMG_L_UNKNOWN;

		if (!bInetFile)
		{
			// local file...
			if (lrcSrcType == LST_ID3V2_SYLT || lrcSrcType == LST_ID3V2_LYRICS || lrcSrcType == LST_LYRICS3V2 || lrcSrcType == LST_KAR_LYRICS)
				nImg = IMG_L_LRC;
			else if (lrcSrcType == LST_ID3V2_USLT || lrcSrcType == LST_M4A_LYRICS)
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
			_stprintf_s(szTemp, CountOf(szTemp), _T("%.1f/5 (%d %s)"), Item.fRate, Item.nRateCount, _TLT("Votes"));
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
		if (m_pLyricsList->GetColumnCount() > COL_URL)
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

		Invalidate();
	}

protected:
	int			CID_PREV_PAGE, CID_NEXT_PAGE, CID_GOOGLE, CID_MORE;
	int			CID_E_ARTIST, CID_E_TITLE, CID_OPEN, CID_SEARCH, CID_L_LYRICS;

	V_LRCSEARCHRESULT	m_vResultsLocal;
	V_LRCSEARCHRESULT	m_vResultsInet;

	tstring				m_strArtistLastSearch, m_strTitleLastSearch;
	int					m_nCurPage, m_nPageCount;

	// cache returned result.
	vector<V_LRCSEARCHRESULT>	m_vCachePages;

	tstring				m_strSongFile, m_strMediaKey, m_strArtist, m_strTitle;

	//
	// Auto search local lyrics, if artist/tile changed?
	//
	bool				m_bAutoSearchLocal;

	CSkinListCtrl		*m_pLyricsList;

	CSearchWorkObj		*m_pSearchObj;
	int					m_nErrResultToShow;

};

UIOBJECT_CLASS_NAME_IMP(CPageSearchLyrics, _T("Container.SearchLyr"))

//////////////////////////////////////////////////////////////////////////

void ShowSearchLyricsDialog(CSkinWnd *pParent)
{
	SkinWndStartupInfo skinWndStartupInfo(_SZ_SKINWND_CLASS_NAME, _SZ_SKINWND_CLASS_NAME, 
		_T("OpenLyrics.xml"), pParent);

	skinWndStartupInfo.mapExchangePool[SZ_EX_POOL_ARTIST] = g_Player.GetArtist();
	skinWndStartupInfo.mapExchangePool[SZ_EX_POOL_TITLE] = g_Player.GetTitle();
	skinWndStartupInfo.mapExchangePool[SZ_EX_POOL_MEDIA_URL] = g_Player.GetSrcMedia();
	skinWndStartupInfo.mapExchangePool[SZ_EX_POOL_ALBUM] = g_Player.GetAlbum();
	skinWndStartupInfo.mapExchangePool[SZ_EX_POOL_MEDIA_KEY] = CMPlayerAppBase::GetAssociateFileKeyword();

	CSkinApp::GetInstance()->GetSkinFactory()->ActiveOrCreateSkinWnd(skinWndStartupInfo);
}

void RegisterSearchLyricsPage(CSkinFactory *pSkinFactory)
{
	AddUIObjNewer2(pSkinFactory, CPageSearchLyrics);
	AddUIObjNewer2(pSkinFactory, CPageWaitMessage);
	AddUIObjNewer2(pSkinFactory, CPageRename);
}
