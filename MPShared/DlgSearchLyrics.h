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
	CDlgSearchLyrics();
	virtual ~CDlgSearchLyrics();

	BOOL OnInitDialog();
	void OnDestroy();

	void OnCommand(UINT uID, UINT nNotifyCode);
	BOOL OnNotify(WIDGET_NOTIFY_INFO *info);

	void OnSize(int cx, int cy);

	void OnOnlineSearch();
	void OnOK();
	void UpdateSearchResultToListCtrl();

protected:
	CMLWndHelper	m_wndHelper;
	CWidgetListCtrl	m_resultList;

	V_LRCSEARCHRESULT m_vSearchResult;
	CWidgetComboBox	m_cbTitle;
	CWidgetComboBox	m_cbArtist;

	tstring			m_strArtistLastSearch, m_strTitleLastSearch;
	int				m_nCurPage, m_nPageCount;

	// cache returned result.
	vector<V_LRCSEARCHRESULT>	m_vCachePages;

	TCHAR			m_szAssociateKeyword[MAX_PATH];				// ¸èÇúÎÄ¼þÃû
	CCECommandBar	m_cmdBar;

#ifdef _WIN32
	CImgList		m_imgList;
#endif

#ifdef _WIN32_DESKTOP
	CAdItem			m_ad;
	CImgUrlCtrl		m_picture;
	CHtmlWnd		m_wndWeb;
#endif

};

#endif // _DLGSEARCHLYRICS_H_
