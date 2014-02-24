#pragma once

namespace LyricsDownloader
{

class WorkStatus
{
public:
	WorkStatus() {
		m_bWorking = false;
		m_bCancel = false;
		m_pNotify = NULL;
	}

	class INotify
	{
	public:
		virtual void Cancel() = 0;

	};

	bool IsCancel() { return m_bCancel; }
	bool IsWorking() { return m_bWorking; }
	void Cancel() { m_bWorking = true; if (m_pNotify) m_pNotify->Cancel(); }
	void SetWorking(bool bWorking) { m_bWorking = bWorking; }

	void SetNotify(INotify *pNotify) { m_pNotify = pNotify; }

	volatile bool		m_bWorking;
	volatile bool		m_bCancel;
	INotify				*m_pNotify;

};

class ILyricsToolWork
{
public:
	enum SuccessStatus
	{
		SS_NONE,
		SS_OK,
		SS_ERROR,
		SS_INFO,
	};

	class IUpdateNotify
	{
	public:
		virtual void OnUpdateItemStatus(int nIndex, SuccessStatus status, LPCTSTR szResult) = 0;
	};

public:
	ILyricsToolWork(IMediaSource *pMediaLib, IUpdateNotify *pUpdateNotify, WorkStatus &status) : 
			m_pMediaLib(pMediaLib), m_pUpdateNotify(pUpdateNotify), m_status(status) {
		m_nLastError = ERR_OK;
	}
	virtual ~ILyricsToolWork() { }

	virtual void OnBegin() { m_nLastError = ERR_OK; }
	virtual void OnProcessItem(int nIndex) = 0;
	virtual void OnEnd(tstring &strMsg) = 0;

	void UpdateItemStatus(int nIndex, SuccessStatus status, LPCTSTR szResult) { m_pUpdateNotify->OnUpdateItemStatus(nIndex, status, szResult); }

protected:
	WorkStatus			&m_status;
	IMediaSource		*m_pMediaLib;
	IUpdateNotify		*m_pUpdateNotify;
	int					m_nLastError;

};

class CLyricsDownloadWork : public ILyricsToolWork, WorkStatus::INotify
{
public:
	CLyricsDownloadWork(IMediaSource *pMediaLib, IUpdateNotify *pUpdateNotify, WorkStatus &status) : ILyricsToolWork(pMediaLib, pUpdateNotify, status)
	{
		m_listSearchItems.reserve(20);
		m_nSavedEmbeddedLyrics = m_nHavedLyrics = 0;
		m_nSavedFileLyrics = 0;
		m_nFailedCount = 0;
		m_nNotFoundCount = 0;
		m_bOnlyUseLocalLyrics = false;
		status.SetNotify(this);
	}

	~CLyricsDownloadWork()
		{ m_status.SetNotify(NULL); }

	virtual void Cancel()
		{ m_session.Close(); }

	virtual void OnProcessItem(int nIndex)
	{
		// Search local lyrics and save to embedded lyrics
		tstring strLyrSrcPath;
		if (m_pMediaLib->GetBestMatchLyrics(nIndex, strLyrSrcPath))
		{
			tstring strMediaSrc = m_pMediaLib->GetLocation(nIndex);

			if (OnLocalLyricsFound(nIndex, strMediaSrc.c_str(), strLyrSrcPath.c_str()) != ERR_NOT_FOUND)
				return;
		}

		if (m_bOnlyUseLocalLyrics)
			return;

		// Add to search
		MLSearchItem	item;
		uint32 nMediaLength = 0;
		m_pMediaLib->GetMediaInfo(nIndex, item.strArtist, item.strAlbum, item.strTitle, item.strMediaFile, nMediaLength);
		item.nMediaLength = nMediaLength;
		item.nIndex = nIndex;

		// Filter spam info in searches.
		if (item.strArtist.empty() || !FilterSpamInfo(item.strArtist, item.strTitle))
		{
			UpdateItemStatus(nIndex, SS_ERROR, _TL3("The artist and title is incomplete"));
			return;
		}

		m_listSearchItems.push_back(item);

		if (m_listSearchItems.size() >= 20)
		{
			m_nLastError = SearchAndDownload();
			if (m_nLastError != ERR_OK)
				return;
		}
	}

	virtual void OnEnd(tstring &strMsg)
	{
		if (m_listSearchItems.size() > 0 && m_nLastError == ERR_OK)
		{
			// Continue, if no error.
			m_nLastError = SearchAndDownload();
		}

		strMsg.clear();
		if (m_nLastError != ERR_OK && !m_status.IsCancel())
		{
			strMsg = _TLT("Error:");
			strMsg += _T(" ");
			strMsg += ERROR2STR_LOCAL(m_nLastError);
			strMsg += _T("\n\n");
		}

		if (m_nFailedCount > 0)
		{
			strMsg += CStrPrintf(_TLT("%d lyrics were failed to save."), m_nFailedCount).c_str();
			strMsg += _T("\n");
		}

		strMsg += CStrPrintf(_TLT("%d lyrics are saved in audio files."), m_nSavedEmbeddedLyrics).c_str();
		strMsg += _T("\n");
		if (m_nSavedFileLyrics > 0)
		{
			strMsg += CStrPrintf(_TLT("%d lyrics files are saved."), m_nSavedFileLyrics).c_str();
			strMsg += _T("\n");
		}
		if (m_nHavedLyrics > 0)
			strMsg += CStrPrintf(_TLT("%d audio files already have lyrics."), m_nHavedLyrics).c_str();

		if (m_nNotFoundCount > 0)
		{
			strMsg += _T("\r\n\r\n");
			strMsg += CStrPrintf(_TLT("Can't find lyrics for %d audio files."), m_nNotFoundCount).c_str();
		}
	}

protected:
	int SearchAndDownload()
	{
		ASSERT(!m_listSearchItems.empty());

		// Search lyrics by batch.
		tstring				strMsg;
		MLMsgRetBatchSearch retLyrics;

		int nRet = g_OnlineSearch.BatchSearch(m_session, m_listSearchItems, retLyrics);
		if (nRet != ERR_OK)
		{
			for (int i = 0; i < m_listSearchItems.size(); i++)
			{
				MLSearchItem &search = m_listSearchItems[i];
				UpdateItemStatus(search.nIndex, SS_ERROR, 
					CStrPrintf(_T("%s: %s"), _TLT("Failed to download lyrics"), ERROR2STR_LOCAL(nRet)).c_str());
			}
			m_listSearchItems.clear();
			return nRet;
		}

		ASSERT(retLyrics.listLyricsInfo.size() == m_listSearchItems.size());

		ListLyricsInfoLite::iterator it = retLyrics.listLyricsInfo.begin();
		for (int i = 0; i < m_listSearchItems.size() && it != retLyrics.listLyricsInfo.end();
			i++, ++it)
		{
			MLSearchItem &search = m_listSearchItems[i];
			MLLyricsInfoLite &retSearch = *it;

			if (retSearch.bufLyrContent.empty())
			{
				m_nNotFoundCount++;
				UpdateItemStatus(search.nIndex, SS_ERROR, _TLT("No lyrics were found."));
				continue;
			}

			OnLyricsDownloaded(search.nIndex, retSearch.strSaveName.c_str(), retSearch.bufLyrContent);
		}

		m_listSearchItems.clear();
		return nRet;
	}

	virtual int OnLocalLyricsFound(int nMediaIndex, LPCTSTR szMediaFile, LPCTSTR szLyrName) = 0;

	virtual void OnLyricsDownloaded(int nMediaIndex, LPCTSTR szLyrSaveName, string &bufLyr) = 0;

protected:
	MLListSearchItems		m_listSearchItems;
	CMLClientSession		m_session;
	int						m_nSavedEmbeddedLyrics, m_nSavedFileLyrics, m_nHavedLyrics;
	int						m_nFailedCount, m_nNotFoundCount;
	bool					m_bOnlyUseLocalLyrics;

};

class CSaveLyricsInITunes : public CLyricsDownloadWork
{
public:
	CSaveLyricsInITunes(IMediaSource *pMediaLib, IUpdateNotify *pUpdateNotify, WorkStatus &status) : CLyricsDownloadWork(pMediaLib, pUpdateNotify, status)
	{
	}

	virtual void OnProcessItem(int nIndex)
	{
		tstring	strLyrics;
		if (m_pMediaLib->GetEmbeddedLyrics(nIndex, strLyrics))
		{
			m_nHavedLyrics++;
			UpdateItemStatus(nIndex, SS_OK, _TLT("Already has lyrics"));
			return;
		}

		CLyricsDownloadWork::OnProcessItem(nIndex);
	}

protected:
	void SetEmbeddedLyrics(CMLData &lyrData, int nIndex)
	{
		tstring strLyrics;

		lyrData.ToString(strLyrics, FT_LYRICS_TXT, true);
		int nRet = m_pMediaLib->SetEmbeddedLyrics(nIndex, strLyrics);
		if (nRet == ERR_OK)
		{
			UpdateItemStatus(nIndex, SS_OK, _TLT("Lyrics were saved successfully"));
			m_nSavedEmbeddedLyrics++;
		}
		else
		{
			UpdateItemStatus(nIndex, SS_ERROR, 
				CStrPrintf(_T("%s: %s"), _TLT("Failed to save embedded lyrics"), ERROR2STR_LOCAL(nRet)).c_str());
			m_nFailedCount++;
		}
	}

	virtual int OnLocalLyricsFound(int nMediaIndex, LPCTSTR szMediaFile, LPCTSTR szLyrName)
	{
		CMLData lyrData;
		int nRet = lyrData.OpenLyrics(szMediaFile, 0, szLyrName, false, ED_SYSDEF);
		if (nRet != ERR_OK)
			UpdateItemStatus(nMediaIndex, SS_ERROR, ERROR2STR_LOCAL(nRet));
		else
			SetEmbeddedLyrics(lyrData, nMediaIndex);

		return nRet;
	}

	virtual void OnLyricsDownloaded(int nMediaIndex, LPCTSTR szLyrSaveName, string &bufLyr)
	{
		// Open lyrics
		CMLData lyrData;
		int nRet = lyrData.OpenLyrics(_T(""), 0, (LPBYTE)bufLyr.c_str(), (int)bufLyr.size());
		if (nRet != ERR_OK)
		{
			UpdateItemStatus(nMediaIndex, SS_ERROR, ERROR2STR_LOCAL(nRet));
			return;
		}

		// Convert to text lyrics.
		SetEmbeddedLyrics(lyrData, nMediaIndex);
	}

};

class CRemoveEmbeddedLyrics : public ILyricsToolWork
{
public:
	CRemoveEmbeddedLyrics(IMediaSource *pMediaLib, IUpdateNotify *pUpdateNotify, WorkStatus &status) : ILyricsToolWork(pMediaLib, pUpdateNotify, status)
	{
		m_nCountEmbeddedRemoved = m_nCountFailed = 0;
		m_nCountFileRemoved = 0;
	}

	virtual void OnProcessItem(int nIndex)
	{
		int nRet = m_pMediaLib->RemoveEmbeddedLyrics(nIndex);
		if (nRet == ERR_OK)
		{
			UpdateItemStatus(nIndex, SS_OK, _TLT("Embedded Lyrics were removed successfully"));
			m_nCountEmbeddedRemoved++;
		}
		else
		{
			if (nRet == ERR_NOT_FOUND)
				UpdateItemStatus(nIndex, SS_INFO, _TLT("No embedded lyrics"));
			else
				UpdateItemStatus(nIndex, SS_ERROR, ERROR2STR_LOCAL(nRet));
		}
	}

	virtual void OnEnd(tstring &strMsg)
	{
		if (m_nLastError != ERR_OK && !m_status.IsCancel())
		{
			strMsg = _TLT("Error:");
			strMsg += _T(" ");
			strMsg += ERROR2STR_LOCAL(m_nLastError);
			strMsg += _T("\n\n");
		}

		strMsg += CStrPrintf(_TLT("%d of %d embedded lyrics are removed."), m_nCountEmbeddedRemoved, m_pMediaLib->GetRowCount()).c_str();

		if (m_nCountFailed > 0)
		{
			strMsg += _T("\n");
			strMsg += CStrPrintf(_TLT("%d embedded lyrics are failed to remove."), m_nCountFailed).c_str();
		}
	}

protected:
	int						m_nCountEmbeddedRemoved, m_nCountFileRemoved, m_nCountFailed;

};

class CAdvRemoveLyrics : public CRemoveEmbeddedLyrics
{
public:
	CAdvRemoveLyrics(IMediaSource *pMediaLib, IUpdateNotify *pUpdateNotify, WorkStatus &status) : CRemoveEmbeddedLyrics(pMediaLib, pUpdateNotify, status)
	{
	}

	void SetInfo(bool bRemoveAssociateLyrFile, const VecString &vEmbeddedLyrNames)
	{
		m_bRemoveAssociateLyrFile = bRemoveAssociateLyrFile;
		m_vEmbeddedLyrNames = vEmbeddedLyrNames;
	}

	virtual void OnProcessItem(int nIndex)
	{
		tstring strMediaFile = m_pMediaLib->GetLocation(nIndex);

		if (m_vEmbeddedLyrNames.size() > 0)
		{
			int succeededCount = 0;
			int nRet = CTagLib::RemoveEmbeddedLyrics(strMediaFile.c_str(), m_vEmbeddedLyrNames, &succeededCount);
			m_nCountEmbeddedRemoved += succeededCount;
			if (succeededCount > 0)
				UpdateItemStatus(nIndex, SS_OK, _TLT("Embedded Lyrics were removed successfully"));
			else if (nRet == ERR_OK)
				UpdateItemStatus(nIndex, SS_INFO, _TLT("No embedded lyrics"));
			else
				UpdateItemStatus(nIndex, SS_ERROR, ERROR2STR_LOCAL(nRet));
		}

		if (m_bRemoveAssociateLyrFile)
		{
			TCHAR	szLyricsFile[MAX_PATH];
			if (g_LyricSearch.GetAssociateLyrics(strMediaFile.c_str(), szLyricsFile, CountOf(szLyricsFile)))
			{
				if (LyrSrcTypeFromName(szLyricsFile) == LST_FILE)
				{
					m_nCountFileRemoved++;
					DeleteFile(szLyricsFile);
					UpdateItemStatus(nIndex, SS_OK, _TLT("Lyrics file was removed successfully"));
				}
			}

			tstring strLyricsFile = strMediaFile;

			FileSetExt(strLyricsFile, _T(".lrc"));
			if (IsFileExist(strLyricsFile.c_str()))
				DeleteFile(strLyricsFile.c_str());

			FileSetExt(strLyricsFile, _T(".txt"));
			if (IsFileExist(strLyricsFile.c_str()))
				DeleteFile(strLyricsFile.c_str());
		}

	}

	virtual void OnEnd(tstring &strMsg)
	{
		CRemoveEmbeddedLyrics::OnEnd(strMsg);

		strMsg += _T("\n");
		strMsg += CStrPrintf(_TLT("%d of %d lyrics files are removed."), m_nCountFileRemoved, m_pMediaLib->GetRowCount()).c_str();
	}

	bool			m_bRemoveAssociateLyrFile;
	VecString		m_vEmbeddedLyrNames;

};

class CAdvDownloadLyrWork : public CLyricsDownloadWork
{
public:
	CAdvDownloadLyrWork(IMediaSource *pMediaLib, IUpdateNotify *pUpdateNotify, WorkStatus &status) : CLyricsDownloadWork(pMediaLib, pUpdateNotify, status)
	{
		// m_downSaveName = m_bKeepFileName ? DOWN_SAVE_NAME_KEEP : DOWN_SAVE_NAME_AS_SONG_NAME;
	}

	void SetInfo(DOWN_SAVE_DIR downSaveDir, DOWN_SAVE_NAME downSaveName, bool bOnlyUseLocalLyrics, const VecString &vSaveEmbeddedLyrNames)
	{
		m_bOnlyUseLocalLyrics = bOnlyUseLocalLyrics;
		m_downSaveDir = downSaveDir;
		m_downSaveName = downSaveName;
		m_vSaveEmbeddedLyrNames = vSaveEmbeddedLyrNames;

		m_nSavedEmbeddedLyrSrcType = 0;
		for (size_t i = 0; i < vSaveEmbeddedLyrNames.size(); i++)
		{
			m_nSavedEmbeddedLyrSrcType |= LyrSrcTypeFromName(m_vSaveEmbeddedLyrNames[i].c_str());
		}
	}

protected:
	void SaveLyrics(LPCTSTR szMediaSrc, LPCTSTR szLyrFileName, LPCSTR lyrBuf, size_t nLenLyr, int nIndex)
	{
		bool bFailed = false;
		int nRet;

		// Save embedded lyrics
		if (m_vSaveEmbeddedLyrNames.size() > 0 && CTagLib::IsEmbeddedLyricsSupported(szMediaSrc))
		{
			// If embedded lyrics exists, do NOT overwrite it.
			VecString vSaveEmbeddedLyrNames;
			uint32 lstExisted = 0;
			if (SearchEmbeddedLyrics(szMediaSrc, lstExisted) == ERR_OK)
			{
				for (size_t i = 0; i < m_vSaveEmbeddedLyrNames.size(); i++)
				{
					if (!IsFlagSet(lstExisted, LyrSrcTypeFromName(m_vSaveEmbeddedLyrNames[i].c_str())))
					{
						vSaveEmbeddedLyrNames.push_back(m_vSaveEmbeddedLyrNames[i]);
						m_nHavedLyrics++;
						UpdateItemStatus(nIndex, SS_OK, _TLT("Already has lyrics"));
					}
				}
			}

			CCharBuffer bufLyrics;
			bufLyrics.append(lyrBuf, nLenLyr);
			int succeededCount = 0;
			nRet = g_autoProcessEmbeddedLyrics.SaveEmbeddedLyrics(szMediaSrc, NULL, &bufLyrics, vSaveEmbeddedLyrNames, false, &succeededCount);
			if (nRet == ERR_OK)
			{
				if (succeededCount > 0)
				{
					UpdateItemStatus(nIndex, SS_OK, _TLT("Lyrics were saved successfully"));
					m_nSavedEmbeddedLyrics++;
				}
			}
			else
			{
				bFailed = true;
				UpdateItemStatus(nIndex, SS_ERROR, 
					CStrPrintf(_T("%s: %s"), _TLT("Failed to save embedded lyrics"), ERROR2STR_LOCAL(nRet)).c_str());
			}
		}

		if (m_downSaveDir == DOWN_SAVE_NO_FILE)
			return;

		// Save as lyrics file
		tstring strLyrFile = g_LyricsDownloader.GetSaveLyricsFile(szMediaSrc,
			szLyrFileName, m_downSaveDir, m_downSaveName);

		if (!IsFileExist(strLyrFile.c_str()))
		{
			nRet = g_LyricsDownloader.SaveDownloadedLyrAsFile(strLyrFile, lyrBuf, nLenLyr);
			if (nRet == ERR_OK)
			{
				if (!bFailed)
					UpdateItemStatus(nIndex, SS_OK, _TLT("Lyrics were saved successfully"));
				m_nSavedFileLyrics++;
			}
			else
			{
				bFailed = true;
				UpdateItemStatus(nIndex, SS_ERROR, 
					CStrPrintf(_TLT("Failed to save file: %s."), strLyrFile.c_str()).c_str());
			}
		}
		else
		{
			m_nHavedLyrics++;
			if (!bFailed)
				UpdateItemStatus(nIndex, SS_OK, _TLT("Already has lyrics"));
		}
	}

	virtual int OnLocalLyricsFound(int nMediaIndex, LPCTSTR szMediaFile, LPCTSTR szLyrName)
	{
		CMLData lyrData;
		int nRet = lyrData.OpenLyrics(szMediaFile, 0, szLyrName, false, ED_SYSDEF);
		if (nRet != ERR_OK)
		{
			UpdateItemStatus(nMediaIndex, SS_ERROR, ERROR2STR_LOCAL(nRet));
			return nRet;
		}

		if (lyrData.GetLyrContentType() == LCT_TXT)
		{
			// For text only lyrics, if we want to save sync lyrics, need to try to download it again.
			if ((m_nSavedEmbeddedLyrSrcType & (LST_ID3V2_SYLT | LST_LYRICS3V2)) != 0
				&& !g_LyricSearch.IsAssociatedLyrics(szMediaFile))
				return ERR_NOT_FOUND;
		}

		tstring str;
		CCharBuffer		bufLyrFile;
		lyrData.ToString(str, FT_LYRICS_LRC, true);
		TStringToProperFileData(str.c_str(), bufLyrFile);

		SaveLyrics(szMediaFile, lyrData.GetSuggestedLyricsFileName().c_str(), 
			bufLyrFile.c_str(), bufLyrFile.size(), nMediaIndex);

		return ERR_OK;
	}

	virtual void OnLyricsDownloaded(int nMediaIndex, LPCTSTR szLyrSaveName, string &bufLyr)
	{
		tstring strMediaFile = m_pMediaLib->GetLocation(nMediaIndex);
		if (strMediaFile.empty())
		{
			ASSERT(0);
			return;
		}

		SaveLyrics(strMediaFile.c_str(), szLyrSaveName, bufLyr.c_str(), bufLyr.size(), nMediaIndex);
	}

	DOWN_SAVE_DIR	m_downSaveDir;
	DOWN_SAVE_NAME	m_downSaveName;
	VecString		m_vSaveEmbeddedLyrNames;
	uint32			m_nSavedEmbeddedLyrSrcType;

};

}
