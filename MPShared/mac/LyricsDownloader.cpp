// DlgAdjustHue.cpp: implementation of the CDlgAdjustHue class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "LyricsDownloader.h"
#include "OnlineSearch.h"
#include "DownloadMgr.h"
#include "MediaSource.h"
#include "../../MLLib/LrcParserHelper.h"
#include "DlgAbout.h"
#include "MPlaylistCtrl.h"
#include "../../Utils/Looper.h"
#include "AutoProcessEmbeddedLyrics.h"
//#include "LyricsDownloader_dialog.h"
#include "LyricsDownloader_worker.h"

using namespace LyricsDownloader;

bool ConvertBufToTextLyrics(string &buf, tstring &lyrics)
{
	LPBYTE byLyrData = (LPBYTE)buf.c_str();
	int nLyrDataLen = (int)buf.size();
	CHAR_ENCODING encoding = GetLyricsTextEncoding(byLyrData, nLyrDataLen);

	if (encoding == ED_UNICODE)
		UCS2ToTString((LPCWSTR)byLyrData, nLyrDataLen / sizeof(WCHAR), lyrics);
	else if (encoding == ED_UNICODE_BIG_ENDIAN)
	{
		UCS2BEToLE((LPWSTR)byLyrData, nLyrDataLen / sizeof(WCHAR));
		UCS2ToTString((LPCWSTR)byLyrData, nLyrDataLen / sizeof(WCHAR), lyrics);
	}
	else if (encoding == ED_UTF8)
		UTF8ToTString((LPCSTR)byLyrData, nLyrDataLen, lyrics);
	else
		MBCSToTString((LPCSTR)byLyrData, nLyrDataLen, lyrics, encoding);

	return true;
}

#define SZ_NO_SUITABLE_LYRICS		_T("**NL")

class CPageLyricsTool : public CSkinContainer, public ILyricsToolWork::IUpdateNotify,  IUIObjNotifyHandler, IEventHandler
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
public:
	CPageLyricsTool()
	{
		m_filesListCtrl = NULL;
		m_pMediaLib = NULL;
		m_nProgress = 0;
		CID_UPDATEPROGRESS = 0, CID_STATUS = 0;
		m_pProgressCtrl = NULL;

		m_threadWorking.Create(WorkThreadProc, this);
		
		m_nCurrentMedia = 0;
		m_nTimerIDPlayNext = 0;
	}
	~CPageLyricsTool()
	{
		m_WorkLooper.Quit();
		m_workStatus.Cancel();
		m_threadWorking.Join();
	}

	enum
	{
		COL_MEDIA_FILE,
		COL_RESULT,
		COL_LYRICS,
	};

	enum
	{
		IMG_NONE,
		IMG_OK,
		IMG_ERROR,
		IMG_INFO,
	};

	struct UpdateItem
	{
		UpdateItem(int nIndex, LPCTSTR szResult)
		{
			this->n = nIndex;
			this->strResult = szResult;
		}
		int			n;
		tstring		strResult;
	};
	typedef list<UpdateItem>	ListUpdateItem;

	void OnCreate();
	virtual void OnDestroy();

	virtual bool OnCommand(int nId);

	virtual bool OnCustomCommand(int nId);

	virtual bool OnOK();
	virtual bool OnCancel();

	void OnUpdateItemStatus(int nIndex, ILyricsToolWork::SuccessStatus status, LPCTSTR szResult)
	{
		m_pMediaLib->SetItemImageIndex(nIndex, status, false);

		m_mutexMsg.Acquire();
		m_listUpdateItems.push_back(UpdateItem(nIndex, szResult));
		m_mutexMsg.Release();
	}

protected:
	void UpdateProgress();

	void ClearResult();

	void FromCurrentPlaylist();

	void UpdateNewMediaList();

	void OnAdvDownloadAllLyrics();
	void OnAdvRemoveAllLyrics();

	void OnDownloadLyricsForiPod();
	void OnRemoveEmbeddedLyrics();

	void PostWork(ILyricsToolWork *pWork);

	static void WorkThreadProc(LPVOID param)
	{
		CPageLyricsTool *pThis = (CPageLyricsTool *)param;

		pThis->m_WorkLooper.Loop();
	}

	void DoWork(ILyricsToolWork *pWork);

	uint32 GetEmbeddedLyricsOpt();

	void ChangeToMediaLib(MediaSourceType type)
	{
		if (m_pMediaLib)
		{
			if (m_pMediaLib->GetType() == type)
				return;

//			g_Player.ClearPlaylist();
			delete m_pMediaLib;
		}

		m_pMediaLib = NewMediaSource(type);
		m_filesListCtrl->SetDataSource(m_pMediaLib);
		m_filesListCtrl->NotifyDataChanged();
	}

	void HideInstructions()
	{
		m_pSkin->SetUIObjectVisible(UID_INSTRUCTION, false, false);
		m_pSkin->SetUIObjectVisible(UID_MEDIA_INFO, true, false);
		m_pSkin->InvalidateRect();
	}
	
	void PlayItem(int nIndex, bool playNow);

	virtual void OnUIObjNotify(IUIObjNotify *pNotify);

	virtual void OnEvent(const IEvent *pEvent)
	{
		if (pEvent->eventType == ET_PLAYER_STATUS_CHANGED)
		{
			CEventPlayerStatusChanged *pStatusEvent = (CEventPlayerStatusChanged *)pEvent;
			if (pStatusEvent->status == PS_STOPED)
			{
				if (m_pMediaLib != NULL && m_nCurrentMedia < m_pMediaLib->GetRowCount())
				{
					if (m_nTimerIDPlayNext != 0)
						m_pSkin->UnregisterTimerObject(this, m_nTimerIDPlayNext);
					m_nTimerIDPlayNext = m_pSkin->RegisterTimerObject(this, 300);
				}
			}
		}
	}
	
	virtual void OnTimer(int nId)
	{
		if (nId != m_nTimerIDPlayNext)
		{
			CSkinContainer::OnTimer(nId);
			return;
		}
		
		m_pSkin->UnregisterTimerObject(this, m_nTimerIDPlayNext);
		m_nTimerIDPlayNext = 0;
		if (g_Player.GetPlayerState() == PS_STOPED
			&& m_pMediaLib != NULL && m_nCurrentMedia < m_pMediaLib->GetRowCount())
		{
			PlayItem(m_nCurrentMedia + 1, true);
		}
	}

	friend class CRunnableDoWork;
	friend class CRunnableUpdateiTunesMediaLib;

protected:
	CSkinListView			*m_filesListCtrl;

	IMediaSource			*m_pMediaLib;

	Mutex					m_mutexMsg;
	tstring					m_strMsg, m_strStatus;
	int						m_nProgress;
	int						CID_UPDATEPROGRESS, CID_STATUS, CID_CANCEL_WORK, UID_INSTRUCTION;
	int						CMD_IMPORT, CMD_DL_LYRICS, UID_MEDIA_INFO, CID_PROGRESS, CID_LISTVIEW;
	CSkinSeekCtrl			*m_pProgressCtrl;
	ListUpdateItem			m_listUpdateItems;

	CThread					m_threadWorking;
	CLooper					m_WorkLooper;		// Used to loop the background work
	WorkStatus				m_workStatus;

	// Handle play/next.
	int						m_nCurrentMedia;
	int						m_nTimerIDPlayNext;

};

UIOBJECT_CLASS_NAME_IMP(CPageLyricsTool, _T("Container.LyricsTool"));


void CPageLyricsTool::PlayItem(int nIndex, bool playNow)
{
	if (m_pMediaLib == NULL)
		return;
	
	uint32	nMediaLength;
	tstring artist, album, title, location;
	if (!m_pMediaLib->GetMediaInfo(nIndex, artist, album, title, location, nMediaLength))
		return;
	
	if (IsFileExist(location.c_str()))
	{
		g_Player.ClearPlaylist();
		g_Player.GetIMPlayer()->SetCurrentMedia(location.c_str());
		
		if (playNow)
			g_Player.Play();
	}
	
	m_nCurrentMedia = nIndex;
}

void CPageLyricsTool::OnUIObjNotify(IUIObjNotify *pNotify)
{
	if (pNotify->nID != CID_LISTVIEW)
		return;
	
	CSkinListCtrlEventNotify *pListViewNotify = (CSkinListCtrlEventNotify*)pNotify;
	if (pListViewNotify->cmd == CSkinListCtrlEventNotify::C_DBL_CLICK)
	{
		HideInstructions();
		PlayItem(pListViewNotify->nClickOnRow, true);
	}
}

void CPageLyricsTool::OnCreate()
{
	CSkinContainer::OnCreate();

	GET_ID_BY_NAME4(CID_STATUS, CID_UPDATEPROGRESS, CID_CANCEL_WORK, UID_INSTRUCTION);
	GET_ID_BY_NAME4(UID_MEDIA_INFO, CMD_IMPORT, CMD_DL_LYRICS, CID_PROGRESS);
	GET_ID_BY_NAME(CID_LISTVIEW);

	m_filesListCtrl = (CSkinListCtrl *)GetUIObjectById(CID_LISTVIEW, CSkinListView::ClassName());
	if (!m_filesListCtrl)
		return;

	LPCTSTR vColName[] = { _T("Media Files"), _T("Result"), _T("Lyrics") };
	for (int i = 0; i < CountOf(vColName); i++)
		m_filesListCtrl->AddColumn(_TL(vColName[i]), 200);

	m_filesListCtrl->LoadColumnWidth(_T("LyricsTool_ListWidth"));

	m_pProgressCtrl = (CSkinSeekCtrl *)m_pSkin->GetUIObjectById(CID_PROGRESS, CSkinSeekCtrl::ClassName());
	ASSERT(m_pProgressCtrl);
	if (m_pProgressCtrl)
	{
		m_pProgressCtrl->SetScrollInfo(0, 100, 0, 0, 1, false);
		m_pProgressCtrl->SetEnable(false);
	}

	//
	// Add current media source file.
	//
	FromCurrentPlaylist();

	m_filesListCtrl->SetDataSource(m_pMediaLib);
	if (!m_pMediaLib || m_pMediaLib->GetRowCount() == 0)
	{
		m_pSkin->SetUIObjectVisible(UID_INSTRUCTION, true, false);
		m_pSkin->SetUIObjectVisible(UID_MEDIA_INFO, false, false);
	}
	
	GetSkinWnd()->RegisterUIObjNotifyHandler(CID_LISTVIEW, this);
	
	RegisterHandler(CSkinApp::GetInstance()->GetEventPatcher(), ET_PLAYER_STATUS_CHANGED);

/*	if (!IsEmptyString(g_Player.GetSrcMedia()))
	{
		ChangeToMediaLib(MST_FILES);
		CFileMediaSource *pFileMediaLib = (CFileMediaSource *)m_pMediaLib;
		V_Str	vFiles;
		vFiles.push_back(g_Player.GetSrcMedia());
		pFileMediaLib->AddFiles(vFiles);
		UpdateNewMediaList();
	}*/
}

void CPageLyricsTool::OnDestroy()
{
	if (m_filesListCtrl)
		m_filesListCtrl->SaveColumnWidth(_T("LyricsTool_ListWidth"));
}

class CRunnableUpdateiTunesMediaLib : public IRunnable
{
public:
	CRunnableUpdateiTunesMediaLib(CPageLyricsTool *pPageLyricsTool)
			: m_pPageLyricsTool(pPageLyricsTool), m_eventFinished(false, false)
		{ m_bUpdateSuccessfully = false; }

	void Run()
	{
		ASSERT(m_pPageLyricsTool->m_pMediaLib);

		m_bUpdateSuccessfully = m_pPageLyricsTool->m_pMediaLib->UpdateLibrary(m_strErr);
		m_eventFinished.Set();
	}

public:
	bool					m_bUpdateSuccessfully;
	tstring					m_strErr;
	Event					m_eventFinished;

protected:
	CPageLyricsTool			*m_pPageLyricsTool;

};

class CRunnableDoWork : public IRunnable
{
public:
	CRunnableDoWork(CPageLyricsTool *pPageLyricsTool, ILyricsToolWork *pWork)
		: m_pPageLyricsTool(pPageLyricsTool) { m_pWork = pWork; }

	void Run()
	{
		m_pPageLyricsTool->DoWork(m_pWork);
	}

protected:
	ILyricsToolWork			*m_pWork;
	CPageLyricsTool			*m_pPageLyricsTool;

};

bool CPageLyricsTool::OnCommand(int nId)
{
	if (m_workStatus.IsWorking() && nId != IDC_ABOUT && nId != IDC_ABOUT
		 && nId != IDC_ML_HELP && nId != IDC_REGISTER && nId != IDC_WEBHOME)
		 return true;

	switch (nId)
	{
	case IDC_IMPORT_ITUNES_LIB:
		{
			if (m_pMediaLib)
			{
//				g_Player.ClearPlaylist();
				delete m_pMediaLib;
				m_pMediaLib = NULL;
			}
			m_filesListCtrl->SetDataSource(NULL);

			ChangeToMediaLib(MST_ITUNES);

			CRunnableUpdateiTunesMediaLib rUpdate(this);

			m_WorkLooper.Post(&rUpdate);
			rUpdate.m_eventFinished.Acquire();

			m_filesListCtrl->SetDataSource(m_pMediaLib);

			if (rUpdate.m_bUpdateSuccessfully)
				UpdateNewMediaList();
			else
			{
				m_pSkin->MessageOut(CStrPrintf(_T("%s\n%s"), 
					_TLT("Failed to load iTunes Library."),
					rUpdate.m_strErr.c_str()).c_str());
			}
		}
		break;
	case IDC_ADD_FILES:
		{
			ChangeToMediaLib(MST_FILES);

			// static LPCTSTR	szSupportedExtion = _T("All supported files\0*.mp3;*.mp2;*.mpa;\0MPEG Audio Files (*.MP3;*.MP2;*.MPA)\0*.mp3;*.mp2;*.mpa\0\0");
			CFileDlgExtFilter		strFileExt;
			strFileExt.AddExtention(_TLT("All supported files"), _T("*.mp3;*.mp2;*.mpa;*.m4a"));
			strFileExt.AddExtention(_TLT("MPEG Audio Files (*.MP3;*.MP2;*.MPA)"), _T("*.mp3;*.mp2;*.mpa"));
			strFileExt.AddExtention(_TLT("AAC Files (*.M4A)"), _T("*.m4a"));
			CFileOpenDlg	dlg(_TLT("Add Media Files"), NULL, strFileExt.c_str(), 0, true);
			if (dlg.DoModal(m_pSkin) != IDOK)
				break;

			V_Str	vFiles;
			dlg.GetOpenFile(vFiles);

			CFileMediaSource *pFileMediaLib = (CFileMediaSource *)m_pMediaLib;
			pFileMediaLib->AddFiles(vFiles);

			UpdateNewMediaList();
		}
		break;
	case IDC_ADD_FOLDER:
		{
			ChangeToMediaLib(MST_FILES);

			CFolderDialog	dlg;
			tstring			strFolder;
			dlg.SetInitFolder(g_Profile.GetString(_T("LastMediaDir"), _T("")));
			if (dlg.DoBrowse(m_pSkin) == IDOK)
			{
				strFolder = dlg.GetFolder();

				g_Profile.WriteString(_T("LastMediaDir"), strFolder.c_str());

				CFileMediaSource *pFileMediaLib = (CFileMediaSource *)m_pMediaLib;
				pFileMediaLib->AddFolder(strFolder.c_str(), true);

				UpdateNewMediaList();
			}
		}
		break;
	case IDC_CLEAR_LIST:
		if (m_pMediaLib)
		{
			delete m_pMediaLib;
			m_pMediaLib = NULL;
			m_filesListCtrl->SetDataSource(NULL);
			m_filesListCtrl->NotifyDataChanged();
		}
		break;
	case IDC_REMOVE_SELECTED:
		{
			vector<int>		vSelected;
			int				n = -1;
			while (1)
			{
				n = m_filesListCtrl->GetNextSelectedItem(n);
				if (n == -1)
					break;
				vSelected.push_back(n);
			}

			for (int i = (int)vSelected.size() - 1; i >= 0; i--)
				m_pMediaLib->RemoveItem(vSelected[i]);

			m_filesListCtrl->NotifyDataChanged();
		}
		break;
	case IDC_DOWNLOAD_ALL_LYRICS:
		HideInstructions();
		OnAdvDownloadAllLyrics();
		break;
	case IDC_REMOVE_ALL_LYRICS:
		HideInstructions();
		OnAdvRemoveAllLyrics();
		break;
	case IDC_DOWNLOAD_ALL_LYR_FOR_IPOD:
		HideInstructions();
		OnDownloadLyricsForiPod();
		break;
// 	case IDC_SAVE_LYRICS_TO_MP3:
// 		HideInstructions();
// 		break;
	case IDC_REMOVE_LRC_IN_MP3:
		HideInstructions();
		OnRemoveEmbeddedLyrics();
		break;
	case IDC_EXIT:
		m_pSkin->PostDestroy();
		break;
	default:
		return false;
	}

	return true;
}

bool CPageLyricsTool::OnCustomCommand(int nId)
{
	if (nId == CID_UPDATEPROGRESS)
		UpdateProgress();
	else if (nId == CMD_PLAYPAUSE)
	{
		HideInstructions();
		
		CMPAutoPtr<IMedia> media;
		if (g_Player.GetIMPlayer()->GetCurrentMedia(&media) != ERR_OK || media == NULL)
		{
			PlayItem(0, false);
		}
		else
			return false;
	}
	else if (nId == CMD_NEXT || nId == CMD_PREVIOUS)
	{
		int media = m_nCurrentMedia + 1;
		if (nId == CMD_PREVIOUS)
			media = m_nCurrentMedia - 1;
		if (m_pMediaLib != NULL && media > 0 && media < m_pMediaLib->GetRowCount())
		{
			m_filesListCtrl->ClearAllSelMark();
			m_pMediaLib->SetRowSelectionState(media, true);
			m_filesListCtrl->MakeSureRowVisible(media);
			m_filesListCtrl->Invalidate();

			PlayItem(media, true);
			HideInstructions();
		}
	}
	else if (nId == CMD_IMPORT)
	{
		OnCommand(IDC_IMPORT_ITUNES_LIB);

		if (m_pMediaLib->GetRowCount() == 0)
			OnCommand(IDC_ADD_FILES);
	}
	else if (nId == CMD_DL_LYRICS)
		OnCommand(IDC_DOWNLOAD_ALL_LYR_FOR_IPOD);
	else if (nId == CID_CANCEL_WORK)
	{
		if (m_workStatus.IsWorking())
			m_workStatus.Cancel();
		else
			m_pSkin->StartAnimation(GetIDByName(_T("CID_AM_HIDE_WORKING")));
	}
	else
		return CSkinContainer::OnCustomCommand(nId);

	return true;
}

bool CPageLyricsTool::OnOK()
{
	if (m_WorkLooper.IsIdle())
	{
		CSkinContainer::OnOK();
	}

	return false;
}

bool CPageLyricsTool::OnCancel()
{
	if (!m_WorkLooper.IsIdle())
	{
		// Cancel.
		if (m_workStatus.IsWorking())
			m_workStatus.Cancel();

		m_pSkin->SetUIObjectText(CID_STATUS, _TLT("Canceling is in progress, please wait..."), true);
	}

	return true;
}

void CPageLyricsTool::UpdateProgress()
{
	bool	bRedrawListCtrl = false;
	tstring	strMsg;

	m_mutexMsg.Acquire();
	strMsg = m_strMsg;
	m_strMsg.resize(0);

	if (m_strStatus.size())
	{
		m_pSkin->SetUIObjectText(CID_STATUS, m_strStatus.c_str(), false);
		m_strStatus.clear();
	}
	if (m_nProgress != -1)
		m_pProgressCtrl->SetScrollPos(m_nProgress, false);

	for (ListUpdateItem::iterator it = m_listUpdateItems.begin(); it != m_listUpdateItems.end(); ++it)
	{
		UpdateItem &item = *it;
		m_pMediaLib->SetResult(item.n, item.strResult.c_str());
	}
	bRedrawListCtrl = m_listUpdateItems.size() > 0;
	m_listUpdateItems.clear();

	m_mutexMsg.Release();

	if (bRedrawListCtrl)
		m_pSkin->InvalidateRect();
	else
	{
		m_pSkin->InvalidateUIObject(CID_STATUS);
		m_pProgressCtrl->Invalidate();
	}

	if (m_nProgress == 100 || m_nProgress == -1)
	{
		// DONE, show message and hide working area.
		if (strMsg.size())
			m_pSkin->MessageOut(strMsg.c_str());

		m_pSkin->SetUIObjectText(CID_CANCEL_WORK, _T("OK"), true);
	}
}

void CPageLyricsTool::ClearResult()
{
	for (int i = 0; i < m_filesListCtrl->GetRowCount(); i++)
	{
		m_pMediaLib->SetItemImageIndex(i, IMG_NONE, false);
		m_pMediaLib->SetResult(i, _T(""));
	}
}

void CPageLyricsTool::FromCurrentPlaylist()
{
	CMPAutoPtr<IPlaylist> playlist;

	MLRESULT ret = g_Player.GetCurrentPlaylist(&playlist);
	if (ret != ERR_OK)
		return;

	int count = playlist->GetCount();
	if (count == 0)
		return;

	ChangeToMediaLib(MST_FILES);

	for (int i = 0; i < count; i++)
	{
		CMPAutoPtr<IMedia> media;
		tstring location;

		ret = playlist->GetItem(i, &media);
		if (ret != ERR_OK)
			return;

		CXStr url;
		if (media->GetSourceUrl(&url) != ERR_OK)
			return;

		CFileMediaSource *pFileMediaLib = (CFileMediaSource *)m_pMediaLib;
		pFileMediaLib->AddFile(url.c_str());
	}
}

void CPageLyricsTool::UpdateNewMediaList()
{
	if (!m_pMediaLib)
		return;

	CUIObject *pObjInst = m_pSkin->GetUIObjectById(UID_INSTRUCTION);
	if (pObjInst != NULL && pObjInst->IsVisible())
	{
		pObjInst->SetProperty(SZ_PN_TEXT, _TLM("Please click here to download all lyrics."));
		pObjInst->SetProperty(SZ_PN_LINK, _T("cmd://CMD_DL_LYRICS"));
		pObjInst->GetParent()->RecalculateUIObjSizePos(pObjInst);
	}

	m_filesListCtrl->NotifyDataChanged();
}

void CPageLyricsTool::OnAdvDownloadAllLyrics()
{
//	ClearResult();
//
//	if (!m_pMediaLib || m_pMediaLib->GetRowCount() == 0)
//	{
//		m_pSkin->MessageOut(_TLT("Please add media files in the list."));
//		return;
//	}
//
//	if (!g_licenseManager.IsLicenseValid())
//	{
//		m_pSkin->MessageOut(_TLT("Sorry, please activate your MiniLyrics to continue."));
//		ShowActivateDialog(m_pSkin);
//		return;
//	}
//
//	CDlgAdvDownloadPrompt	dlg;
//
//	if (dlg.DoModal(m_pSkin) != IDOK)
//		return;
//
//	m_pSkin->SetUIObjectText(CID_STATUS, _TLT("Downloading lyrics is in progress..."), true);
//
//	CAdvDownloadLyrWork *pWork = new CAdvDownloadLyrWork(m_pMediaLib, this, m_workStatus);
//	pWork->SetInfo(dlg.m_downSaveDir, dlg.m_downSaveName, dlg.m_bOnlyUseLocalLyrics, dlg.m_vSaveEmbeddedLyrNames);
//	PostWork(pWork);
}

void CPageLyricsTool::OnAdvRemoveAllLyrics()
{
//	ClearResult();
//
//	if (!m_pMediaLib || m_pMediaLib->GetRowCount() == 0)
//	{
//		m_pSkin->MessageOut(_TLT("Please add media files in the list."));
//		return;
//	}
//
//	CDlgAdvRemovePrompt	dlg;
//
//	if (dlg.DoModal(m_pSkin) != IDOK)
//		return;
//
//	int nRet = m_pSkin->MessageOut(_TLT("Are you sure that you want to remove the embedded lyrics of all the files?"), MB_ICONQUESTION | MB_OKCANCEL);
//	if (nRet != IDOK)
//		return;
//
//	m_pSkin->SetUIObjectText(CID_STATUS, _TLT("Removing embedded lyrics in media files is in progress..."), true);
//
//	CAdvRemoveLyrics *pWork = new CAdvRemoveLyrics(m_pMediaLib, this, m_workStatus);
//	pWork->SetInfo(dlg.m_bRemoveAssociateLyrFile, dlg.m_vEmbeddedLyrNames);
//	PostWork(pWork);
}

void CPageLyricsTool::OnDownloadLyricsForiPod()
{
	ClearResult();

	if (!m_pMediaLib || m_pMediaLib->GetRowCount() == 0)
	{
		m_pSkin->MessageOut(_TLT("Please add media files in the list."));
		return;
	}

	if (!g_licenseManager.IsLicenseValid())
	{
		m_pSkin->MessageOut(_TLT("Sorry, please activate your MiniLyrics to continue."));
		ShowActivateDialog(m_pSkin);
		return;
	}

	m_pSkin->SetUIObjectText(CID_STATUS, _TLT("Downloading lyrics for iPod is in progress..."), true);

	PostWork(new CSaveLyricsInITunes(m_pMediaLib, this, m_workStatus));
}

void CPageLyricsTool::OnRemoveEmbeddedLyrics()
{
	ClearResult();

	if (!m_pMediaLib || m_pMediaLib->GetRowCount() == 0)
	{
		m_pSkin->MessageOut(_TLT("Please add media files in the list."));
		return;
	}

	int nRet = m_pSkin->MessageOut(_TLT("Are you sure that you want to remove the embedded lyrics of all the files?"), MB_ICONQUESTION | MB_OKCANCEL);
	if (nRet != IDOK)
		return;

	m_pSkin->SetUIObjectText(CID_STATUS, _TLT("Removing embedded lyrics in media files is in progress..."), true);

	PostWork(new CRemoveEmbeddedLyrics(m_pMediaLib, this, m_workStatus));
}

void CPageLyricsTool::PostWork(ILyricsToolWork *pWork)
{
	if (!m_WorkLooper.IsIdle())
	{
		DBGLOG0(_T("Working looper is NOT idle now. Can't post work."));
		return;
	}

	m_pProgressCtrl->SetScrollPos(0, false);

	m_pSkin->SetUIObjectText(CID_CANCEL_WORK, _T("Cancel"), false);
	m_pSkin->StartAnimation(GetIDByName(_T("CID_AM_SHOW_WORKING")));

	CRunnableDoWork	*pRunnable = new CRunnableDoWork(this, pWork);
	m_WorkLooper.PostFreeAfterRun(pRunnable);
}

void CPageLyricsTool::DoWork(ILyricsToolWork *pWork)
{
	if (!m_pMediaLib)
		return;

	m_nProgress = 0;

	pWork->OnBegin();
	int nCount = m_pMediaLib->GetRowCount();
	for (int i = 0; i < nCount; i++)
	{
		if (m_workStatus.IsCancel())
			break;

		pWork->OnProcessItem(i);

		m_nProgress = i * 100 / nCount;
		if (m_nProgress != 100)
			m_pSkin->PostCustomCommandMsg(CID_UPDATEPROGRESS);
	}

	tstring	strMsg;
	pWork->OnEnd(strMsg);

	m_nProgress = 100;

	m_mutexMsg.Acquire();
	m_strStatus = _TLT("Finished.");
	m_strMsg = strMsg;
	if (pWork)
	{
		delete pWork;
		pWork = NULL;
	}
	m_mutexMsg.Release();

	m_pSkin->PostCustomCommandMsg(CID_UPDATEPROGRESS);
}

uint32 CPageLyricsTool::GetEmbeddedLyricsOpt()
{
	uint32		uEmbeddedLyricsTypes = 0;

	if (IsButtonChecked(_T("CID_C_ID3V2_SYLT")))
		uEmbeddedLyricsTypes |= LST_ID3V2_SYLT;
	if (IsButtonChecked(_T("CID_C_ID3V2_USLT")))
		uEmbeddedLyricsTypes |= LST_ID3V2_USLT;
	if (IsButtonChecked(_T("CID_C_LYR3V2")))
		uEmbeddedLyricsTypes |= LST_LYRICS3V2;

	return uEmbeddedLyricsTypes;
}

void RegisterLyricsToolPage(CSkinFactory *pSkinFactory)
{
	AddUIObjNewer2(pSkinFactory, CPageLyricsTool);
}
