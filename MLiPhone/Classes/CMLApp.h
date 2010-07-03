#pragma once

#import "IMLAppNotify.h"
#import "LyricsLocalSearch.h"
#import "OnlineSearch.h"
#import "LyricsDownloadMgr.h"

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
	bool SearchCacheForCurrentMedia(V_LRCSEARCHRESULT &vLrcSearchResult);
	void DownloadLyrics(LPCTSTR szMediaKey, LPCTSTR szUrl, LPCTSTR szFileName);

	LPCTSTR GetArtist() const { return m_strArtist.c_str(); }
	LPCTSTR GetTitle() const { return m_strTitle.c_str(); }
	LPCTSTR GetMediaKey() const { return m_strCurMediaKey.c_str(); }

public:
	CLyricsLocalSearch &GetLocalLyrSearch() { return m_localLyrSearch; }
	COnlineSearch &GetOnlineSearch() { return m_onlineLyrSearch; }
	CLyricsDownloadMgr &GetLyrDownloadMgr() { return m_lyrDownloadMgr; }
	CMLData &GetLyricsData() { return m_lyrData; }

protected:
	BOOL SearchInCacheResult(bool bShowInfoText = false);

	void SetDefaultSettings();
	
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
