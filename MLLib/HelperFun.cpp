#include "StdAfx.h"
#include "HelperFun.h"
#ifdef WIN32
#ifdef _WINCE_
#define _mbschr		strchr
#else
#include "mbstring.h"
#endif
#endif
#include "LyricsTagReader.h"


/*
void TestMDStrToBin()
{
	char		szDig[33], szDig2[33];
	unsigned char dig[16], dig2[16];
	const char	*szStr;

	szStr = "hello???dgggggggggggga1233335!!!!!233";
	MD5Str(szStr, strlen(szStr), szDig);
	MDStrToBin(szDig, dig);
	MDString(szStr, strlen(szStr), dig2);
	if (memcmp(dig2, dig, 16) != 0)
		ERRLOG0("not equal");
	MDPrint(dig, szDig2);
	if (strcmp(szDig2, szDig) != 0)
		ERRLOG0("not equal");
}
*/

bool MDStrToBin(const char *szDigest, unsigned char *digest)
{
	unsigned int i;
	
	for (i = 0; i < 16; i++)
	{
		if (*szDigest >= 'A' && *szDigest <= 'Z')
			*digest = *szDigest - 'A' + 10;
		else if (*szDigest >= '0' && *szDigest <= '9')
			*digest = *szDigest - '0';
		else
			return false;

		szDigest++;
		*digest = *digest << 4;
		if (*szDigest >= 'A' && *szDigest <= 'Z')
			*digest |= *szDigest - 'A' + 10;
		else if (*szDigest >= '0' && *szDigest <= '9')
			*digest |= *szDigest - '0';
		else
			return false;
		digest++;
		szDigest++;
	}

	if (*szDigest != '\0')
		return false;

	return true;
}

//
// szVer1 > szVer2 return 1;
// szVer1 == szVer2 return 0;
// szVer1 < szVer2 return -1;
//
int CompareVersion(LPCTSTR szVer1, LPCTSTR szVer2)
{
	TCHAR	szStdVer1[32], szStdVer2[32];

	_tcscpy_safe(szStdVer1, 32, szVer1);
	TrimStr(szStdVer1);
	_tcscpy_safe(szStdVer2, 32, szVer2);
	TrimStr(szStdVer2);

	return _tcsicmp(szStdVer1, szStdVer2);
}

//
// szVer1 > szVer2 return 1;
// szVer1 == szVer2 return 0;
// szVer1 < szVer2 return -1;
//
int CompareVersion(LPCTSTR szVer1, int nMajor, int nMinor, int nBuild)
{
	TCHAR	szStdVer1[32], szStdVer2[32];

	_tcscpy_safe(szStdVer1, 32, szVer1);
	TrimStr(szStdVer1);

	_stprintf_s(szStdVer2, CountOf(szStdVer1), _T("%d.%d.%d"), nMajor, nMinor, nBuild);

	return _tcsicmp(szStdVer1, szStdVer2);
}

int GoDie()
{
	char		*p;
	p = (char *)"";

	strcpy_safe(p, 100, "xeta");

	int			*p1 = (int *)1;
	*p1 = 100;

	return 1;
}

//
//// 将字符串进行异或操作
//void XorString(char *szString, int nLen)
//{
//	for (int i = 0; i < nLen; i++)
//	{
//		szString[i] = szString[i] ^ 0xB5;
//	}
//}

//////////////////////////////////////////////////////////////////////////
//

const TCHAR *_SZ_SONG_EXT[] = {
	_T("mp3"), _T("669"), _T("aac"), _T("amf"), _T("aif"), _T("aiff"), _T("ape"), _T("avi"), _T("asf"), _T("au"), _T("cda"), 
	_T("far"), _T("it"), _T("kar"), _T("m2v"), _T("m3u"), _T("m4a"), _T("mac"), _T("mdz"), _T("mid"), _T("midi"), 
	_T("miz"), _T("mod"), _T("mp2"), _T("mp1"), _T("mpg"), _T("mp4"), _T("mpeg"), _T("mtm"), 
	_T("nst"), _T("nsa"), _T("nsv"), _T("ogg"), _T("okt"), _T("pls"), _T("ptm"), _T("rmi"), _T("s3m"), 
	_T("s3z"), _T("snd"), _T("stm"), _T("stz"), _T("voc"), _T("wav"), _T("wma"), _T("wmv"), _T("xm"), 
	_T("xmz"), _T("mkv"), _T("mka"), _T("ult"), _T("lrc"), _T("txt"), NULL
};

void FilterSongFileExt(LPTSTR szFile)
{
	LPTSTR	szDot;
	LPCTSTR	*szPos;

	szDot = _tcsrchr(szFile, '.');
	if (szDot)
	{
		szDot++;
		for (szPos = _SZ_SONG_EXT; *szPos; szPos++)
		{
			if (_tcsicmp(szDot, *szPos) == 0)
			{
				szDot--;
				szDot[0] = '\0';
				return;
			}
		}
	}
}

BOOL AnalysisLyricsFileNameEx(TCHAR szArtist[]/* out */, int nLenArtist, TCHAR szTitle[]/* out */, int nLenTitle, LPCTSTR szFileName)
{
	LPCTSTR		szEndArtist;
	LPCTSTR		szBegTitle;
	TCHAR		szFullTitleNew[MAX_PATH];

	//
	// 将FullTitle中的.mp3, .ra, .rm ....等替换掉
	_tcscpy_safe(szFullTitleNew, MAX_PATH, szFileName);
	FilterSongFileExt(szFullTitleNew);

	// 作者和标题之间是以：' - ' or ' － ' or '-'为分隔
	szEndArtist = szBegTitle = _tcsstr(szFullTitleNew, _T(" - "));
	if (szBegTitle)
		szBegTitle += 3;	// strlen(_T(" - "));
	else
	{
/* 不再专门对中文分割做处理
		szEndArtist = szBegTitle = _tcsstr(szFullTitleNew, _T(" － "));
		if (szBegTitle)
			szBegTitle += strlen(_T(" － "));
		else
*/		{
			szEndArtist = szBegTitle = _tcschr(szFullTitleNew, '-');
			if (szBegTitle)
				szBegTitle++;	// -
		}
	}


	//
	// 处理作者……
	if (szEndArtist == NULL)
	{
		//
		// 认为 szFullTitle 都是 Title, Artist 为空
		szBegTitle = szFullTitleNew;

		szArtist[0] = '\0';
	}
	else
	{
		_tcsncpysz_safe(szArtist, nLenArtist, szFullTitleNew, int(szEndArtist - szFullTitleNew));
	}

	_tcscpy_safe(szTitle, nLenTitle, szBegTitle);

	TrimStr(szTitle);
	TrimStr(szArtist);

	return TRUE;
}

const WCHAR *FindRightestCharOfLineUcs2(const WCHAR *szStr, WCHAR ch)
{
	const WCHAR	*szEndLine, *szEnd = NULL, *szChFound;
	szEndLine = wcschr(szStr, '\n');
	if (szEndLine)
	{
		szChFound = wcschr(szStr, ch);
		while (szChFound && szChFound + 1 < szEndLine)
		{
			szEnd = szChFound;
			if (szEnd[1] == '[')
				break;
			szChFound = wcschr(szChFound + 1, ch);
		}
	}
	else
	{
		szChFound = wcschr(szStr, ch);
		while (szChFound)
		{
			szEnd = szChFound;
			if (szEnd[1] == '[')
				break;
			szChFound = wcschr(szChFound + 1, ch);
		}
	}

	return szEnd;
}

const char *FindRightestCharOfLineMbcs(const char *szStr, char ch)
{
	const char	*szEndLine, *szEnd = NULL, *szChFound;
#if defined(WIN32) && !defined(_WINCE_)
typedef const unsigned char *		mbcstr;
#else
typedef const char *		mbcstr;
#endif
	szEndLine = (const char *)_mbschr((mbcstr)szStr, (unsigned int)'\n');
	if (szEndLine)
	{
		szChFound = (const char *)_mbschr((mbcstr)szStr, (unsigned int)ch);
		while (szChFound && szChFound + 1 <= szEndLine)
		{
			szEnd = szChFound;
			if (szEnd[1] == '[')	// a new tag begins?
				break;
			szChFound = (const char *)_mbschr((mbcstr)szChFound + 1, (unsigned int)ch);
		}
	}
	else
	{
		szChFound = (const char *)_mbschr((mbcstr)szStr, (unsigned int)ch);
		while (szChFound)
		{
			szEnd = szChFound;
			if (szEnd[1] == '[')
				break;
			szChFound = (const char *)_mbschr((mbcstr)szChFound + 1, (unsigned int)ch);
		}
	}

	return szEnd;
}

const char *FindRightestCharOfLineUtf8(const char *szStr, char ch)
{
	const char	*szEndLine, *szEnd = NULL, *szChFound;
	szEndLine = strchr(szStr, '\n');
	if (szEndLine)
	{
		szChFound = strchr(szStr, ch);
		while (szChFound && szChFound + 1 < szEndLine)
		{
			szEnd = szChFound;
			if (szEnd[1] == '[')
				break;
			szChFound = strchr(szChFound + 1, ch);
		}
	}
	else
	{
		szChFound = strchr(szStr, ch);
		while (szChFound)
		{
			szEnd = szChFound;
			if (szEnd[1] == '[')
				break;
			szChFound = strchr(szChFound + 1, ch);
		}
	}

	return szEnd;
}

void ConvertToUtf8(const WCHAR *str, int nLen, stringex &strOut, CHAR_ENCODING nEncodingID)
{
	UCS2ToUtf8(str, nLen, strOut);
}

void ConvertToUtf8(const char *str, int nLen, stringex &strOut, CHAR_ENCODING nEncodingID)
{
	MbcsToUtf8(str, nLen, strOut, nEncodingID);
}

BOOL IsEncryptedLyrics(LPCTSTR szFile)
{
	CLyricsTagReader<TCHAR>	lrcTagReader;
	CCharBuffer		str;
	tstringex		strKey;

	if (!OpenFileToBuffer(szFile, str))
		return FALSE;

	lrcTagReader.m_pstrKey = &strKey;
	if (lrcTagReader.ReadFromStr(str.c_str(), str.size(), false, false, ED_SYSDEF)
		&& strKey.size())
		return TRUE;

	return FALSE;
}

/*
template<class CHARDEF>
BOOL SetLrcLyricsTagValue_t(CCharBuffer &str, stringex_t<CHARDEF> &strName, stringex_t<CHARDEF> &strValue)
{
	const CHARDEF		*szBeg, *szPos;
	const CHARDEF		*szEnd;
	const CHARDEF		*wstr;
	FILE_ENCODING		fileEncoding = GetStrEncodingType(str.c_str());

	if (fileEncoding == FE_UCS2)
		wstr = (const CHARDEF *)((const BYTE *)str.c_str() + 2);
	else
		wstr = (const CHARDEF *)str.c_str();


	if (fileEncoding == FE_UCS2)
	{
		// unicode
		const WCHAR		*szBeg, *szPos;
		const WCHAR		*szEnd;
		const WCHAR		*wstr;
		wstr = (const WCHAR *)((const BYTE *)str + 2);

		szBeg = wstr;

		szPos = _xcsistr(szBeg, szName);

		if (szPos)
		{
			szPos += strlen(szName);

			szEnd = wcschr(szPos, ']');
			nPos = int((char *)szPos - str.data());
			str.replace(nPos, (size_t)(szEnd - szPos) * sizeof(CHARDEF),
				(const char *)strValue.c_str(), strValue.size() * sizeof(CHARDEF));
			if (szEnd)
			{
				ConvertStr(szPos, size_t(szEnd - szPos), strValue, nEncodingID);
				return TRUE;
			}
		}
	}
	else
	{
		// other encoding
		const char		*szBeg, *szPos;
		const char		*szEnd;

		szBeg = (const char *)str;

		szPos = _xcsistr(szBeg, szName);

		if (szPos)
		{
			szPos += strlen(szName);

			szEnd = (const char *)_mbschr((mbcstr)szPos, (unsigned int)']');
			if (szEnd)
			{
				ConvertStr(szPos, size_t(szEnd - szPos), strValue, nEncodingID);
				return TRUE;
			}
		}
	}

	szBeg = wstr;

	szPos = _xcsistr(szBeg, strName.c_str());

	if (szPos)
	{
		szPos += strName.size();
		szEnd = _xcschr(szPos, ']');
		if (szEnd)
		{
			int		nPos;

			nPos = int((char *)szPos - str.data());
			str.replace(nPos, (size_t)(szEnd - szPos) * sizeof(CHARDEF),
				(const char *)strValue.c_str(), strValue.size() * sizeof(CHARDEF));
			return TRUE;
		}
	}

	if (strValue.empty())
		return TRUE;

	stringex_t<CHARDEF>		strTemp;
	int						nOffset;

	strTemp = strName;
	strTemp += strValue;
	strTemp += ']';
	strTemp += '\r';
	strTemp += '\n';

	if (fileEncoding == FE_UCS2)
		nOffset = 2;
	else if (fileEncoding == FE_UTF8)
		nOffset = 3;
	else
		nOffset = 0;

	str.insert(nOffset, (const char *)strTemp.c_str(), strTemp.size() * sizeof(CHARDEF));

	return TRUE;
}
*/
/*
BOOL SetLrcLyricsTagValue(CCharBuffer &str, const char *szName, LPCTSTR szValue, int nEncodingID)
{
	int			nPos;
	FILE_ENCODING		fileEncoding = GetStrEncodingType(str.c_str());
	if (fileEncoding == FE_UCS2)
	{
		wstringex		strName, strValue;
		ConvertStr(szName, -1, strName, nEncodingID);
		ConvertStr(szValue, -1, strValue, nEncodingID);

		// unicode
		const WCHAR		*szBeg, *szPos;
		const WCHAR		*szEnd;
		const WCHAR		*wstr;
		wstr = (const WCHAR *)((const BYTE *)str.c_str() + 2);

		szBeg = wstr;

		szPos = _xcsistr(szBeg, szName);

		if (szPos)
		{
			szPos += strlen(szName);

			szEnd = FindRightestCharOfLineUcs2(szPos, ']');
			// szEnd = wcschr(szPos, ']');
			if (szEnd)
			{
				nPos = int((char *)szPos - str.data());
				str.replace(nPos, (size_t)(szEnd - szPos) * sizeof(WCHAR),
					(const char *)strValue.c_str(), strValue.size() * sizeof(WCHAR));

				return TRUE;
			}
		}
		if (!strValue.empty())
		{
			strName += strValue;
			strName += L"]\r\n";
			str.insert(2, (const char *)strName.c_str(), strName.size() * sizeof(WCHAR));
		}
	}
	else
	{
		stringex		strName, strValue;
		if (fileEncoding == FE_UTF8)
		{
#ifdef _UNICODE
			strName = szName;
			UCS2ToUtf8(szValue, -1, strValue);
#else
			strName = szName;
			MbcsToUtf8(szValue, -1, strValue, nEncodingID);
#endif
		}
		else
		{
			ConvertStr(szName, -1, strName, nEncodingID);
			ConvertStr(szValue, -1, strValue, nEncodingID);
		}
		const char		*szBeg, *szPos;
		const char		*szEnd;

		szBeg = (const char *)str;

		szPos = _xcsistr(szBeg, szName);

		if (szPos)
		{
			szPos += strlen(szName);

			if (fileEncoding == FE_UTF8)
				szEnd = FindRightestCharOfLineUtf8(szPos, ']');
			else
				szEnd = FindRightestCharOfLineMbcs(szPos, ']');
			// szEnd = (const char *)_mbschr((mbcstr)szPos, (unsigned int)']');
			if (szEnd)
			{
				nPos = int((char *)szPos - str.data());
				str.replace(nPos, (size_t)(szEnd - szPos) * sizeof(char),
					(const char *)strValue.c_str(), strValue.size() * sizeof(char));

				return TRUE;
			}
		}
		if (!strValue.empty())
		{
			int		nOffset;
			strName += strValue;
			strName += "]\r\n";
			if (fileEncoding == FE_UTF8)
				nOffset = 3;
			else
				nOffset = 0;
			str.insert(nOffset, (const char *)strName.c_str(), strName.size() * sizeof(char));
		}
	}

	return TRUE;
}

// [ti:party]
// [ar:Beyond]
// [al:音乐天地]
// [by:]
BOOL SaveLrcLyricsTag(LPCTSTR szFile, LPCTSTR szTitle, LPCTSTR szArtist, LPCTSTR szAlbum, LPCTSTR szOffsetTime, LPCTSTR szEncoding)
{
	CCharBuffer		str;
	int				nEncodingID = ED_SYSDEF;

	if (!OpenFileToBuffer(szFile, str))
		return FALSE;

	if (szEncoding)
		nEncodingID = GetEncodingID(szEncoding);
	else
	{
		tstringex		strEncoding;
		GetLrcLyricsTagValue(str.data(), str.size(), "[encoding:", strEncoding, ED_SYSDEF);
		if (strEncoding.size())
		{
			nEncodingID = GetEncodingID(strEncoding.c_str());
		}
	}

	if (szOffsetTime)
		SetLrcLyricsTagValue(str, "[offset:", szOffsetTime, nEncodingID);
	if (szAlbum)
		SetLrcLyricsTagValue(str, "[al:", szAlbum, nEncodingID);
	if (szTitle)
		SetLrcLyricsTagValue(str, "[ti:", szTitle, nEncodingID);
	if (szArtist)
		SetLrcLyricsTagValue(str, "[ar:", szArtist, nEncodingID);

	if (szEncoding)
	{
		BYTE			byTag[3];
		FILE_ENCODING	fileEncoding;

		fileEncoding = GetStrEncodingType(str.c_str());
		if (nEncodingID == ED_UTF8 && fileEncoding != FE_UTF8)
		{
			byTag[0] = 0xEF;
			byTag[1] = 0xBB;
			byTag[2] = 0xBF;
			str.insert(0, (const char *)byTag, 3);
		}
		else if (nEncodingID == ED_UNICODE && fileEncoding != FE_UCS2)
		{
			byTag[0] = 0xFF;
			byTag[1] = 0xFE;
			str.insert(0, (const char *)byTag, 2);
		}
		else
			SetLrcLyricsTagValue(str, "[encoding:", szEncoding, nEncodingID);
	}

	return SaveDataAsFile(szFile, str.c_str(), str.size());
}


//
//BOOL SaveLyricsTimeOffset(LPCTSTR szFile, int nTimeOffset)
//{
//	TCHAR		szOffset[64];
//
//	_stprintf(szOffset, _T("%d"), nTimeOffset);
//
//	return SaveLrcLyricsTag(szFile, NULL, NULL, NULL, szOffset, NULL);
//}



template<class CHARDEF>
BOOL SetTxtLyricsTagValue_t(CCharBuffer &str, stringex_t<CHARDEF> &strName, stringex_t<CHARDEF> &strValue)
{
	const CHARDEF		*szBeg, *szPos;
	const CHARDEF		*szEnd;
	const CHARDEF		*wstr;
	FILE_ENCODING		fileEncoding = GetStrEncodingType(str.c_str());

	if (fileEncoding == FE_UCS2)
		wstr = (const CHARDEF *)((const BYTE *)str.c_str() + 2);
	else
		wstr = (const CHARDEF *)str.c_str();

	szBeg = wstr;

	szPos = _xcsistr(szBeg, strName.c_str());

	if (szPos)
	{
		szPos += strName.size();
		szEnd = _xcsistr(szPos, "\r\n");
		if (szEnd)
		{
			int		nPos;

			nPos = int((char *)szPos - str.data());
			str.replace(nPos, (size_t)(szEnd - szPos) * sizeof(CHARDEF),
				(const char *)strValue.c_str(), strValue.size() * sizeof(CHARDEF));
			return TRUE;
		}
	}

	if (strValue.empty())
		return TRUE;

	stringex_t<CHARDEF>		strTemp;
	int						nOffset;

	strTemp = strName;
	strTemp += strValue;
	strTemp += '\r';
	strTemp += '\n';

	if (fileEncoding == FE_UCS2)
		nOffset = 2;
	else if (fileEncoding == FE_UTF8)
		nOffset = 3;
	else
		nOffset = 0;

	str.insert(nOffset, (const char *)strTemp.c_str(), strTemp.size() * sizeof(CHARDEF));

	return TRUE;
}

// example:
// encoding: gb2312
BOOL SetTxtLyricsTagValue(CCharBuffer &str, const char *szName, LPCTSTR szValue, int nEncodingID)
{
	FILE_ENCODING		fileEncoding = GetStrEncodingType(str.c_str());
	if (fileEncoding == FE_UCS2)
	{
		wstringex		strName, strValue;
		ConvertStr(szName, -1, strName, nEncodingID);
		ConvertStr(szValue, -1, strValue, nEncodingID);
		SetTxtLyricsTagValue_t(str, strName, strValue);
	}
	else
	{
		stringex		strName, strValue;
		if (fileEncoding == FE_UTF8)
		{
#ifdef _UNICODE
			strName = szName;
			UCS2ToUtf8(szValue, -1, strValue);
#else
			MbcsToUtf8(szName, -1, strName, nEncodingID);
			MbcsToUtf8(szValue, -1, strValue, nEncodingID);
#endif
		}
		else
		{
			ConvertStr(szName, -1, strName, nEncodingID);
			ConvertStr(szValue, -1, strValue, nEncodingID);
		}
		SetTxtLyricsTagValue_t(str, strName, strValue);
	}

	return TRUE;
}

// encoding: 
// Artist: 2 Be 3
// Album: Miscellaneous
// Title: Partir Un Jour
BOOL SaveTxtLyricsTag(LPCTSTR szFile, LPCTSTR szTitle, LPCTSTR szArtist, LPCTSTR szAlbum, LPCTSTR szEncoding)
{
	CCharBuffer		str;
	int				nEncodingID = ED_SYSDEF;

	if (!OpenFileToBuffer(szFile, str))
		return FALSE;

	if (szEncoding)
		nEncodingID = GetEncodingID(szEncoding);
	else
	{
		tstringex		strEncoding;
		GetTxtLyricsTagValue(str.data(), str.size(), "encoding: ", strEncoding, ED_SYSDEF);
		if (strEncoding.size())
		{
			nEncodingID = GetEncodingID(strEncoding.c_str());
		}
	}

	if (szAlbum)
		SetTxtLyricsTagValue(str, "Album: ", szAlbum, nEncodingID);
	if (szArtist)
		SetTxtLyricsTagValue(str, "Title: ", szArtist, nEncodingID);
	if (szTitle)
		SetTxtLyricsTagValue(str, "Artist: ", szTitle, nEncodingID);

	if (szEncoding)
	{
		BYTE			byTag[3];
		FILE_ENCODING	fileEncoding;

		fileEncoding = GetStrEncodingType(str.c_str());
		if (nEncodingID == ED_UTF8 && fileEncoding != FE_UTF8)
		{
			byTag[0] = 0xEF;
			byTag[1] = 0xBB;
			byTag[2] = 0xBF;
			str.insert(0, (const char *)byTag, 3);
		}
		else if (nEncodingID == ED_UNICODE && fileEncoding != FE_UCS2)
		{
			byTag[0] = 0xFF;
			byTag[1] = 0xFE;
			str.insert(0, (const char *)byTag, 2);
		}
		else
			SetTxtLyricsTagValue(str, "encoding: ", szEncoding, nEncodingID);
	}

	return SaveDataAsFile(szFile, str.c_str(), str.size());
}
*/

//////////////////////////////////////////////////////////////////////////
//

void AnalysisProxySetting(LPCTSTR szProxySetting, TCHAR szServer[], int nMaxSize, int &nPort)
{
	LPCTSTR		szColon;

	szColon = _tcschr(szProxySetting, ':');
	if (szColon == NULL)
	{
		_tcscpy_safe(szServer, nMaxSize, szProxySetting);
		nPort = 80;
	}
	else
	{
		_tcsncpy_safe(szServer, nMaxSize, szProxySetting,
			int(szColon - szProxySetting));
		nPort = _ttoi(szColon + 1);
	}
}

#ifdef WIN32
bool GetMLEncriptyData(stringex &strData)
{
	TCHAR			szFile[MAX_PATH];
	stringex		str;
	TCHAR			szMD5[64];

	GetModuleFileName(GetAppInstance(), szFile, CountOf(szFile));

	if (!OpenFileToBuffer(szFile, str))
		return false;

	MD5Str(str.c_str() + 0x100, str.size() - 0x100, szMD5);

	// 0x51 ~ 0x78
	// decodebase64(str.c_str() + 0x51, 0x78 - 0x51, strData);
	strData.append(str.c_str() + 0x51, 0x78 - 0x51);

	for (uint i = 0; i < 32; i++)
		strData[i] = strData[i] ^ szMD5[i];

	// 解密之后的数据了，这里存储的是核心数据

	return true;
}
#endif
