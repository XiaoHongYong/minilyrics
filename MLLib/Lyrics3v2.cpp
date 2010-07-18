// Lyrics3v2.cpp: implementation of the CLyrics3v2 class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MLLib.h"
#include "Lyrics3v2.h"
#include "../ID3Lib/ID3v1.h"
// #include "io.h"
#include "LyricsTagReader.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define _ID3_V1_LEN		128

void SetId3v1(char *szId3v1, const char *szTitle, const char *szArtist, const char * szAlbum, const char * szYear, const char *szComment, BYTE byTrack, BYTE byGenre)
{
	char	*szPtr;

	memset(szId3v1, 0, ID3_V1_LEN);
	memcpy(szId3v1, "TAG", ID3_V1_LEN_ID);

	szPtr = szId3v1 + ID3_V1_LEN_ID;
	if (szTitle)
		strcpy_safe(szPtr, ID3_V1_LEN_TITLE, szTitle);

	szPtr += ID3_V1_LEN_TITLE;
	if (szArtist)
		strcpy_safe(szPtr, ID3_V1_LEN_ARTIST, szArtist);

	szPtr += ID3_V1_LEN_ARTIST;
	if (szAlbum)
		strcpy_safe(szPtr, ID3_V1_LEN_ALBUM, szAlbum);

	szPtr += ID3_V1_LEN_ALBUM;
	if (szYear)
		strcpy_safe(szPtr, ID3_V1_LEN_YEAR, szYear);

	szPtr += ID3_V1_LEN_YEAR;
	if (szComment)
		strcpy_safe(szPtr, ID3_V1_LEN_COMMENT, szComment);

	szPtr += ID3_V1_LEN_COMMENT;
	*szPtr = (char)byTrack;

	szPtr += ID3_V1_LEN_TRACK;
	*szPtr = (char)byGenre;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLyrics3v2::CLyrics3v2()
{

}

CLyrics3v2::~CLyrics3v2()
{

}

#define ERR_GOTO_FAILED(p)		{ nErr=p; goto FAILED; }

int CLyrics3v2::ReadLyrcsInfo(LPCTSTR szMp3File, stringex &strLyrics)
{
	FILE		*fp = NULL;
	char		szTemp[256];
	int			nId3v1Len = 0;
	int			nField;
	int			nLyricsTagLen = 0;
	int			i;
	int			nErr = ERR_OK;

	fp = _tfopen(szMp3File, _T("rb"));
	if (!fp)
		ERR_GOTO_FAILED(ERR_OPEN_FILE);

	// ID3_V1_LEN = 128
	if (fseek(fp, -_ID3_V1_LEN, SEEK_END) != 0)
		ERR_GOTO_FAILED(ERR_SEEK_FILE);

	if (fread(szTemp, 1, 3, fp) != 3)
		ERR_GOTO_FAILED(ERR_READ_FILE);
	if (strncmp(szTemp, "TAG", 3) != 0)
	{
		// return 0;
		nId3v1Len = 0;
	}
	else
		nId3v1Len = _ID3_V1_LEN;

	// 9: LYRICS200
	if (fseek(fp, -(nId3v1Len + 9), SEEK_END) != 0)
		ERR_GOTO_FAILED(ERR_SEEK_FILE);

	// 9: LYRICS200
	if (fread(szTemp, 1, 9, fp) != 9)
		ERR_GOTO_FAILED(ERR_READ_FILE);
	if (strncmp(szTemp, "LYRICS200", 9) != 0)
		ERR_GOTO_FAILED(ERR_NOT_FIND_LRC3V2);

	// 9: LYRICS200
	if (fseek(fp, -(nId3v1Len + 9 + 6), SEEK_END) != 0)
		ERR_GOTO_FAILED(ERR_SEEK_FILE);

	// tag size
	if (fread(szTemp, 1, 6, fp) != 6)
		ERR_GOTO_FAILED(ERR_READ_FILE);
	szTemp[6] = '\0';
	nLyricsTagLen = atoi(szTemp);

	// 9: LYRICSBEGIN
	if (fseek(fp, -(nId3v1Len + 9 + 6 + nLyricsTagLen), SEEK_END) != 0)
		ERR_GOTO_FAILED(ERR_SEEK_FILE);

	// 9: LYRICSBEGIN
	if (fread(szTemp, 1, 11, fp) != 11)
		ERR_GOTO_FAILED(ERR_READ_FILE);
	if (strncmp(szTemp, "LYRICSBEGIN", 11) != 0)
		ERR_GOTO_FAILED(ERR_BAD_LRC3V2_FORMAT);

	for (i = nLyricsTagLen - 11; i > 0; )
	{
		char		szFiledName[16];
		if (fread(szFiledName, 1, 3, fp) != 3)
			ERR_GOTO_FAILED(ERR_READ_FILE);
		szFiledName[3] = '\0';

		if (fread(szTemp, 1, 5, fp) != 5)
			ERR_GOTO_FAILED(ERR_READ_FILE);
		szTemp[6] = '\0';

		nField = atoi(szTemp);
		i -= nField + 3 + 5;

		if (strncmp(szFiledName, "IND", 3) == 0)
		{
			ASSERT(nField < CountOf(szTemp) - 1);
			if (nField >= CountOf(szTemp) - 1)
				return ERR_BAD_LRC3V2_FORMAT;
			if (fread(szTemp, 1, nField, fp) != nField)
				ERR_GOTO_FAILED(ERR_READ_FILE);
			szTemp[nField] = '\0';
			// m_strInd = szTemp;
		}
		else if (strncmp(szFiledName, "LYR", 3) == 0)
		{
			strLyrics.reserve(nField);
			if (fread(strLyrics.data(), 1, nField, fp) != nField)
				ERR_GOTO_FAILED(ERR_READ_FILE);
			strLyrics.resize(nField);
			strLyrics.append("\0\0", 2);
			strLyrics.resize(nField);
		}
		else
		{
			if (fseek(fp, nField, SEEK_CUR) != 0)
				ERR_GOTO_FAILED(ERR_SEEK_FILE);
		}
	}

	fclose(fp);

	return nErr;

FAILED:
	if (fp)
		fclose(fp);

	return nErr;
}

int CLyrics3v2::WriteLyrcsInfo(LPCTSTR szMp3File, const char *szLyrics, int nLen, BOOL bTimeStamp)
{
	FILE		*fp = NULL;
	char		szTemp[256];
	int			nId3v1Len = 0;
	int			nLyricsTagLen = 0;
	int			nOffset = 0;
	char		szId3v1Tag[_ID3_V1_LEN + 1];
	int			nErr = ERR_OK;
	stringex	strAnsyLyr;

	FILE_ENCODING fe = GetStrEncodingType(szLyrics);
	if (fe == FE_UCS2)
	{
		ConvertStr((LPCWSTR)(szLyrics + 2), (nLen - 2) / sizeof(WCHAR), strAnsyLyr);

		// Only when converted ansi lyrics is same with org one, we shall convert it.
		wstringex	strUcs2;
		ConvertStr(strAnsyLyr.c_str(), strAnsyLyr.size(), strUcs2);
		if (wcsncmp((LPCWSTR)(szLyrics + 2), strUcs2.c_str(), strUcs2.size()) == 0)
		{
			szLyrics = strAnsyLyr.c_str();
			nLen = strAnsyLyr.size();
		}
	}
	else if (fe == FE_UTF8)
	{
		Utf8ToMbcs(szLyrics + 3, nLen - 3, strAnsyLyr);

		// Only when converted ansi lyrics is same with org one, we shall convert it.
		stringex	strUtf8;
		MbcsToUtf8(strAnsyLyr.c_str(), strAnsyLyr.size(), strUtf8);
		if (strncmp(szLyrics + 3, strUtf8.c_str(), strUtf8.size()) == 0)
		{
			szLyrics = strAnsyLyr.c_str();
			nLen = strAnsyLyr.size();
		}
	}


	fp = _tfopen(szMp3File, _T("r+b"));
	if (!fp)
	{
		SetCustomErrorDesc(CWin32Error());
		ERR_GOTO_FAILED(ERR_CUSTOM_ERROR);
	}

	// ID3_V1_LEN = 128
	if (fseek(fp, -_ID3_V1_LEN, SEEK_END) != 0)
		ERR_GOTO_FAILED(ERR_SEEK_FILE);

	if (fread(szTemp, 1, 3, fp) != 3)
		ERR_GOTO_FAILED(ERR_READ_FILE);
	if (strncmp(szTemp, "TAG", 3) != 0)
	{
		//
		// write tag info
		stringex	strTitle, strArtist, strAlbum, strEncoding;

		memset(szId3v1Tag, 0, _ID3_V1_LEN);
		memcpy(szId3v1Tag, "TAG", 3);

		//
		// 先从歌词标签中取得信息
		//
		CLyricsTagReader<char>		lrctagReader;
		lrctagReader.Set(&strTitle, &strArtist, &strAlbum);
		lrctagReader.ReadFromStr(szLyrics, nLen, false);
		strEncoding = lrctagReader.m_strEncoding;
		if (strTitle.empty())
		{
			// 从文件名中取得信息
			TCHAR		szArtist[MAX_PATH], szTitle[MAX_PATH];
			AnalysisLyricsFileNameEx(szArtist, CountOf(szArtist), szTitle, CountOf(szTitle), FileGetName(szMp3File));

			ConvertStr(szArtist, -1, strArtist, ED_SYSDEF);
			ConvertStr(szTitle, -1, strTitle, ED_SYSDEF);
		}

		SetId3v1(szId3v1Tag, strTitle.c_str(), strArtist.c_str(), strAlbum.c_str());

		nId3v1Len = 0;
	}
	else
	{
		if (fseek(fp, -_ID3_V1_LEN, SEEK_END) != 0)
			ERR_GOTO_FAILED(ERR_SEEK_FILE);
		if (fread(szId3v1Tag, 1, _ID3_V1_LEN, fp) != _ID3_V1_LEN)
			ERR_GOTO_FAILED(ERR_READ_FILE);
		nId3v1Len = _ID3_V1_LEN;
	}

	// 9: LYRICS200
	if (fseek(fp, -(nId3v1Len + 9), SEEK_END) != 0)
		ERR_GOTO_FAILED(ERR_SEEK_FILE);

	// 9: LYRICS200
	if (fread(szTemp, 1, 9, fp) != 9)
		ERR_GOTO_FAILED(ERR_READ_FILE);
	nOffset = nId3v1Len;
	if (strncmp(szTemp, "LYRICS200", 9) == 0)
	{
		if (fseek(fp, -(nId3v1Len + 9 + 6), SEEK_END) != 0)
			ERR_GOTO_FAILED(ERR_SEEK_FILE);

		// tag size
		if (fread(szTemp, 1, 6, fp) != 6)
			ERR_GOTO_FAILED(ERR_READ_FILE);
		szTemp[6] = '\0';
		nLyricsTagLen = atoi(szTemp);

		nOffset += 9 + 6 + nLyricsTagLen;

		// 9: LYRICSBEGIN
		if (fseek(fp, -nOffset, SEEK_END) != 0)
			ERR_GOTO_FAILED(ERR_SEEK_FILE);

		// 9: LYRICSBEGIN
		if (fread(szTemp, 1, 11, fp) != 11)
			ERR_GOTO_FAILED(ERR_READ_FILE);
		if (strncmp(szTemp, "LYRICSBEGIN", 11) != 0)
			ERR_GOTO_FAILED(ERR_BAD_LRC3V2_FORMAT);
	}

	if (fseek(fp, -nOffset, SEEK_END) != 0)
		ERR_GOTO_FAILED(ERR_SEEK_FILE);

	nLyricsTagLen = 0;
	if (fwrite("LYRICSBEGIN", 1, 11, fp) != 11)
		ERR_GOTO_FAILED(ERR_WRITE_FILE);
	nLyricsTagLen += 11;

	if (fwrite("IND00003", 1, 8, fp) != 8)
		ERR_GOTO_FAILED(ERR_WRITE_FILE);
	nLyricsTagLen += 8;

	if (bTimeStamp)
		szTemp[1] = '1';
	szTemp[0] = '1';
	szTemp[2] = '0';
	if (fwrite(szTemp, 1, 3, fp) != 3)
		ERR_GOTO_FAILED(ERR_WRITE_FILE);
	nLyricsTagLen += 3;

	if (fwrite("LYR", 1, 3, fp) != 3)
		ERR_GOTO_FAILED(ERR_WRITE_FILE);
	nLyricsTagLen += 3;

	if (nLen == -1)
		nLen = strlen(szLyrics);
	sprintf_s(szTemp, CountOf(szTemp), "%05d", nLen);
	if (fwrite(szTemp, 1, 5, fp) != 5)
		ERR_GOTO_FAILED(ERR_WRITE_FILE);
	nLyricsTagLen += 5;

	if (fwrite(szLyrics, 1, nLen, fp) != nLen)
		ERR_GOTO_FAILED(ERR_WRITE_FILE);
	nLyricsTagLen += nLen;

	sprintf_s(szTemp, CountOf(szTemp), "%06d", nLyricsTagLen);
	if (fwrite(szTemp, 1, 6, fp) != 6)
		ERR_GOTO_FAILED(ERR_WRITE_FILE);

	if (fwrite("LYRICS200", 1, 9, fp) != 9)
		ERR_GOTO_FAILED(ERR_WRITE_FILE);

	if (fwrite(szId3v1Tag, 1, _ID3_V1_LEN, fp) != _ID3_V1_LEN)
		ERR_GOTO_FAILED(ERR_WRITE_FILE);

	filetruncate(fp, ftell(fp));

	fclose(fp);

	return nErr;

FAILED:
	if (fp)
		fclose(fp);

	return nErr;
}

int CLyrics3v2::ClearLyricsInfo(LPCTSTR szMp3File)
{
	FILE		*fp = NULL;
	char		szTemp[256];
	int			nId3v1Len = 0;
	int			nLyricsTagLen = 0;
	int			nOffset = 0;
	char		szId3v1Tag[_ID3_V1_LEN + 1];
	int			nErr;

	fp = _tfopen(szMp3File, _T("r+b"));
	if (!fp)
		ERR_GOTO_FAILED(ERR_OPEN_FILE);

	// ID3_V1_LEN = 128
	if (fseek(fp, -_ID3_V1_LEN, SEEK_END) != 0)
		ERR_GOTO_FAILED(ERR_SEEK_FILE);

	if (fread(szTemp, 1, 3, fp) != 3)
		ERR_GOTO_FAILED(ERR_READ_FILE);
	if (strncmp(szTemp, "TAG", 3) != 0)
	{
		nId3v1Len = 0;
	}
	else
	{
		if (fseek(fp, -_ID3_V1_LEN, SEEK_END) != 0)
			ERR_GOTO_FAILED(ERR_SEEK_FILE);
		if (fread(szId3v1Tag, 1, _ID3_V1_LEN, fp) != _ID3_V1_LEN)
			ERR_GOTO_FAILED(ERR_READ_FILE);
		nId3v1Len = _ID3_V1_LEN;
	}

	// 9: LYRICS200
	if (fseek(fp, -(nId3v1Len + 9), SEEK_END) != 0)
		ERR_GOTO_FAILED(ERR_SEEK_FILE);

	// 9: LYRICS200
	if (fread(szTemp, 1, 9, fp) != 9)
		ERR_GOTO_FAILED(ERR_READ_FILE);
	nOffset = nId3v1Len;
	if (strncmp(szTemp, "LYRICS200", 9) == 0)
	{
		if (fseek(fp, -(nId3v1Len + 9 + 6), SEEK_END) != 0)
			ERR_GOTO_FAILED(ERR_SEEK_FILE);

		// tag size
		if (fread(szTemp, 1, 6, fp) != 6)
			ERR_GOTO_FAILED(ERR_READ_FILE);
		szTemp[6] = '\0';
		nLyricsTagLen = atoi(szTemp);

		nOffset += 9 + 6 + nLyricsTagLen;

		// 9: LYRICSBEGIN
		if (fseek(fp, -nOffset, SEEK_END) != 0)
			ERR_GOTO_FAILED(ERR_SEEK_FILE);

		// 9: LYRICSBEGIN
		if (fread(szTemp, 1, 11, fp) != 11)
			ERR_GOTO_FAILED(ERR_READ_FILE);
		if (strncmp(szTemp, "LYRICSBEGIN", 11) != 0)
			ERR_GOTO_FAILED(ERR_NOT_FIND_LRC3V2);
	}
	else
		ERR_GOTO_FAILED(ERR_NOT_FIND_LRC3V2);

	if (fseek(fp, -nOffset, SEEK_END) != 0)
		ERR_GOTO_FAILED(ERR_SEEK_FILE);

	if (nId3v1Len)
	{
		if (fwrite(szId3v1Tag, 1, _ID3_V1_LEN, fp) != _ID3_V1_LEN)
			ERR_GOTO_FAILED(ERR_WRITE_FILE);
	}

	filetruncate(fp, ftell(fp));

	fclose(fp);

	return ERR_OK;

FAILED:
	if (fp)
		fclose(fp);

	return nErr;
}
