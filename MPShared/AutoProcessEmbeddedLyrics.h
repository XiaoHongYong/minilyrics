// AutoProcessEmbeddedLyrics.h: interface for the CAutoProcessEmbeddedLyrics class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AutoProcessEmbeddedLyrics_H__D17BD83B_4582_495C_AECC_FB2CE67E4E19__INCLUDED_)
#define AFX_AutoProcessEmbeddedLyrics_H__D17BD83B_4582_495C_AECC_FB2CE67E4E19__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CAutoProcessEmbeddedLyrics  
{
public:
	struct Item
	{
		tstring				m_strSongFile;
		UINT				m_uEmbeddedLyrType;		// LST_ID3V2_SYLT, LST_ID3V2_USLT, LST_LYRICS3V2
		bool				m_bRemove;		// true: remove, false: add.
		bool				m_bTryFailed;
		bool				m_bDealOK;

		// save embedded lyrics
		tstring				m_strLyricsFile;
		tstring				m_strLyrics;		// if m_strLyricsFile.empty(), this is available.

		// remove embedded lyrics
		vector<string>		m_vSyncLang, m_vUnsyncLang;

		Item()
		{
			m_bDealOK = false;
			m_bTryFailed = false;
			m_uEmbeddedLyrType = 0;
		}
	};
	typedef list<Item>		LIST_ITEMS;

	CAutoProcessEmbeddedLyrics();
	virtual ~CAutoProcessEmbeddedLyrics();

	int SaveEmbeddedLyrics(LPCTSTR szSongFile, LPCTSTR szLyricsFile, LPCTSTR szLyrics, UINT uEmbeddedLyrType = LST_ID3V2_SYLT | LST_ID3V2_USLT | LST_LYRICS3V2, bool bAddToQueueOnFailed = true);
	int RemoveEmbeddedLyrics(LPCTSTR szSongFile, vector<string> &vSyncLang, vector<string> &vUnsyncLang, UINT uEmbeddedLyrType = LST_ID3V2_SYLT | LST_ID3V2_USLT | LST_LYRICS3V2, bool bAddToQueueOnFailed = true);
	void RemoveJob(LPCTSTR szSongFile);

	void OnQuit();
	void OnSongChanged();

protected:
	friend class CDlgBatchProcessEmbeddedLyrics;

	int DealEmbeddedLyrics(Item &item);
	int DealSaveEmbeddedLyrics(Item &item);
	int DealRemoveEmbeddedLyrics(Item &item);

	LIST_ITEMS				m_listJobs;
	Mutex					m_mutex;

};

extern CAutoProcessEmbeddedLyrics		g_autoProcessEmbeddedLyrics;

#endif // !defined(AFX_AutoProcessEmbeddedLyrics_H__D17BD83B_4582_495C_AECC_FB2CE67E4E19__INCLUDED_)
