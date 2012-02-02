#pragma once

DECLAR_CPPUNIT_TEST_REG(CAutoProcessEmbeddedLyrics)

class CAutoProcessEmbeddedLyrics  
{
public:
	struct Item
	{
		tstring				m_strSongFile;
		UINT				m_uEmbeddedLyrType;		// LST_ID3V2_SYLT, LST_ID3V2_USLT, LST_LYRICS3V2
		bool				m_bRemove;		// true: remove, false: add.
		byte				m_nTryFailed;
		bool				m_bDealOK;

		// save embedded lyrics
		CCharBuffer			m_strLyrics;

		Item()
		{
			m_bDealOK = false;
			m_nTryFailed = 0;
			m_uEmbeddedLyrType = 0;
		}

		bool FromXML(SXNode *pNode);
		void ToXML(CXMLWriter &xmlWriter);
	};
	typedef list<Item>		LIST_ITEMS;

	CAutoProcessEmbeddedLyrics();
	virtual ~CAutoProcessEmbeddedLyrics();

	int SaveEmbeddedLyrics(LPCTSTR szSongFile, LPCTSTR szLyricsFile, CCharBuffer *pBufLyrics, UINT uEmbeddedLyrType = LST_ID3V2_SYLT | LST_ID3V2_USLT | LST_LYRICS3V2, bool bAddToQueueOnFailed = true);
	int RemoveEmbeddedLyrics(LPCTSTR szSongFile, UINT uEmbeddedLyrType = LST_ID3V2_SYLT | LST_ID3V2_USLT | LST_LYRICS3V2, bool bAddToQueueOnFailed = true);
	void RemoveJob(LPCTSTR szSongFile);

	void Init();

	bool OnPostQuit();
	void OnSongChanged();

	static bool CanSaveEmbeddedLyricsInMediaFile(LPCTSTR szFile);

protected:
	friend class CDlgBatchProcessEmbeddedLyrics;
	friend class CTestCaseCAutoProcessEmbeddedLyrics;

	int DealEmbeddedLyrics(Item &item);
	int DealSaveEmbeddedLyrics(Item &item);
	int DealRemoveEmbeddedLyrics(Item &item);

	void SaveJobs();
	void LoadJobs(bool bDeleteFile);

	LIST_ITEMS				m_listJobs;
	Mutex					m_mutex;

};

extern CAutoProcessEmbeddedLyrics		g_autoProcessEmbeddedLyrics;
