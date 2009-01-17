// DlgSearchLyrics.h: interface for the CDlgSearchLyrics class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_DLGSEARCHLYRICS_H_)
#define _DLGSEARCHLYRICS_H_

#pragma once


#ifdef _WIN32_DESKTOP
#include "win32/ImgUrlCtrl.h"
#endif

#include "AdManage.h"

class CDlgSearchLyrics : public CBaseDialog
{
public:
	enum { COL_FILENAME, COL_RATING, COL_DOWNLOADS, COL_UPLOADER, COL_ARTIST, COL_TITLE, COL_ALBUM, COL_URL};
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

	CDlgSearchLyrics();
	virtual ~CDlgSearchLyrics();

	BOOL OnInitDialog();
	void OnDestroy();

	void OnCommand(UINT uID, UINT nNotifyCode);
	BOOL OnNotify(WIDGET_NOTIFY_INFO *info);

	void OnSize(int cx, int cy);

	void OnTimer(UINT nIDEvent);

protected:
	void OnSearch();

	void OnOK();
	void UpdateSearchResultToListCtrl();
	void AppendItemInResultsList(LrcSearchResult &Item, bool bInetFile);

	void SearchOnline();
	void SearchLocal();

	LrcSearchResult *GetCurSel();

protected:
	CMLWndHelper		m_wndHelper;
	CWidgetListCtrl		m_resultList;

	V_LRCSEARCHRESULT	m_vResultsLocal;
	V_LRCSEARCHRESULT	m_vResultsInet;
	CWidgetComboBox		m_cbTitle;
	CWidgetComboBox		m_cbArtist;

	tstring				m_strArtistLastSearch, m_strTitleLastSearch;
	int					m_nCurPage, m_nPageCount;

	// cache returned result.
	vector<V_LRCSEARCHRESULT>	m_vCachePages;

	TCHAR				m_szAssociateKeyword[MAX_PATH];
	tstring				m_strSongFile, m_strArtist, m_strTitle;

	CCECommandBar		m_cmdBar;

	CMLMenu				m_menu;

	//
	// Auto search local lyrics, if artist/tile changed?
	//
	bool				m_bAutoSearchLocal;


#ifdef _WIN32
	CImgList			m_imgList;
#endif

#ifdef _WIN32_DESKTOP
	//
	// Ad
	//
	CAdItem				m_ad;
	CImgUrlCtrl			m_picture;
	CHtmlWnd			m_wndWeb;
#endif

};

#endif // _DLGSEARCHLYRICS_H_
