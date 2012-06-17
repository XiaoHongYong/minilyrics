// ID3v1.h: interface for the CID3v1 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ID3V1_H__02584EC3_8F78_4370_8044_71FE6A670146__INCLUDED_)
#define AFX_ID3V1_H__02584EC3_8F78_4370_8044_71FE6A670146__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


enum ID3_V1Lengths
{
  ID3_V1_LEN         = 128,
  ID3_V1_LEN_ID      =   3,
  ID3_V1_LEN_TITLE   =  30,
  ID3_V1_LEN_ARTIST  =  30,
  ID3_V1_LEN_ALBUM   =  30,
  ID3_V1_LEN_YEAR    =   4,
  ID3_V1_LEN_COMMENT =  29,
  ID3_V1_LEN_TRACK   =   1,
  ID3_V1_LEN_GENRE   =   1
};

struct ID3V1
{
	char	szTitle[ID3_V1_LEN_TITLE + 1];
	char	szArtist[ID3_V1_LEN_ARTIST + 1];
	char	szAlbum[ID3_V1_LEN_ALBUM + 1];
	char	szYear[ID3_V1_LEN_YEAR + 1];
	char	szComment[ID3_V1_LEN_COMMENT + 1];
	byte	byTrack;
	byte	byGenre;
};

class CID3v1  
{
public:
	CID3v1();
	virtual ~CID3v1();

#pragma pack(push)
#pragma pack(1)
	struct ID3v1Data
	{
		char	szID3ID[ID3_V1_LEN_ID];
		char	szTitle[ID3_V1_LEN_TITLE];
		char	szArtist[ID3_V1_LEN_ARTIST];
		char	szAlbum[ID3_V1_LEN_ALBUM];
		char	szYear[ID3_V1_LEN_YEAR];
		char	szComment[ID3_V1_LEN_COMMENT];
		byte	byTrack;
		byte	byGenre;
	};
#pragma pack(pop)

	static LPCTSTR *GetSupportedExtArray();

	int GetTag(LPCTSTR szFile, ID3V1 *id3v1);
	int GetTag(FILE *fp, ID3V1 *id3v1);

	int SaveTag(LPCTSTR szFile, ID3V1 *id3v1);
	int SaveTag(FILE *fp, ID3V1 *id3v1);

	static LPCSTR* GetAllGenreDescription();
	static unsigned int GetGenreCount();
	static LPCSTR GetGenreDescription(unsigned int nGenre);
	static int GetGenreIndex(LPCSTR szGenre);

};

#endif // !defined(AFX_ID3V1_H__02584EC3_8F78_4370_8044_71FE6A670146__INCLUDED_)
