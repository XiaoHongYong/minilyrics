
#import "../../MacPluginiTunes/ETEyeTunes.h"
#import "../../MacPluginiTunes/ETTrack.h"
#import "../../MacPluginiTunes/ETPlaylist.h"


#import "StdAfx.h"
#import "MediaSource.h"
#import "MPlayerAppBase.h"
#import "../../TagLib/TagLib.h"

bool IMediaSource::GetBestMatchLyrics(int nIndex, tstring &lyrSrcPath)
{
	uint32	nMediaLength;
	tstring artist, album, title, location;
	if (!GetMediaInfo(nIndex, artist, album, title, location, nMediaLength))
		return false;

	return g_LyricSearch.GetBestMatchLyrics(location.c_str(), artist.c_str(), title.c_str(), lyrSrcPath);
}


//////////////////////////////////////////////////////////////////////////

class CiTunesMediaSource : public IMediaSource
{
public:
	CiTunesMediaSource();
	~CiTunesMediaSource();
	
	bool UpdateLibrary(tstring &err);
	
	virtual int GetItemCount();
	virtual bool GetLocation(int nIndex, tstring &location);
	virtual bool GetMediaInfo(int nIndex, tstring &artist, tstring &album, tstring &title, tstring &location, uint32 &nMediaLength);
	
	virtual bool GetEmbeddedLyrics(int nIndex, tstring &lyrics);
	virtual int SetEmbeddedLyrics(int nIndex, tstring &lyrics);
	
	virtual int RemoveEmbeddedLyrics(int nIndex);
	
protected:
	struct Item
	{
		tstring		location, artist, album, title;
		uint32		duration;
		ETTrack		*trackFile;
	};
	typedef vector<Item>	VecTracks;
	
	virtual void RemoveItem(int nIndex);
	
protected:
	virtual bool UpdateMediaInfo(Item &item);
	
protected:
	VecTracks				m_vTracks;
	EyeTunes				*m_iTunes;

};


IMediaSource *GetiTunesMediaSource()
{
	return new CiTunesMediaSource();
}

CiTunesMediaSource::CiTunesMediaSource() : IMediaSource(MST_ITUNES)
{
	m_iTunes = [EyeTunes sharedInstance];
}

CiTunesMediaSource::~CiTunesMediaSource()
{

}

bool CiTunesMediaSource::UpdateLibrary(tstring &err)
{
	if (m_iTunes == nil)
		return false;
	
	ETPlaylist *playlist = [m_iTunes libraryPlaylist];

	NSArray *tracks = [playlist tracks];
	for (int i = 0; i < [tracks count]; i++)
	{
		ETTrack *track = [tracks objectAtIndex:i];
		if (track == nil)
			continue;

		NSString *kind = [track kind];
//		if (kind != ITTrackKindFile)
//			continue;

		NSString *location = [track location];
		if (location == nil)
			continue;

		Item item;
		item.trackFile = track;
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
	ETTrack *track = item.trackFile;

	item.location = [[track location] UTF8String];
	item.album = [[track album] UTF8String];
	item.artist = [[track artist] UTF8String];
	item.title = [[track name] UTF8String];
	
	item.duration = [track duration] * 1000;

	return true;
}

bool CiTunesMediaSource::GetEmbeddedLyrics(int nIndex, tstring &lyrics)
{
	if (nIndex < 0 || nIndex >= (int)m_vTracks.size())
	{
		ASSERT(0);
		return false;
	}

	ETTrack *trackFile = m_vTracks[nIndex].trackFile;

	NSString *nsLyrics = [trackFile lyrics];
	if (nsLyrics == nil || [nsLyrics length] == 0)
		return false;

	lyrics = [nsLyrics UTF8String];

	return true;
}

int CiTunesMediaSource::SetEmbeddedLyrics(int nIndex, tstring &lyrics)
{
	if (nIndex < 0 || nIndex >= (int)m_vTracks.size())
	{
		ASSERT(0);
		return ERR_FALSE;
	}
	
	ETTrack *trackFile = m_vTracks[nIndex].trackFile;

	[trackFile setLyrics: [NSString stringWithUTF8String:lyrics.c_str()]];

	return ERR_OK;
}

int CiTunesMediaSource::RemoveEmbeddedLyrics(int nIndex)
{
	if (nIndex < 0 || nIndex >= (int)m_vTracks.size())
	{
		ASSERT(0);
		return ERR_FALSE;
	}
	
	tstring lyrics;
	return SetEmbeddedLyrics(nIndex, lyrics);
}

void CiTunesMediaSource::RemoveItem(int nIndex)
{
	if (nIndex < 0 || nIndex >= (int)m_vTracks.size())
	{
		ASSERT(0);
		return;
	}

	[m_vTracks[nIndex].trackFile release];
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

bool CFileMediaSource::GetEmbeddedLyrics(int nIndex, tstring &lyrics)
{
	lyrics.clear();

	if (nIndex < 0 || nIndex >= (int)m_vFiles.size())
		return false;

	Item &item = m_vFiles[nIndex];

	// Get ID3v2 USLT
	CID3v2IF		id3v2(ED_SYSDEF);
	int				nRet;

	if (!IsFileExist(item.file.c_str()))
		return false;

	nRet = id3v2.Open(item.file.c_str(), false, false);
	if (nRet == ERR_OK)
	{
		ID3v2UnsynchLyrics unsyncLyrics;
		nRet = id3v2.GetUnsyncLyrics(unsyncLyrics);
		if (nRet == ERR_OK)
			lyrics = unsyncLyrics.m_strLyrics;
		id3v2.Close();
	}

	return lyrics.size() > 0;
}

// Save lyrics as id3v2 unsync lyrics.
int CFileMediaSource::SetEmbeddedLyrics(int nIndex, tstring &lyrics)
{
	if (nIndex < 0 || nIndex >= (int)m_vFiles.size())
		return ERR_FALSE;
	Item &item = m_vFiles[nIndex];

	CID3v2IF		id3v2(ED_SYSDEF);

	int nRet = id3v2.Open(item.file.c_str(), true, true);
	if (nRet != ERR_OK)
		return nRet;

	nRet = id3v2.SetUnsynchLyrics(NULL, _T(""), lyrics.c_str());
	if (nRet != ERR_OK)
		return nRet;

	return id3v2.Save();
}

int CFileMediaSource::RemoveEmbeddedLyrics(int nIndex)
{
	if (nIndex < 0 || nIndex >= (int)m_vFiles.size())
		return ERR_FALSE;
	Item &item = m_vFiles[nIndex];

	int nRemovedCount = 0;
	VecString vLyrNames;
	vLyrNames.push_back(SZ_SONG_ID3V2_SYLT);
	vLyrNames.push_back(SZ_SONG_ID3V2_USLT);

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
