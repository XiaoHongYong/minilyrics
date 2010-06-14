#pragma once

#include "IMLAppNotify.h"
#include "LyricsLocalSearch.h"
#include "OnlineSearch.h"
#include "LyricsDownloadMgr.h"

class CMLApp
{
public:
	CMLApp(void);
	virtual ~CMLApp(void);

	static CMLApp *GetInstance();

	int Init();
	void Quit();

	void OnMediaChanged(LPCTSTR szMediaKey, LPCTSTR szArtist, LPCTSTR szAlbum, LPCTSTR szTitle, LPCTSTR szEmbeddedLyr, int nMediaLength);

	virtual void OnLyricsSearchEnd(int nResult, bool &bPopupSearchWnd);
	virtual void OnLyricsDownloaded(LPCTSTR szMediaKey, LPCTSTR szFile);

public:
	CLyricsLocalSearch &GetLocalLyrSearch() { return m_localLyrSearch; }
	COnlineSearch &GetOnlineSearch() { return m_onlineLyrSearch; }
	CLyricsDownloadMgr &GetLyrDownloadMgr() { return m_lyrDownloadMgr; }
	CMLData &GetLyricsData() { return m_lyrData; }

protected:
	BOOL SearchInCacheResult(bool bShowInfoText = false);

protected:
	CLyricsLocalSearch		m_localLyrSearch;
	COnlineSearch			m_onlineLyrSearch;
	CLyricsDownloadMgr		m_lyrDownloadMgr;
	CMLData					m_lyrData;

	// Media information
	tstring					m_strCurMediaKey;
	tstring					m_strArtist, m_strTitle, m_strAlbum;
	int						m_nMediaLength;

	static CMLApp			*m_pAppInstance;

};
