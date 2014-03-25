#include "StdAfx.h"
#include "MPlayerAppBase.h"
#include "MediaSource.h"
#include "../TagLib/TagLib.h"
#include "../pluginiTunes/iTunesCOMInterface_i.c"
#include "AutoProcessEmbeddedLyrics.h"

bool IMediaSource::GetBestMatchLyrics(int nIndex, tstring &lyrSrcPath)
{
	uint32	nMediaLength;
	tstring artist, album, title, location;
	if (!GetMediaInfo(nIndex, artist, album, title, location, nMediaLength))
		return false;

	return g_LyricSearch.GetBestMatchLyrics(location.c_str(), artist.c_str(), title.c_str(), lyrSrcPath);
}


//////////////////////////////////////////////////////////////////////////

CiTunesMediaSource::CiTunesMediaSource() : IMediaSource(MST_ITUNES)
{
}

CiTunesMediaSource::~CiTunesMediaSource()
{

}

bool CiTunesMediaSource::UpdateLibrary(tstring &err)
{
	if (m_iTunesApp == NULL)
	{
		HRESULT hr = ::CoCreateInstance(CLSID_iTunesApp, NULL, CLSCTX_LOCAL_SERVER, IID_IiTunes, (PVOID *)&m_iTunesApp);
		if (FAILED(hr) || m_iTunesApp == NULL)
		{
			ERRLOG1(_T("Failed to create iTunesApp Instance: %X"), hr);
			err = _TLT("Maybe you haven't installed iTunes.");
			return false;
		}
#ifdef _DEBUG
		m_threadId = GetCurrentThreadId();
#endif
	}

	CMPAutoPtr<IITLibraryPlaylist> mainLibrary;
	HRESULT hr = m_iTunesApp->get_LibraryPlaylist(&mainLibrary);
	if (FAILED(hr) || mainLibrary == NULL)
		return false;

	CMPAutoPtr<IITTrackCollection> tracks;
	hr = mainLibrary->get_Tracks(&tracks);
	if (FAILED(hr) || tracks == NULL)
		return false;

	long countTracks = 0;
	hr = tracks->get_Count(&countTracks);
	if (FAILED(hr))
		return false;

	for (int i = 0; i < countTracks; i++)
	{
		CMPAutoPtr<IITTrack> track;
		HRESULT hr = tracks->get_Item(i, &track);
		if (FAILED(hr) || track == NULL)
			continue;

		ITTrackKind kind;
		hr = track->get_Kind(&kind);
		if (FAILED(hr))
			continue;

		if (kind != ITTrackKindFile)
			continue;

		IITFileOrCDTrack *trackFile = NULL;
		hr = track->QueryInterface(IID_IITFileOrCDTrack, (LPVOID *)&trackFile);
		if (FAILED(hr) || trackFile == NULL)
			continue;

		Item item;
		item.trackFile = trackFile;
		UpdateMediaInfo(item);
		m_vTracks.push_back(item);
	}

	return true;
}

int CiTunesMediaSource::GetItemCount()
{
	return (int)m_vTracks.size();
}

bool CiTunesMediaSource::GetLocation(int nIndex, tstring &location)
{
	if (nIndex < 0 || nIndex >= (int)m_vTracks.size())
	{
		ASSERT(0);
		return false;
	}

	Item &item = m_vTracks[nIndex];
	location = item.location;

	return true;
}

bool CiTunesMediaSource::GetMediaInfo(int nIndex, tstring &artist, tstring &album, tstring &title, tstring &location, uint32 &nMediaLength)
{
	if (nIndex < 0 || nIndex >= (int)m_vTracks.size())
	{
		ASSERT(0);
		return false;
	}

	Item &item = m_vTracks[nIndex];
	artist = item.artist;
	album = item.album;
	title = item.title;
	location = item.location;
	nMediaLength = item.duration;

	return true;
}

bool CiTunesMediaSource::UpdateMediaInfo(Item &item)
{
	IITFileOrCDTrack *trackFile = item.trackFile;

	bstr_s bstrLocation, bstrArtist, bstrAlbum, bstrTitle;
	HRESULT hr = trackFile->get_Location(&bstrLocation);
	if (FAILED(hr) || bstrLocation == NULL)
		return false;

	hr = trackFile->get_Artist(&bstrArtist);
	if (SUCCEEDED(hr) && !IsEmptyString(bstrArtist.c_str()))
		item.artist = bstrArtist.c_str();

	hr = trackFile->get_Album(&bstrAlbum);
	if (SUCCEEDED(hr) && !IsEmptyString(bstrAlbum.c_str()))
		item.album = bstrAlbum.c_str();

	hr = trackFile->get_Name(&bstrTitle);
	if (SUCCEEDED(hr) && !IsEmptyString(bstrTitle.c_str()))
		item.title = bstrTitle.c_str();

	item.location = bstrLocation.c_str();

	long duration = 0;
	hr = trackFile->get_Duration(&duration);
	item.duration = duration;

	return true;
}

bool CiTunesMediaSource::GetEmbeddedLyrics(int nIndex, tstring &lyrics)
{
	if (nIndex < 0 || nIndex >= (int)m_vTracks.size())
	{
		ASSERT(0);
		return false;
	}

	ASSERT(m_threadId == GetCurrentThreadId());

	IITFileOrCDTrack *trackFile = m_vTracks[nIndex].trackFile;

	bstr_s bstrLyrics;
	HRESULT hr = trackFile->get_Lyrics(&bstrLyrics);
	if (FAILED(hr) || IsEmptyString(bstrLyrics.c_str()))
		return false;

	lyrics = bstrLyrics.c_str();

	return true;
}

int CiTunesMediaSource::SetEmbeddedLyrics(int nIndex, tstring &lyrics)
{
	if (nIndex < 0 || nIndex >= (int)m_vTracks.size())
	{
		ASSERT(0);
		return ERR_FALSE;
	}

	ASSERT(m_threadId == GetCurrentThreadId());

	IITFileOrCDTrack *trackFile = m_vTracks[nIndex].trackFile;

	bstr_s	bstrLyrics(lyrics.c_str());
	HRESULT hr = trackFile->put_Lyrics(bstrLyrics.c_str());
	if (FAILED(hr))
	{
		SetCustomErrorDesc(OSError(hr).Description());
		return ERR_CUSTOM_ERROR;
	}

	return ERR_OK;
}

int CiTunesMediaSource::RemoveEmbeddedLyrics(int nIndex)
{
	if (nIndex < 0 || nIndex >= (int)m_vTracks.size())
	{
		ASSERT(0);
		return ERR_FALSE;
	}

	ASSERT(m_threadId == GetCurrentThreadId());

	IITFileOrCDTrack *trackFile = m_vTracks[nIndex].trackFile;

	bstr_s bstrLocation;
	HRESULT hr = trackFile->get_Location(&bstrLocation);
	if (FAILED(hr))
		return false;

	bstr_s bstrLyrics;
	hr = trackFile->get_Lyrics(&bstrLyrics);
	if (FAILED(hr))
	{
		ERRLOG1(_T("Failed to get lyrics: %s"), OSError(hr).Description());
		return ERR_NOT_FOUND;
	}

	if (IsEmptyString(bstrLyrics.c_str()))
		return ERR_NOT_FOUND;

	bstr_s	bstrLyricsEmpty(L"");
	hr = trackFile->put_Lyrics(bstrLyricsEmpty.c_str());
	if (FAILED(hr))
	{
		SetCustomErrorDesc(OSError(hr).Description());
		return ERR_CUSTOM_ERROR;
	}

	return ERR_OK;
}

void CiTunesMediaSource::RemoveItem(int nIndex)
{
	if (nIndex < 0 || nIndex >= (int)m_vTracks.size())
	{
		ASSERT(0);
		return;
	}

	m_vTracks[nIndex].trackFile->Release();
	m_vTracks.erase(m_vTracks.begin() + nIndex);
}

//////////////////////////////////////////////////////////////////////////

void CFileMediaSource::AddFolder(LPCTSTR szFolder, bool bIncludeSubFolder)
{
	CMLFileFind		find;
	tstring			strDir;
	tstring			strFile;

	if (!find.OpenDir(szFolder))
		return;

	strDir = szFolder;
	DirStringAddSlash(strDir);

	while (find.FindNext())
	{
		if (find.IsCurDir() && bIncludeSubFolder)
		{
			strFile = strDir + find.GetCurName();
			AddFolder(strFile.c_str(), bIncludeSubFolder);
		}
		else
		{
			if (CTagLib::IsMediaTypeSupported(find.GetCurName()))
			{
				strFile = strDir + find.GetCurName();
				if (!IsFileExist(strFile.c_str()))
				{
					m_vFiles.push_back(Item(strFile));
				}
			}
		}
	}
}

void CFileMediaSource::AddFiles(const V_Str &vFiles)
{
	for (uint i = 0; i < vFiles.size(); i++)
	{
		if (!IsFileExist(vFiles[i].c_str()))
		{
			tstring	str = vFiles[i];
			m_vFiles.push_back(Item(str));
		}
	}
}

void CFileMediaSource::AddFile(LPCTSTR szFile)
{
	if (!IsFileExist(szFile))
	{
		tstring str = szFile;
		m_vFiles.push_back(Item(str));
	}
}

void CFileMediaSource::Close()
{
	m_vFiles.clear();
}

int CFileMediaSource::GetItemCount()
{
	return (int)m_vFiles.size();
}

bool CFileMediaSource::GetLocation(int nIndex, tstring &location)
{
	if (nIndex < 0 || nIndex >= (int)m_vFiles.size())
		return false;

	location = m_vFiles[nIndex].file;

	return true;
}

bool CFileMediaSource::GetMediaInfo(int nIndex, tstring &artist, tstring &album, tstring &title, tstring &location, uint32 &nMediaLength)
{
	if (nIndex < 0 || nIndex >= (int)m_vFiles.size())
		return false;

	Item &item = m_vFiles[nIndex];

	if (!item.bInfoAvailable)
	{
		int nRet = CTagLib::GetTagFast(item.file.c_str(), &item.artist, &item.title, &item.album,
			NULL, NULL, NULL, &item.nMediaLength);
		if (nRet != ERR_OK || item.title.empty())
		{
			// Analysis file name
			AnalysisLyricsFileNameEx(item.artist, item.title, item.file.c_str());
		}
		item.bInfoAvailable = true;
		item.nMediaLength /= 1000;
	}

	location = item.file;
	artist = item.artist;
	album = item.album;
	title = item.title;
	nMediaLength = item.nMediaLength;

	return true;
}

void TryToRecoverDmagedM4aFiles(LPCTSTR szFile)
{
	// Bug fixes for adding ID3v2 tag to m4a files. This bug was fixed after: 7.6.43.
	if (!FileIsExtSame(szFile, _T(".m4a")))
		return;

	CID3v2IF		id3v2(ED_SYSDEF);

	int nRet = id3v2.Open(szFile, true, false);
	if (nRet == ERR_OK)
	{
		nRet = id3v2.RemoveTagAndClose();
		LOG2(LOG_LVL_INFO, _T("Remove ID3v2 Tag from damaged M4A file: %s, Result: %d"), szFile, nRet);
	}
}

bool CFileMediaSource::GetEmbeddedLyrics(int nIndex, tstring &lyrics)
{
	lyrics.clear();

	if (nIndex < 0 || nIndex >= (int)m_vFiles.size())
		return false;

	Item &item = m_vFiles[nIndex];

	if (!IsFileExist(item.file.c_str()))
		return false;

	if (CTagLib::IsID3v2TagSupported(item.file.c_str()))
	{
		// ID3v2 lyrics
		CID3v2IF		id3v2(ED_SYSDEF);
		int				nRet;

		nRet = id3v2.Open(item.file.c_str(), false, false);
		if (nRet == ERR_OK)
		{
			ID3v2UnsynchLyrics unsyncLyrics;
			nRet = id3v2.GetUnsyncLyrics(unsyncLyrics);
			if (nRet == ERR_OK)
				lyrics = unsyncLyrics.m_strLyrics;
			id3v2.Close();
		}
	}
	else if (CTagLib::IsM4aTagSupported(item.file.c_str()))
	{
		CM4aTag	tag;
		int nRet = tag.Open(item.file.c_str(), false);
		if (nRet != ERR_OK)
			return false;

		nRet = tag.GetLyrics(lyrics);
	}

	return lyrics.size() > 0;
}

// Save lyrics as id3v2 unsync lyrics.
int CFileMediaSource::SetEmbeddedLyrics(int nIndex, tstring &lyrics)
{
	if (nIndex < 0 || nIndex >= (int)m_vFiles.size())
		return ERR_FALSE;

	Item &item = m_vFiles[nIndex];


	if (!CTagLib::IsEmbeddedLyricsSupported(item.file.c_str()))
		return ERR_NOT_SUPPORT;

	TryToRecoverDmagedM4aFiles(item.file.c_str());

	CCharBuffer bufLyrics;
	VecString vLyrNames;

	TStringToProperFileData(lyrics.c_str(), bufLyrics);
	if (CTagLib::IsID3v2TagSupported(item.file.c_str()))
		vLyrNames.push_back(SZ_SONG_ID3V2_USLT);
	else if (CTagLib::IsM4aTagSupported(item.file.c_str()))
		vLyrNames.push_back(SZ_SONG_M4A_LYRICS);
	else
		return ERR_NOT_SUPPORT;

	return g_autoProcessEmbeddedLyrics.SaveEmbeddedLyrics(item.file.c_str(), NULL, &bufLyrics, vLyrNames);
}

int CFileMediaSource::RemoveEmbeddedLyrics(int nIndex)
{
	if (nIndex < 0 || nIndex >= (int)m_vFiles.size())
		return ERR_FALSE;
	Item &item = m_vFiles[nIndex];

	int nRemovedCount = 0;
	VecString vLyrNames;
	// vLyrNames.push_back(SZ_SONG_ID3V2_SYLT);
	if (CTagLib::IsID3v2TagSupported(item.file.c_str()))
		vLyrNames.push_back(SZ_SONG_ID3V2_USLT);
	else if (CTagLib::IsM4aTagSupported(item.file.c_str()))
	{
		TryToRecoverDmagedM4aFiles(item.file.c_str());

		vLyrNames.push_back(SZ_SONG_M4A_LYRICS);
	}
	else
		return ERR_NOT_SUPPORT;

	return CTagLib::RemoveEmbeddedLyrics(item.file.c_str(), vLyrNames, &nRemovedCount);
}

void CFileMediaSource::RemoveItem(int nIndex)
{
	if (nIndex < 0 || nIndex >= (int)m_vFiles.size())
		return;

	m_vFiles.erase(m_vFiles.begin() + nIndex);
}

void CFileMediaSource::ExpandMediaFiles(V_Str &vFiles)
{
	for (int i = 0; i < (int)vFiles.size(); ++i)
	{
		if (CTagLib::IsMediaTypeSupported(vFiles[i].c_str())
			&& !IsFileExist(vFiles[i].c_str()))
		{
			m_vFiles.push_back(Item(vFiles[i]));
		}
	}
}

bool CFileMediaSource::IsFileExist(LPCTSTR szFile)
{
	for (int i = 0; i < (int)m_vFiles.size(); i++)
	{
		if (_tcscmp(szFile, m_vFiles[i].file.c_str()) == 0)
			return true;
	}

	return false;
}
