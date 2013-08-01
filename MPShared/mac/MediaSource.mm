
#import "../../MacPluginiTunes/ETEyeTunes.h"
#import "../../MacPluginiTunes/ETTrack.h"
#import "../../MacPluginiTunes/ETPlaylist.h"


#import "StdAfx.h"
#import "MediaSource.h"
#import "MPlayerAppBase.h"
#import "../../TagLib/TagLib.h"

IMediaSource::~IMediaSource()
{
	for (size_t i = 0; i < m_vTracks.size(); i++)
	{
		delete m_vTracks[i];
	}
	m_vTracks.clear();
}

bool IMediaSource::GetBestMatchLyrics(int nIndex, tstring &lyrSrcPath)
{
	uint32	nMediaLength;
	tstring artist, album, title, location;
	if (!GetMediaInfo(nIndex, artist, album, title, location, nMediaLength))
		return false;

	return g_LyricSearch.GetBestMatchLyrics(location.c_str(), artist.c_str(), title.c_str(), lyrSrcPath);
}

void IMediaSource::RemoveItem(int nIndex)
{
	if (nIndex >= 0 && nIndex < (int)m_vTracks.size())
	{
		delete m_vTracks[nIndex];
		m_vTracks.erase(m_vTracks.begin() + nIndex);
	}
}

void IMediaSource::SetResult(int nIndex, LPCTSTR szResult)
{
	if (nIndex >= 0 && nIndex < (int)m_vTracks.size())
	{
		m_vTracks[nIndex]->result = szResult;
	}
}

int IMediaSource::GetRowCount() const
{
	return (int)m_vTracks.size();
}

bool IMediaSource::IsRowSelected(int row) const
{
	return m_vTracks[row]->isSelected;
}

void IMediaSource::SetRowSelectionState(int nIndex, bool bSelected)
{
	m_vTracks[nIndex]->isSelected = bSelected;
}

int IMediaSource::GetItemImageIndex(int row)
{
	return m_vTracks[row]->imageIndex;
}

bool IMediaSource::SetItemImageIndex(int nItem, int nImageIndex, bool bRedraw)
{
	m_vTracks[nItem]->imageIndex = nImageIndex;
	if (bRedraw)
		m_pView->Invalidate();

	return true;
}

tstring FormatMediaTitle(LPCTSTR szArtist, LPCTSTR szTitle);

LPCTSTR GetLyricsDescription(LPCTSTR szLyrics)
{
	LRC_SOURCE_TYPE	lst = LyrSrcTypeFromName(szLyrics);
	if (lst == LST_FILE)
		return szLyrics;
	return LyrSrcTypeToDesc(lst);
}

LPCTSTR IMediaSource::GetCellText(int row, int col)
{
	Item *item = m_vTracks[row];

	if (col == COL_RESULT)
	{
		return item->result.c_str();
	}
	else if (col == COL_LYRICS)
	{
		if (item->lyrics.empty())
		{
			TCHAR		szLyrics[MAX_PATH];
			if (g_LyricSearch.GetAssociateLyrics(GetLocation(row).c_str(), szLyrics, CountOf(szLyrics)))
			{
				if (_tcscmp(szLyrics, NONE_LYRCS) == 0)
					item->lyrics = _TLT("No suitable lyrics for the song file");
				else
					item->lyrics = GetLyricsDescription(szLyrics);
			}
		}
		return item->lyrics.c_str();
	}
	else
		return "";
	
	//	item.location = [[track location] UTF8String];
	//	item.album = [[track album] UTF8String];
	//	item.artist = [[track artist] UTF8String];
	//	item.title = [[track name] UTF8String];
	//
	//	item.duration = [track duration] * 1000;
	//
	
}

CRawImage *IMediaSource::GetCellImage(int row, int col)
{
	return NULL;
}

//////////////////////////////////////////////////////////////////////////

class CiTunesMediaSource : public IMediaSource
{
public:
	CiTunesMediaSource();
	~CiTunesMediaSource();
	
	bool UpdateLibrary(tstring &err);
	
	virtual tstring GetLocation(int nIndex);
	virtual bool GetMediaInfo(int nIndex, tstring &artist, tstring &album, tstring &title, tstring &location, uint32 &nMediaLength);
	
	virtual bool GetEmbeddedLyrics(int nIndex, tstring &lyrics);
	virtual int SetEmbeddedLyrics(int nIndex, tstring &lyrics);
	
	virtual int RemoveEmbeddedLyrics(int nIndex);
	
	virtual LPCTSTR GetCellText(int row, int col);

protected:
	class ItemT : public Item
	{
	public:
		ETTrack		*track;
		
		ItemT() { track = NULL; }
		virtual ~ItemT() {
			[track release];
		}
	};
	
	ETTrack *GetTrack(int index);

protected:
	EyeTunes				*m_iTunes;
	ETPlaylist				*m_playlist;

};

CiTunesMediaSource::CiTunesMediaSource() : IMediaSource(MST_ITUNES)
{
	m_iTunes = [EyeTunes sharedInstance];
}

CiTunesMediaSource::~CiTunesMediaSource()
{
}

ETTrack *CiTunesMediaSource::GetTrack(int index)
{
	if (index < 0 || index >= (int)m_vTracks.size())
	{
		ASSERT(0);
		return NULL;
	}
	
	ItemT *item = (ItemT*)m_vTracks[index];
	if (item->track == NULL)
		item->track = [m_playlist trackAtIndex:index];

	return item->track;
}

bool CiTunesMediaSource::UpdateLibrary(tstring &err)
{
	if (m_iTunes == nil)
		return false;
	
	m_playlist = [m_iTunes libraryPlaylist];
	int count = [m_playlist trackCount];

	for (int i = 0; i < count; i++)
	{
		ItemT *item = new ItemT;
		m_vTracks.push_back(item);
	}

	return true;
}

tstring CiTunesMediaSource::GetLocation(int nIndex)
{
	ETTrack *track = GetTrack(nIndex);
	if (track == NULL)
		return _T("");

	return [[track location] UTF8String];
}

tstring FixFileUri(LPCTSTR szUri);

bool CiTunesMediaSource::GetMediaInfo(int nIndex, tstring &artist, tstring &album, tstring &title, tstring &location, uint32 &nMediaLength)
{
	ETTrack *track = GetTrack(nIndex);
	if (track == NULL)
		return false;

	artist = [[track artist] UTF8String];
	album = [[track album] UTF8String];
	title = [[track name] UTF8String];
	location = [[track location] UTF8String];
	nMediaLength = [track duration] * 1000;
	
	location = FixFileUri(location.c_str());

	if (artist.empty() && title.find('-') != -1)
		AnalysisLyricsFileNameEx(artist, title, title.c_str());

	return true;
}

bool CiTunesMediaSource::GetEmbeddedLyrics(int nIndex, tstring &lyrics)
{
	ETTrack *track = GetTrack(nIndex);
	if (track == NULL)
		return false;

	NSString *nsLyrics = [track lyrics];
	if (nsLyrics == nil || [nsLyrics length] == 0)
		return false;

	lyrics = [nsLyrics UTF8String];

	return true;
}

int CiTunesMediaSource::SetEmbeddedLyrics(int nIndex, tstring &lyrics)
{
	ETTrack *track = GetTrack(nIndex);
	if (track == NULL)
		return false;

	[track setLyrics: [NSString stringWithUTF8String:lyrics.c_str()]];

	return ERR_OK;
}

int CiTunesMediaSource::RemoveEmbeddedLyrics(int nIndex)
{
	tstring lyrics;
	return SetEmbeddedLyrics(nIndex, lyrics);
}

LPCTSTR CiTunesMediaSource::GetCellText(int row, int col)
{
	if (col != COL_MEDIA_FILE)
		return IMediaSource::GetCellText(row, col);

	Item *item = m_vTracks[row];

	if (item->title.empty())
	{
		ETTrack *track = GetTrack(row);
		if (track == NULL)
			return _T("");

		item->title = FormatMediaTitle([[track artist] UTF8String], [[track name] UTF8String]);
		if (item->title.empty())
		{
			item->title = FileGetTitle(GetLocation(row).c_str());
		}
	}
	
	return item->title.c_str();
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
					m_vTracks.push_back(new ItemF(strFile));
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
			m_vTracks.push_back(new ItemF(str));
		}
	}
}

void CFileMediaSource::AddFile(LPCTSTR szFile)
{
	if (!IsFileExist(szFile))
	{
		tstring str = szFile;
		m_vTracks.push_back(new ItemF(str));
	}
}

tstring CFileMediaSource::GetLocation(int nIndex)
{
	if (nIndex < 0 || nIndex >= (int)m_vTracks.size())
		return "";
	
	ItemF *item = (ItemF*)m_vTracks[nIndex];

	return item->file;
}

bool CFileMediaSource::GetMediaInfo(int nIndex, tstring &artist, tstring &album, tstring &title, tstring &location, uint32 &nMediaLength)
{
	if (nIndex < 0 || nIndex >= (int)m_vTracks.size())
		return false;

	ItemF *item = (ItemF*)m_vTracks[nIndex];

	int nRet = CTagLib::GetTagFast(item->file.c_str(), &artist, &title, &album,
		NULL, NULL, NULL, &nMediaLength);
	if (nRet != ERR_OK || title.empty())
	{
		// Analysis file name
		AnalysisLyricsFileNameEx(artist, title, item->file.c_str());
	}
	nMediaLength /= 1000;
	location = item->file;

	return true;
}

bool CFileMediaSource::GetEmbeddedLyrics(int nIndex, tstring &lyrics)
{
	lyrics.clear();

	if (nIndex < 0 || nIndex >= (int)m_vTracks.size())
		return false;

	ItemF *item = (ItemF*)m_vTracks[nIndex];

	// Get ID3v2 USLT
	CID3v2IF		id3v2(ED_SYSDEF);
	int				nRet;

	if (!IsFileExist(item->file.c_str()))
		return false;

	nRet = id3v2.Open(item->file.c_str(), false, false);
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
	if (nIndex < 0 || nIndex >= (int)m_vTracks.size())
		return ERR_FALSE;

	ItemF *item = (ItemF*)m_vTracks[nIndex];
	CID3v2IF		id3v2(ED_SYSDEF);

	int nRet = id3v2.Open(item->file.c_str(), true, true);
	if (nRet != ERR_OK)
		return nRet;

	nRet = id3v2.SetUnsynchLyrics(NULL, _T(""), lyrics.c_str());
	if (nRet != ERR_OK)
		return nRet;

	return id3v2.Save();
}

int CFileMediaSource::RemoveEmbeddedLyrics(int nIndex)
{
	if (nIndex < 0 || nIndex >= (int)m_vTracks.size())
		return ERR_FALSE;
	ItemF *item = (ItemF*)m_vTracks[nIndex];

	int nRemovedCount = 0;
	VecString vLyrNames;
	vLyrNames.push_back(SZ_SONG_ID3V2_SYLT);
	vLyrNames.push_back(SZ_SONG_ID3V2_USLT);

	return CTagLib::RemoveEmbeddedLyrics(item->file.c_str(), vLyrNames, &nRemovedCount);
}

bool CFileMediaSource::IsFileExist(LPCTSTR szFile)
{
	for (int i = 0; i < (int)m_vTracks.size(); i++)
	{
		if (_tcscmp(szFile, ((ItemF *)m_vTracks[i])->file.c_str()) == 0)
			return true;
	}

	return false;
}

LPCTSTR CFileMediaSource::GetCellText(int row, int col)
{
	if (col != COL_MEDIA_FILE)
		return IMediaSource::GetCellText(row, col);

	Item *item = m_vTracks[row];
	tstring &file = ((ItemF*)m_vTracks[row])->file;

	if (item->title.empty())
	{
		item->title = FileGetTitle(file.c_str());
	}
	
	return item->title.c_str();
}

IMediaSource *NewMediaSource(MediaSourceType type)
{
	if (type == MST_ITUNES)
		return new CiTunesMediaSource();
	else
		return new CFileMediaSource();
}
