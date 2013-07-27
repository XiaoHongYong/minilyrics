#pragma once

enum MediaSourceType
{
	MST_ITUNES,
	MST_FILES,
};

class IMediaSource
{
public:
	IMediaSource(MediaSourceType type) : m_type(type) { }
	virtual ~IMediaSource() { }
	
	virtual bool UpdateLibrary(tstring &err) { return true; }

	MediaSourceType GetType() const { return m_type; }

	virtual int GetItemCount() = 0;
	virtual bool GetLocation(int nIndex, tstring &location) = 0;
	virtual bool GetMediaInfo(int nIndex, tstring &artist, tstring &album, tstring &title, tstring &location, uint32 &nMediaLength) = 0;

	virtual bool GetEmbeddedLyrics(int nIndex, tstring &lyrics) = 0;
	virtual int SetEmbeddedLyrics(int nIndex, tstring &lyrics) = 0;

	virtual int RemoveEmbeddedLyrics(int nIndex) = 0;

	virtual bool GetBestMatchLyrics(int nIndex, tstring &lyrSrcPath);

	virtual void RemoveItem(int nIndex) = 0;

protected:
	MediaSourceType		m_type;

};

IMediaSource *GetiTunesMediaSource();

class CFileMediaSource : public IMediaSource
{
public:
	CFileMediaSource() : IMediaSource(MST_FILES) { }

	void AddFolder(LPCTSTR szFolder, bool bIncludeSubFolder);
	void AddFiles(const V_Str &vFiles);
	void AddFile(LPCTSTR szFile);

	virtual void Close();

	virtual int GetItemCount();
	virtual bool GetLocation(int nIndex, tstring &location);
	virtual bool GetMediaInfo(int nIndex, tstring &artist, tstring &album, tstring &title, tstring &location, uint32 &nMediaLength);

	virtual bool GetEmbeddedLyrics(int nIndex, tstring &lyrics);
	virtual int SetEmbeddedLyrics(int nIndex, tstring &lyrics);

	virtual int RemoveEmbeddedLyrics(int nIndex);

	virtual void RemoveItem(int nIndex);

protected:
	void ExpandMediaFiles(V_Str &vFiles);
	bool IsFileExist(LPCTSTR szFile);

protected:
	struct Item
	{
		tstring		file, artist, album, title;
		bool		bInfoAvailable;
		uint32		nMediaLength;

		Item(tstring &file) {
			this->bInfoAvailable = false;
			this->file = file;
			this->nMediaLength = 0;
		}
	};
	typedef vector<Item>		VecItems;

	VecItems				m_vFiles;

};
