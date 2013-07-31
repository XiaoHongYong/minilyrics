#pragma once

enum MediaSourceType
{
	MST_ITUNES,
	MST_FILES,
};

class IMediaSource : public IListViewDataSource
{
public:
	IMediaSource(MediaSourceType type) : m_type(type) { }
	virtual ~IMediaSource();
	
	virtual bool UpdateLibrary(tstring &err) { return true; }

	MediaSourceType GetType() const { return m_type; }

	virtual tstring GetLocation(int nIndex) = 0;
	virtual bool GetMediaInfo(int nIndex, tstring &artist, tstring &album, tstring &title, tstring &location, uint32 &nMediaLength) = 0;

	virtual bool GetEmbeddedLyrics(int nIndex, tstring &lyrics) = 0;
	virtual int SetEmbeddedLyrics(int nIndex, tstring &lyrics) = 0;

	virtual int RemoveEmbeddedLyrics(int nIndex) = 0;

	virtual bool GetBestMatchLyrics(int nIndex, tstring &lyrSrcPath);

	virtual void RemoveItem(int nIndex);

	virtual void SetResult(int nIndex, LPCTSTR szResult);


	//
	// Common Methods for IListViewDataSource
	//

	virtual int GetRowCount() const;
	
	virtual bool IsRowSelected(int row) const;
	virtual void SetRowSelectionState(int nIndex, bool bSelected);
	
	virtual int GetItemImageIndex(int row);
	virtual bool SetItemImageIndex(int nItem, int nImageIndex, bool bRedraw = true);
	
	virtual LPCTSTR GetCellText(int row, int col);
	virtual CRawImage *GetCellImage(int row, int col);

protected:
	class Item
	{
	public:
		int imageIndex;
		bool isSelected;
		tstring		result, lyrics;
		tstring		title;
		
		Item() { imageIndex = 0; isSelected = false; }
		virtual ~Item() { }

	};

	typedef vector<Item *>	VecTracks;

	enum
	{
		COL_MEDIA_FILE,
		COL_RESULT,
		COL_LYRICS,
	};

protected:
	MediaSourceType		m_type;
	VecTracks			m_vTracks;

};

class CFileMediaSource : public IMediaSource
{
public:
	CFileMediaSource() : IMediaSource(MST_FILES) { }
	
	void AddFolder(LPCTSTR szFolder, bool bIncludeSubFolder);
	void AddFiles(const V_Str &vFiles);
	void AddFile(LPCTSTR szFile);
	
	virtual tstring GetLocation(int nIndex);
	virtual bool GetMediaInfo(int nIndex, tstring &artist, tstring &album, tstring &title, tstring &location, uint32 &nMediaLength);
	
	virtual bool GetEmbeddedLyrics(int nIndex, tstring &lyrics);
	virtual int SetEmbeddedLyrics(int nIndex, tstring &lyrics);
	
	virtual int RemoveEmbeddedLyrics(int nIndex);

protected:
	bool IsFileExist(LPCTSTR szFile);

	virtual LPCTSTR GetCellText(int row, int col);

protected:
	class ItemF : public Item
	{
	public:
		ItemF(tstring &file) { this->file = file; }
		tstring		file;
	};

};

IMediaSource *NewMediaSource(MediaSourceType type);
