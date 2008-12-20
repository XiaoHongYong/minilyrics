#pragma once

class CCurMediaAlbumArt
{
public:
	CCurMediaAlbumArt();
	virtual ~CCurMediaAlbumArt();

	bool IsLoaded() const { return m_bLoaded; }
	int Load();

	void Close();

	PRAW_IMAGE_DATA LoadAlbumArtByIndex(int nIndex);

	int GetPicCount();

public:
	// V_PICS				m_vPics;
	ID3v2Pictures			m_id3v2Pic;

	// album art of song file directory.
	V_Str					m_vAlbumPicFile;

	bool					m_bLoaded;

};
