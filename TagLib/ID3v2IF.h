#pragma once

#include "../ID3Lib/ID3v2.h"

class CID3v2IF : public CID3v2
{
public:
	CID3v2IF(CHAR_ENCODING encoding);
	~CID3v2IF();

	int ListLyrics(VecString &vLyrNames);

	int GetSyncLyrics(ID3v2SynchLyrics &lyrics, LPCTSTR szName = NULL);
	int GetUnsyncLyrics(ID3v2UnsynchLyrics &lyrics, LPCTSTR szName = NULL);
	int GetUserDefLyrics(ID3v2TextUserDefined &lyrics);

	int RemoveLyrics(LPCTSTR szName);

	int SetSynchLyrics(LPCTSTR szName, ID3v2SynchLyrics &lyrics);
	int SetUnsynchLyrics(LPCTSTR szName, LPCTSTR szDesc, LPCTSTR szLyrics);
	int SetUserDefLyrics(LPCTSTR szLyrics);

	virtual int GetTags(tstring &artist, tstring &title, tstring &album, tstring &comment, 
		tstring &track, tstring &year, tstring &genre);

	virtual int SetTags(LPCTSTR szArtist, LPCTSTR szTitle, LPCTSTR szAlbum, LPCTSTR szComment, 
		LPCTSTR szTrack, LPCTSTR szYear, LPCTSTR szGenre);

	int UpdateUserDefinedTextFrameByDesc(ID3v2TextUserDefined &text);
	int RemoveUserDefinedTextFrameByDesc(LPCTSTR szDesc);

	int UpdateGeneralCommentFrame(LPCTSTR szText);
	int RemoveGeneralCommentFrame();

	int UpdateGeneralUrlFrame(LPCTSTR szText);
	int RemoveGeneralUrlFrame();

	int GetPictures(ID3v2Pictures &pictures);
	int UpdatePictures(ID3v2Pictures &pictures);

	int UpdatePicFrame(ID3v2Pictures::ITEM *pic);
	int RemovePicFrame(ID3v2FrameUID_t nFrameUID, UINT picType);

protected:
	int UpdateTextFrame(UINT nFrameID, LPCTSTR szText);

	FrameIterator GetSyncLyricsFrame(LPCTSTR szName);
	FrameIterator GetUnsyncLyricsFrame(LPCTSTR szName);
	FrameIterator GetUserDefLyricsFrame();

	int SetUnsynchLyrics(LPCTSTR szName, ID3v2UnsynchLyrics &lyrics);

};
