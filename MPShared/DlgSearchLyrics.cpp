// DlgSearchLyrics.cpp: implementation of the CDlgSearchLyrics class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "LyricsDownloadMgr.h"
#include "DlgSearchLyrics.h"
#include "RegistrationDlg.h"
#include "OnlineSearch.h"
#include "PreferencePages.h"
#include "WaitingDlg.h"
#include "Helper.h"
#include "ErrorResolveDlg.h"
#include <set>


//////////////////////////////////////////////////////////////////////
// CRenameDlg
class CRenameDlg : public CBaseDialog
{
public:
	BOOL OnInitDialog();
	void OnOK();
	CRenameDlg(): CBaseDialog(IDD_RENAME){
	}
	virtual ~CRenameDlg(){
	}

public:
	void UpdateListBoxHorzExtent(LPCTSTR szStrInsert);

	tstring		m_strTitle;
	tstring		m_strStaticText;
	tstring		m_strName;
};

BOOL CRenameDlg::OnInitDialog()
{
	if (m_strStaticText.size())
		SetDlgItemText(IDC_STATIC1, m_strStaticText.c_str());
	if (m_strTitle.size())
		SetWindowText(m_strTitle.c_str());

	CBaseDialog::OnInitDialog();

	if (m_strName.size())
		SetDlgItemText(IDC_NAME, m_strName.c_str());

	return TRUE;
}

void CRenameDlg::OnOK()
{
	TCHAR	szNewName[256];

	GetDlgItemText(IDC_NAME, szNewName, 256);
	m_strName = szNewName;

	if (m_strName.size() == 0)
		return;

	CBaseDialog::OnOK();
}

// CRenameDlg
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

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDlgSearchLyrics::CDlgSearchLyrics() : CBaseDialog(IDD_LRC_SEARCH)
{
#ifdef _WIN32_DESKTOP
	if (!IsRegistered() && !g_bEngEdition)
		m_nIDTemplate = IDD_LRC_SEARCH;
#endif
}

CDlgSearchLyrics::~CDlgSearchLyrics()
{

}

BOOL CDlgSearchLyrics::OnNotify(WIDGET_NOTIFY_INFO *info)
{
	if (info->nCtrlID == IDC_RESULT_LIST)
	{
		if (info->nCode == WN_GEN_DBL_CLICK)
			OnOK();
	}

	return TRUE;
}

void CDlgSearchLyrics::OnCommand(UINT uID, UINT nNotifyCode)
{
	CBaseDialog::OnCommand(uID, nNotifyCode);

	// from menu or accelerator
	switch (uID)
	{
#ifdef _WIN32_DESKTOP
	case IDC_WEB_SEARCH:
		{
			if (m_ad.adType == CAdItem::ADT_SEARCH)
			{
				TCHAR		szUrl[512], szInet[512];
				TCHAR		szKeyword[256];
				GetDlgItemText(IDC_E_KEYWORD, szKeyword, 256);
				_tcscpy(szUrl, m_ad.strUrl.c_str());
				_tcscat(szUrl, szKeyword);
				UriEscape(szUrl, szInet, 512);
				OpenUrl(this, szInet);
			}
			else
			{
				OpenUrl(this, m_ad.strLink.c_str());
			}
		}
		break;
	case IDC_CB_ARTIST:
	case IDC_CB_TITLE:
		{
			TCHAR		szArtist[256], szTitle[256];
			TCHAR		szKeyword[256];
			GetDlgItemText(IDC_CB_ARTIST, szArtist, CountOf(szArtist));
			GetDlgItemText(IDC_CB_TITLE, szTitle, CountOf(szTitle));
			_stprintf(szKeyword, _T("%s %s"), szArtist, szTitle);
			SetDlgItemText(IDC_E_KEYWORD, szKeyword);
		}
		break;
#endif
	case IDC_NEXT_PAGE:
		{
			if (m_nCurPage >= m_nPageCount - 1)
				return;
			m_nCurPage++;

			if (m_nCurPage < (int)m_vCachePages.size())
				m_vSearchResult = m_vCachePages[m_nCurPage];
			else
				SearchOnline();

			UpdateSearchResultToListCtrl();

			// 设置缺省选中的一项为第一项
			m_resultList.SetItemSelectionState(0, TRUE);

			EnableDlgItem(IDC_NEXT_PAGE, m_nCurPage < m_nPageCount - 1);
			EnableDlgItem(IDC_PREV_PAGE, m_nCurPage > 0);
		}
		break;
	case IDC_PREV_PAGE:
		{
			if (m_nCurPage <= 0)
				return;
			m_nCurPage--;

			ASSERT(m_nCurPage < (int)m_vCachePages.size());
			m_vSearchResult = m_vCachePages[m_nCurPage];

			UpdateSearchResultToListCtrl();

			// 设置缺省选中的一项为第一项
			m_resultList.SetItemSelectionState(0, TRUE);

			EnableDlgItem(IDC_NEXT_PAGE, m_nCurPage < m_nPageCount - 1);
			EnableDlgItem(IDC_PREV_PAGE, m_nCurPage > 0);
		}
		break;
	case IDC_NOT_MATCH:
		{
			if (IsEmptyString(m_szAssociateKeyword))
				return;

			CBaseDialog::OnOK();

			g_LyricSearch.AssociateLyrics(m_szAssociateKeyword, NONE_LYRCS);
			g_mpApp.ResearchLyrics();
		}
		break;
	case IDC_SEARCH:
		{
			TCHAR		szArtist[256];
			TCHAR		szTitle[256];

			GetDlgItemText(IDC_CB_ARTIST, szArtist, CountOf(szArtist));
			GetDlgItemText(IDC_CB_TITLE, szTitle, CountOf(szTitle));
			m_strArtistLastSearch = szArtist;
			m_strTitleLastSearch = szTitle;
			m_nCurPage = 0;
			m_nPageCount = 1;

			m_vCachePages.clear();

			OnSearch();
		}
		break;
	case IDC_NO_PROMPT:
		// 如果存在多首匹配的歌词，则提示用户来选择
		g_Profile.WriteProfileInt(SZ_SECT_LYR_DL, _T("DownLrcUserSelect"), 
			!IsButtonChecked(IDC_NO_PROMPT));
		break;
	case IDC_MORECMD:
		{
			CRect		rc;

			bool		bEnable;

			bEnable = GetCurSelLyricsFile() && IsCurSelLyricsAFile();

			m_menu.EnableItem(IDC_EDIT_LYRICS, bEnable);
			m_menu.EnableItem(IDC_DEL_FILE, bEnable);
			m_menu.EnableItem(IDC_RENAME, bEnable);
			GetDlgItemRect(IDC_MORECMD, &rc);
			m_menu.TrackPopupMenu(rc.left, rc.top, this);
		}
		break;
	case IDC_RENAME:
		{
			if (GetCurSelLyricsFile() && IsCurSelLyricsAFile())
			{
				CRenameDlg	dlg;
				TCHAR		szFileNew[MAX_PATH];

				FileGetTitle(m_szLyric, dlg.m_strName);
				if (dlg.DoModal(this) == IDOK)
				{
					FileGetPath(szFileNew, CountOf(szFileNew), m_szLyric);
					_tcscat(szFileNew, dlg.m_strName.c_str());
					_tcscat(szFileNew, FileGetExt(m_szLyric));
					if (!MoveFile(m_szLyric, szFileNew))
					{
						MessageOut(TOLOCAL(_T("Rename File FAILED!")));
					}
					else
					{
						m_listLyrics.SetItemText(m_nSelectedItem, COL_FILE_PATHNAME, szFileNew);
						m_listLyrics.SetItemText(m_nSelectedItem, COL_FILE_NAME, FileGetName(szFileNew));
					}
				}
			}
		}
		break;
	case IDC_DEL_FILE:
		{
			if (GetCurSelLyricsFile() && IsCurSelLyricsAFile())
			{
				SHDeleteFile(m_szLyric, this);
				if (!IsFileExist(m_szLyric))
				{
					m_listLyrics.DeleteItem(m_nSelectedItem);
				}
			}
		}
		break;
	case IDC_EDIT_LYRICS:
		//
		// 编辑歌词
		//
		{
			if (GetCurSelLyricsFile() && IsCurSelLyricsAFile())
			{
				tstring		strEditor;
				GetNotepadEditor(strEditor);
				Execute(this,
					CMLProfile::GetDir(SZ_SECT_UI, _T("LyricsEditor"), strEditor.c_str()).c_str(),
					m_szLyric);
			}
		}
		break;
	case IDC_SEARCH:
		{
			OnSearch();
		}
		break;
	case IDC_BROWSE:
		{
			if (!GetCurSelLyricsFile() || !IsFileExist(m_szLyric))
				EmptyStr(m_szLyric);

			// 设置当前目录
			CFileOpenDlg	dlg(TOLOCAL(_T("Open Lyrics file")), m_szLyric, SZ_SUPPORTED_FILE_TYPE, 1);

			if (dlg.DoModal(this) == IDOK)
			{
				_tcscpy_safe(m_szLyric, CountOf(m_szLyric), dlg.GetOpenFile());
				m_nSelectedItem = m_listLyrics.InsertItem(m_listLyrics.GetItemCount(), FileGetName(m_szLyric));
				m_listLyrics.SetItemText(m_nSelectedItem, COL_FILE_PATHNAME, m_szLyric);
				m_listLyrics.SetItemSelectionState(m_nSelectedItem, true);

				OnOK();
			}
		}
		break;
	default:
		break;
	}
}

BOOL CDlgSearchLyrics::OnInitDialog()
{
	CBaseDialog::OnInitDialog();

	m_wndHelper.Init(this);

	m_wndHelper.Add(IDC_CB_ARTIST, TRUE, FALSE, FALSE, TRUE);
	m_wndHelper.Add(IDC_CB_TITLE, TRUE, FALSE, FALSE, TRUE);
	m_wndHelper.Add(IDC_ONLINE_SEARCH, TRUE, FALSE, TRUE, TRUE);
	m_wndHelper.Add(IDC_RESULT_LIST, TRUE, TRUE, FALSE, FALSE);
	m_wndHelper.Add(IDOK, FALSE, TRUE, TRUE, TRUE);
	m_wndHelper.Add(IDCANCEL, FALSE, TRUE, TRUE, TRUE);
	m_wndHelper.AddAlignRightBottom(IDC_NEXT_PAGE, true, true);
	m_wndHelper.AddAlignRightBottom(IDC_PREV_PAGE, true, true);

	m_wndHelper.AddAlignCenter(IDOK, false);
	m_wndHelper.AddAlignCenter(IDC_NOT_MATCH, false);
	m_wndHelper.AddAlignCenter(IDCANCEL, false);

#ifdef _WIN32_DESKTOP
	if (m_nIDTemplate == IDD_LRC_DOWNLOAD)
	{
		m_wndHelper.Add(IDC_E_KEYWORD, TRUE, FALSE, FALSE, TRUE);
		m_wndHelper.Add(IDC_S_AD, TRUE, FALSE, FALSE, TRUE);
		m_wndHelper.Add(IDC_WEB_SEARCH, TRUE, FALSE, TRUE, TRUE);
	}
	m_wndHelper.Add(IDC_NO_PROMPT, FALSE, TRUE, TRUE, TRUE);
	m_wndHelper.Add(IDC_MLHELP, TRUE, FALSE, TRUE, TRUE);

	{
		//
		// 记住上次保存的窗口大小
		//
		CRect	rc, rcRestrict;
		int		cx, cy;

		GetWindowRect(&rc);
		cx = g_Profile.GetProfileInt(_T("LrcDownSelDlg_width"), rc.Width());
		cy = g_Profile.GetProfileInt(_T("LrcDownSelDlg_height"), rc.Height());
		if (cx <= 450)
			cx = 450;
		if (cy <= 300)
			cy = 300;
		rc.right = rc.left + cx;
		rc.bottom = rc.top + cy;
		GetMonitorRestrictRect(rc, rcRestrict);

		if (cx > rcRestrict.Width() + 15)
			cx = rcRestrict.Width();
		if (cy > rcRestrict.Height() + 15)
			cy = rcRestrict.Height();

		MoveWindow(rc.left, rc.top, cx, cy, FALSE);
	}

	if (!g_bEngEdition && !IsRegistered() && g_AdManage.GetCurAd(ADB_SEARCHWND, m_ad))
	{
		HDC			hdc;

		if (m_ad.adType == CAdItem::ADT_SEARCH)
		{
			// 搜索引擎广告
			ShowDlgItem(IDC_S_AD, SW_SHOW);
			ShowDlgItem(IDC_S_KEYWORD, SW_SHOW);
			ShowDlgItem(IDC_E_KEYWORD, SW_SHOW);
			ShowDlgItem(IDC_WEB_SEARCH, SW_SHOW);
			if (m_ad.strContent.c_str())
				SetDlgItemText(IDC_WEB_SEARCH, m_ad.strContent.c_str());
		}
		else if (m_ad.adType == CAdItem::ADT_WEB || m_ad.adType == CAdItem::ADT_WEBSTR)
		{
			CRect		rcClient;
			int			xMargin;

			GetClientRect(&rcClient);
			xMargin = (rcClient.Width() - m_ad.nWidth - 2) / 2;
			if (m_wndWeb.Create(CRect(xMargin, 10, rcClient.right - xMargin, 10 + m_ad.nHeight + 4), this))
			{
				if (m_ad.adType == CAdItem::ADT_WEB)
				{
					tstring		str, strUrl;

					ParseMLTextMacro(m_ad.strUrl.c_str(), str);
					UriEscape(str.c_str(), strUrl);
					m_wndWeb.DisplayHTMLPage(strUrl.c_str());
				}
				else if (m_ad.adType == CAdItem::ADT_WEBSTR)
					m_wndWeb.DisplayHTMLStr(m_ad.strContent.c_str());
			}
		}
		else if (m_ad.adType == CAdItem::ADT_IMG)
		{
			m_picture.Create(_T(""), 0, 0, 300, 100, this, WS_CHILD);
			m_picture.ShowWindow(SW_SHOW);

			hdc = GetDC(m_hWnd);
			m_picture.SetUrl(m_ad.strLink.c_str());
			if (m_picture.Load(m_ad.strImgFile.c_str(), hdc))
			{
				m_picture.AutoResize();
				m_picture.BeginDraw();
			}
			ReleaseDC(m_hWnd, hdc);
			// m_picture.Load(MAKEINTRESOURCE(IDR_GIF1), _T("GIF"));

			CRect		rcClient, rc;
			int			nOffset;
			::GetWindowRect(m_picture.GetHandle(), &rc);
			GetClientRect(&rcClient);
			rc.bottom = rc.Height() + 10;
			rc.top = 10;
			nOffset = (rcClient.Width() - rc.Width()) / 2;
			rc.right = rc.Width() + nOffset;
			rc.left = nOffset;
			m_picture.MoveWindow(rc.left, rc.top, rc.Width(), rc.Height(), TRUE);
		}
	}
#endif

	m_cbTitle.Attach(this, IDC_CB_TITLE);
	m_cbArtist.Attach(this, IDC_CB_ARTIST);

	m_resultList.Attach(this, IDC_RESULT_LIST);
	m_resultList.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	LPCTSTR		szHead[] = {_T("File Name"), _T("Rating"), _T("Downloads"), _T("Uploader"), _T("Artist"), _T("Title"), _T("Album"), NULL};
	int			arrWidth[] = {300,        80,              40,              150,        150,      150,    150,     200,   0};
	int			i;
	for (i = 0; szHead[i] != NULL; i++)
		m_resultList.InsertColumn(i, TOLOCAL(szHead[i]), LVCFMT_LEFT, arrWidth[i]);
	if (g_Profile.GetProfileInt(_T("ShowLink"), false))
		m_resultList.InsertColumn(i, _T("URL"), LVCFMT_LEFT, 200);

	m_resultList.LoadColumnWidth(_T("LrcDownSelDlg_Files"));

#ifdef _WIN32
	// 创建ImageList
	m_imgList.LoadBitmap(IDB_LRC_TYPE, 16, RGB(255, 0, 255));

	m_resultList.SetImageList(m_imgList.GetHandle(), LVSIL_SMALL);
#endif

	if (g_bEngEdition)
		ShowDlgItem(IDC_OFFLINE_SEARCH, SW_HIDE);

	g_mpApp.GetAssociateFileKeyword(m_szAssociateKeyword, CountOf(m_szAssociateKeyword));

	//
	// 在对话框中显示所有可以下载的歌词

	TCHAR	szArtist[256];
	TCHAR	szTitle[256];
	//
	// 根据文件名字重新搜索
	TCHAR	szSongName[MAX_PATH];

	_tcscpy_safe(szSongName, CountOf(szSongName), FileGetName(g_Player.GetSrcMedia()));

	std::set<tstring>	setArTi;

	setArTi.insert(szSongName);
	setArTi.insert(g_Player.GetFullTitle());

	AnalysisLyricsFileNameEx(szArtist, szTitle, g_Player.GetFullTitle());
	setArTi.insert(szArtist);
	setArTi.insert(szTitle);

	AnalysisLyricsFileNameEx(szArtist, szTitle, szSongName);
	setArTi.insert(szArtist);
	setArTi.insert(szTitle);

	setArTi.insert(g_Player.GetArtist());
	setArTi.insert(g_Player.GetTitle());

	std::set<tstring>::iterator	it, itEnd;

	itEnd = setArTi.end();
	for (it = setArTi.begin(); it != itEnd; ++it)
	{
		m_cbArtist.AddString((*it).c_str());
		m_cbTitle.AddString((*it).c_str());
	}

	// 先根据IDTag3中的歌曲名搜索
	g_OnlineSearch.SearchCacheForCur(&m_vSearchResult);

	//
	// 搜索出来啦，添加到列表中去
	if (m_vSearchResult.size())
	{
		UpdateSearchResultToListCtrl();

		// 设置缺省选中的一项为第一项
		m_resultList.SetItemSelectionState(0, TRUE);
	}

	m_cbTitle.SelectString(-1, g_Player.GetTitle());
	m_cbArtist.SelectString(-1, g_Player.GetArtist());

	// 如果存在多首匹配的歌词，则提示用户来选择
	CheckButton(IDC_NO_PROMPT, !g_Profile.GetProfileBool(SZ_SECT_LYR_DL, _T("DownLrcUserSelect"), true));

	CeSHInitFullScrDlg(this);

	m_cmdBar.Create(this, -1);

	EnableDlgItem(IDC_NEXT_PAGE, false);
	EnableDlgItem(IDC_PREV_PAGE, false);

	return TRUE;
}

void CDlgSearchLyrics::OnDestroy()
{
#ifdef _WIN32_DESKTOP
	WINDOWPLACEMENT		wndPlacement;

	wndPlacement.length = sizeof(wndPlacement);
	GetWindowPlacement(m_hWnd, &wndPlacement);
	g_Profile.WriteProfileInt(_T("LrcDownSelDlg_width"), CRect(wndPlacement.rcNormalPosition).Width());
	g_Profile.WriteProfileInt(_T("LrcDownSelDlg_height"), CRect(wndPlacement.rcNormalPosition).Height());
#endif
	m_cmdBar.Destroy();

	m_resultList.SaveColumnWidth(_T("LrcDownSelDlg_Files"));

	CBaseDialog::OnDestroy();

	m_LangDlgHelper.Destroy();
}

void CDlgSearchLyrics::OnSize(int cx, int cy)
{
	m_wndHelper.OnResize();
	InvalidateRect(NULL, TRUE);
}

void CDlgSearchLyrics::OnSearch()
{
	SearchLocal();
	SearchOnline();
}

void CDlgSearchLyrics::OnOK()
{
	int 		nPos;
	int			nCount;

	nCount = m_resultList.GetSelectedCount();
	if (nCount <= 0)
		return;

	//
	// 将下载连接添加到任务队列中
	nPos = m_resultList.GetNextSelectedItem(-1);
	if (nPos != -1)
	{
		TCHAR			szFileName[MAX_PATH] = _T("");
		LrcSearchResult	*pSel;
		int				nError;

		ASSERT(nPos < (int)m_vSearchResult.size());
		pSel = m_vSearchResult.GetByOrder(nPos);
		if (!pSel)
			return;

		m_resultList.GetItemText(nPos, COL_FILENAME, szFileName, CountOf(szFileName));

		nError = g_LyricsDownloader.AddLyricsDownloadTask(m_szAssociateKeyword, 
			pSel->strUrl.c_str(), szFileName);
		if (nError != ERR_OK)
		{
			TCHAR	szLocal[MAX_PATH];

			UriUnescape(pSel->strUrl.c_str(), szLocal, MAX_PATH);
			ERRLOG2(_T("FAILED to Add Task: %s, %s"), (LPCTSTR)Error2Str(nError), szLocal);
		}
	}

	CBaseDialog::OnOK();
}

void CDlgSearchLyrics::UpdateSearchResultToListCtrl()
{
	int		nItem;
	TCHAR	szTemp[256];

	m_resultList.DeleteAllItems();
	for (V_LRCSEARCHRESULT::iterator it = m_vSearchResult.begin(); it != m_vSearchResult.end(); it++)
	{
		LrcSearchResult	&Item = *it;

		int		nImg;
		MLFileType		fileType;

		fileType = MLGetFileType(Item.strSaveFileName.c_str());
		if (fileType == FT_LYRICS_LRC)
			nImg = 1;
		else if (fileType == FT_LYRICS_TXT)
			nImg = 0;
		else
			nImg = 2;

		nItem = m_resultList.InsertItem(m_resultList.GetItemCount(), Item.strSaveFileName.c_str(), nImg);
		if (Item.fRate != 0.0)
		{
			_stprintf(szTemp, _T("%.1f/5 (%d %s)"), Item.fRate, Item.nRateCount, TOLOCAL(_T("Votes")));
		}
		else
			_tcscpy(szTemp, _T("-"));
		m_resultList.SetItemText(nItem, COL_RATING, szTemp);

		_itot(Item.nDownloads, szTemp, 10);
		m_resultList.SetItemText(nItem, COL_DOWNLOADS, szTemp);

		m_resultList.SetItemText(nItem, COL_UPLOADER, Item.strUploader.c_str());
		m_resultList.SetItemText(nItem, COL_ARTIST, Item.strArtist.c_str());
		m_resultList.SetItemText(nItem, COL_TITLE, Item.strTitle.c_str());
		m_resultList.SetItemText(nItem, COL_ALBUM, Item.strAlbum.c_str());
		m_resultList.SetItemText(nItem, COL_URL, Item.strUrl.c_str());
	}
}

struct CLyricsResultSortCmp
{
public:
	bool operator()(const LrcSearchResult &item1, const LrcSearchResult &item2)
	{
		return _tcsicmp(item1.strTitle.c_str(), item2.strTitle.c_str()) < 0;
	}
};

void CDlgSearchLyrics::SearchLocal()
{
	tstring			strFullTitle;
	TCHAR			szArtist[256];
	TCHAR			szTitle[256];
	TCHAR			szLyricsFile[MAX_PATH];

	GetDlgItemText(IDC_CB_ARTIST, szArtist, CountOf(szArtist));
	GetDlgItemText(IDC_CB_TITLE, szTitle, CountOf(szTitle));

	strFullTitle = szArtist;
	strFullTitle += _T(" - ");
	strFullTitle += szTitle;

	{
		//
		// Add "No suitable lyrics"
		//
		LrcSearchResult		result;
		result.strUrl = NONE_LYRCS;
		result.strSaveFileName = TOLOCAL(_T("No suitable lyrics for the song file"));
		result.nMatchValue = MATCH_VALUE_WITH_LRC - 5;
		m_vSearchResult.AddResult(result);
	}

	g_LyricSearch.SearchAllMatchLyrics(g_Player.GetSrcMedia(), strFullTitle.c_str(), m_vSearchResult);

	//
	// Set best match lyrics
	//
	if (g_LyricSearch.GetBestMatchLyrics(g_Player.GetSrcMedia(), g_Player.GetArtist(), g_Player.GetTitle(), szLyricsFile))
	{
		LrcSearchResult	*pResult;
		pResult = m_vSearchResult.SearchByUrl(szLyricsFile);
		if (pResult)
		{
			pResult->nMatchValue = MATCH_VALUE_MAX;
		}
		else
		{
			LrcSearchResult		result;
			result.strUrl = szLyricsFile;
			result.nMatchValue = MATCH_VALUE_MAX;
			m_vSearchResult.AddResult(result);
		}
	}

	m_vSearchResult.UpdateLyricsName();
}

void CDlgSearchLyrics::SearchOnline()
{
	if (m_strArtistLastSearch.empty() && m_strTitleLastSearch.empty())
		return;

	if (m_strTitleLastSearch.empty())
		ShowRegisterNotice(this, false);

	CWaitingDlg		dlg;
	CSearchWorkObj	searchObj;

	m_resultList.DeleteAllItems();

	searchObj.m_strTitle = m_strTitleLastSearch;
	searchObj.m_strArtist = m_strArtistLastSearch;
	searchObj.m_pvSearchResult = &m_vSearchResult;
	searchObj.m_nCurPage = m_nCurPage;

	// LOG0(LOG_LVL_EVENT_IMP, _T("Begin Online search."));

	if (dlg.ShowWaitingDlg(this, &searchObj, 
		TOLOCAL(_T("Searching Lyrics")), 
		TOLOCAL(_T("MiniLyrics is searching lyrics now, it may take a few seconds."))) != IDOK)
		return;

	if (searchObj.m_strMsg.size())
	{
		MessageOut(searchObj.m_strMsg.c_str());
	}

	if (searchObj.m_nResult == ERR_NOT_FOUND)
	{
		tstring		strMsg;
		strMsg = TOLOCAL(_T("Sorry, no result returned. Please make sure you enter the correct artist name and title."));
		strMsg += _T("\r\n");
		strMsg += TOLOCAL(_T("If you have the lyrics, please upload it to our server to share it with others."));
		MessageOut(strMsg.c_str());
	}
	else if (searchObj.m_nResult != ERR_OK)
		ShowErrorDlg(this, searchObj.m_nResult);

	// LOG2(LOG_LVL_EVENT_IMP, _T("Online search returned: %d, %s"), searchObj.m_nResult, (LPCTSTR)ERROR2STR_LOCAL(searchObj.m_nResult));

	m_nCurPage = searchObj.m_nCurPage;
	m_nPageCount = searchObj.m_nPageCount;

	if (m_nPageCount > 1)
	{
		m_vCachePages.push_back(m_vSearchResult);
	}
	else
	{
		// only one page
		if (m_strTitleLastSearch.empty())
		{
			// search by artist, sort the result by abc order.
			CLyricsResultSortCmp	cmpFun;
			vector<LrcSearchResult>	vTemp;

			vTemp.resize(m_vSearchResult.size());
			std::copy(m_vSearchResult.begin(), m_vSearchResult.end(), vTemp.begin());
			sort(vTemp.begin(), vTemp.end(), cmpFun);
			std::copy(vTemp.begin(), vTemp.end(), m_vSearchResult.begin());
		}
	}

	UpdateSearchResultToListCtrl();

	// 设置缺省选中的一项为第一项
	m_resultList.SetItemSelectionState(0, TRUE);

	EnableDlgItem(IDC_NEXT_PAGE, m_nCurPage < m_nPageCount - 1);
	EnableDlgItem(IDC_PREV_PAGE, m_nCurPage > 0);
}
