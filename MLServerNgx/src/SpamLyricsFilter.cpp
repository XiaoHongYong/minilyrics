// SpamLyricsFilter.cpp: implementation of the CSpamLyricsFilter class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "SpamLyricsFilter.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

bool TextFileToVStr(LPCTSTR szFile, vector<string> &vStr);


#ifdef _UNICODE
bool TextFileToVStr(LPCTSTR szFile, vector<wstring> &vStr)
{
	FILE	*fp;
	char	szBuff[1024];
	wstringex	strW;

	fp = _tfopen(szFile, _T("rt"));
	if (!fp)
		return false;

	while (fgets(szBuff, CountOf(szBuff) - 1, fp))
	{
		int		nLen;
		nLen = strlen(szBuff);
		if (nLen > 0)
		{
			while (szBuff[nLen - 1] == '\r'
				|| szBuff[nLen - 1] == '\n')
				nLen--;
			if (nLen > 0)
			{
				szBuff[nLen] = '\0';
				ConvertStr(szBuff, -1, strW);
				vStr.push_back(strW.c_str());
			}
		}
	}
	fclose(fp);

	return true;
}
#endif

/*

bool TextFileToVStr(LPCTSTR szFile, vector<string> &vStr)
{
	FILE	*fp;
	char	szBuff[1024];

	fp = _tfopen(szFile, _T("rt"));
	if (!fp)
		return false;

	while (fgets(szBuff, CountOf(szBuff) - 1, fp))
	{
		int		nLen;
		nLen = strlen(szBuff);
		if (nLen > 0)
		{
			while (szBuff[nLen - 1] == '\r'
				|| szBuff[nLen - 1] == '\n')
				nLen--;
			if (nLen > 0)
			{
				szBuff[nLen] = '\0';
				vStr.push_back(szBuff);
			}
		}
	}
	fclose(fp);

	return true;
}
*/

bool ToFilterItem(vector<string> &vStr, CSpamLyricsFilter::CVFilterA &vFilter)
{
	int		nLevel;
	LPCSTR	str;
	for (int i = 0; i < (int)vStr.size(); i++)
	{
		nLevel = atoi(vStr[i].c_str());
		str = strstr(vStr[i].c_str(), ", ");
		if (!str)
			return false;
		vFilter.Add(str + 2, nLevel);
	}

	return true;
}

#ifdef _UNICODE
bool ToFilterItem(vector<wstring> &vStr, CSpamLyricsFilter::CVFilterW &vFilter)
{
	int		nLevel;
	LPCWSTR	str;
	for (int i = 0; i < (int)vStr.size(); i++)
	{
		nLevel = _wtoi(vStr[i].c_str());
		str = wcsstr(vStr[i].c_str(), L", ");
       if (!str)
           return false;
       vFilter.Add(str + 2, nLevel);
   }

	return true;
}
#endif


template<class XCHAR>
void GetLineInfo(const XCHAR *szText, int &nLineCount, int &nMaxWdith)
{
	const XCHAR		*szFindPos;
	int				nCur = 0;

	nLineCount = 1;
	nMaxWdith = 0;

	szFindPos = szText;

	while (*szFindPos)
	{
		if (*szFindPos == '\n')
		{
			if (nCur > nMaxWdith)
				nMaxWdith = nCur;
			nCur = 0;
			nLineCount++;
		}
		else
			nCur++;
		szFindPos++;
	}
	if (nCur > nMaxWdith)
		nMaxWdith = nCur;
}

template<class XCHAR>
void GetLrcTimeTagCount(const XCHAR *szText, int &nCount)
{
	ASSERT(szText);

	const XCHAR		*szPos;

	szPos = szText;
	nCount = 0;
	while (*szPos != '\0')
	{
		// [12:23.22]
		if (*szPos != '[')
		{
			szPos++;
			continue;
		}
		szPos++;

		// 12:23.22]
		if (!IsDigit(*szPos))
			continue;
		szPos++;

		while (IsDigit(*szPos))
			szPos++;

		// :23.22]
		if (*szPos != ':' && *szPos != '.')
			continue;
		szPos++;

		// 23.22]
		if (!IsDigit(*szPos))
			continue;
		szPos++;

		while (IsDigit(*szPos))
			szPos++;

		if (*szPos == ']')
		{
			// ]
			szPos++;
			nCount++;
			continue;
		}

		// .22]
		if (*szPos != '.' && *szPos != ':')
			continue;
		szPos++;

		// 22]
		if (!IsDigit(*szPos))
			continue;
		szPos++;

		while (IsDigit(*szPos))
			szPos++;

		// ]
		if (*szPos == ']')
		{
			szPos++;
			nCount++;
			continue;
		}
	}
}

// what is a fake .lrc?
// Time tags less than 3.
bool IsFakeLrc(stringex &str)
{
	int		nTimeTagaccount;

	if (GetFileEncodingByBom((LPBYTE)str.c_str()) == FE_UCS2)
		GetLrcTimeTagCount((LPCWSTR)str.c_str(), nTimeTagaccount);
	else
		GetLrcTimeTagCount(str.c_str(), nTimeTagaccount);

	return (nTimeTagaccount < 3);
}

#ifdef _UNICODE
bool ConvertFakeLrcToTxt(wstringex &str)
{
	LPCWSTR		szPos;
	bool		bConvert = false;
	int			i;

	static LPCWSTR		aszTagRemove[] =
	{
		L"[by:", L"[offset:", L"[dl:"
	};

	static LPCWSTR		aszTag[] = 
	{
		L"[ar:", L"[ti:", L"[al:", L"[artist:", L"[title:", L"[album:"
	};

	static LPCWSTR		aszTagTo[] = 
	{
		L"artist: ", L"title: ", L"album: ", L"artist: ", L"title: ", L"album: "
	};

	// replace info tag
	for (i = 0; i < CountOf(aszTag); i++)
	{
		LPCWSTR	szTag = aszTag[i];
		szPos = stristr(str.c_str(), szTag);
		if (szPos)
		{
			int		n;
			n = int(szPos - str.c_str());
			str.replace(n, wcslen(szTag), aszTagTo[i], wcslen(aszTagTo[i]));
			szPos = wcschr(str.c_str() + n, ']');
			if (szPos)
			{
				n = int(szPos - str.c_str());
				str.erase(n, 1);
			}
			bConvert = true;
		}
	}

	// remove info tag
	LPCWSTR	szEnd;
	for (i = 0; i < CountOf(aszTagRemove); i++)
	{
		LPCWSTR	szTag = aszTagRemove[i];
		szPos = stristr(str.c_str(), szTag);
		if (szPos)
		{
			int		n;
			n = int(szPos - str.c_str());
			szEnd = wcschr(szPos, ']');
			if (szEnd)
			{
				szEnd++;
				str.erase(n, int(szEnd - szPos));
			}
			bConvert = true;
		}
	}

	// remove time tag
	szPos = str.c_str();
	LPCWSTR		szBeg;
	while (*szPos)
	{
		if (*szPos == '[')
		{
			szBeg = szPos;
			szPos++;
			while (IsDigit(*szPos))
				szPos++;
			if (*szPos == ':' || *szPos == '.')
			{
				szPos++;
				while (IsDigit(*szPos))
					szPos++;
				if (*szPos == ']')
				{
					//  [00:21]
					szPos++;
					int		n = int(szBeg - str.c_str());
					str.erase(n, int(szPos - szBeg));
					szPos = str.c_str() + n;
					bConvert = true;
					continue;
				}
				else if (*szPos == '.' || *szPos == ':')
				{
					szPos++;
					while (IsDigit(*szPos))
						szPos++;
					if (*szPos == ']')
					{
						szPos++;
						int		n = int(szBeg - str.c_str());
						str.erase(n, int(szPos - szBeg));
						szPos = str.c_str() + n;
						bConvert = true;
						continue;
					}
				}
			}
		}
		szPos++;
	}

	return bConvert;
}
#endif

// remove .lrc tags, and convert to text file
bool ConvertFakeLrcToTxt(stringex &str)
{
	LPCSTR		szPos;
	bool		bConvert = false;
	int			i;

	static LPCSTR		aszTagRemove[] =
	{
		"[by:", "[offset:", "[dl:"
	};

	static LPCSTR		aszTag[] = 
	{
		"[ar:", "[ti:", "[al:", "[artist: ", "[title: ", "[album: "
	};

	static LPCSTR		aszTagTo[] = 
	{
		"artist: ", "title: ", "album: ", "artist: ", "title: ", "album: "
	};

	// replace info tag
	for (i = 0; i < CountOf(aszTag); i++)
	{
		LPCSTR	szTag = aszTag[i];
		szPos = stristr(str.c_str(), szTag);
		if (szPos)
		{
			int		n;
			n = int(szPos - str.c_str());
			str.replace(n, strlen(szTag), aszTagTo[i], strlen(aszTagTo[i]));
			szPos = strchr(str.c_str() + n, ']');
			if (szPos)
			{
				n = int(szPos - str.c_str());
				str.erase(n, 1);
			}
			bConvert = true;
		}
	}

	// remove info tag
	LPCSTR	szEnd;
	for (i = 0; i < CountOf(aszTagRemove); i++)
	{
		LPCSTR	szTag = aszTagRemove[i];
		szPos = stristr(str.c_str(), szTag);
		if (szPos)
		{
			int		n;
			n = int(szPos - str.c_str());
			szEnd = strchr(szPos, ']');
			if (szEnd)
			{
				szEnd++;
				str.erase(n, int(szEnd - szPos));
			}
			bConvert = true;
		}
	}

	// remove time tag
	szPos = str.c_str();
	LPCSTR		szBeg;
	while (*szPos)
	{
		if (*szPos == '[')
		{
			szBeg = szPos;
			szPos++;
			while (IsDigit(*szPos))
				szPos++;
			if (*szPos == ':' || *szPos == '.')
			{
				szPos++;
				while (IsDigit(*szPos))
					szPos++;
				if (*szPos == ']')
				{
					//  [00:21]
					szPos++;
					int		n = int(szBeg - str.c_str());
					str.erase(n, int(szPos - szBeg));
					szPos = str.c_str() + n;
					bConvert = true;
					continue;
				}
				else if (*szPos == '.' || *szPos == ':')
				{
					szPos++;
					while (IsDigit(*szPos))
						szPos++;
					if (*szPos == ']')
					{
						szPos++;
						int		n = int(szBeg - str.c_str());
						str.erase(n, int(szPos - szBeg));
						szPos = str.c_str() + n;
						bConvert = true;
						continue;
					}
				}
			}
		}
		szPos++;
	}

	return bConvert;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSpamLyricsFilter::CSpamLyricsFilter(LPCTSTR szCfgFilePrefix)
{
	m_nMinContentSize = 100;
	m_nMinContentLines = 7;
	strCfgFilePrefix = szCfgFilePrefix;
}

CSpamLyricsFilter::~CSpamLyricsFilter()
{

}

bool CSpamLyricsFilter::Load(LPCTSTR dataDir)
{
	tstring			strFile;
	stringex	buffer;
	vector<string>	vStr;
	vector<wstring>	vStrW;

	m_vContentFilter.clear();
#ifdef _UNICODE
	m_vContentFilterW.clear();
#endif

	m_nMinContentSize = g_Profile.GetInt(_T("MinContentSize"), 200);

#ifdef _UNICODE
	m_strFilterExceptW = g_Profile.GetString(_T("FilterExcept"), _T(""));
	ConvertStr(m_strFilterExceptW.c_str(), m_strFilterExceptW.size(), m_strFilterExcept);
#else
	m_strFilterExcept = g_Profile.GetString(_T("FilterExcept"), _T(""));
	ConvertStr(m_strFilterExcept.c_str(), m_strFilterExcept.size(), m_strFilterExceptW);
#endif

    if (dataDir) {
        strFile = dataDir;
    } else {
        strFile = GetAppResourceDir();
    }
	strFile += strCfgFilePrefix + _T("SpamNameFilter.txt");
	if (!OpenFileToBuffer(strFile.c_str(), buffer))
	{
		ERRLOG1(_T("Can't Open Filter File: %s"), strFile.c_str());
		return false;
	}
	ConvertStr(buffer.c_str(), buffer.size(), m_strNameFilter);
#ifdef _UNICODE
	TextFileToVStr(strFile.c_str(), vStrW);
	ToFilterItem(vStrW, m_vNameFilter);
#else
	TextFileToVStr(strFile.c_str(), vStr);
	ToFilterItem(vStr, m_vNameFilter);
#endif
//
// 	m_vNameFilter.Add(_T("www."));
// 	m_vNameFilter.Add(_T(".net"));
// 	m_vNameFilter.Add(_T(".com"));

    if (dataDir) {
        strFile = dataDir;
    } else {
        strFile = GetAppResourceDir();
    }
	strFile += strCfgFilePrefix + _T("SpamContentFilter.txt");
	if (!OpenFileToBuffer(strFile.c_str(), buffer))
	{
		ERRLOG1(_T("Can't Open Filter File: %s"), strFile.c_str());
		return false;
	}
	ConvertStr(buffer.c_str(), buffer.size(), m_strContentFilter);
	vStrW.clear();
#ifdef _UNICODE
	TextFileToVStr(strFile.c_str(), vStrW);
	ToFilterItem(vStrW, m_vContentFilterW);
#endif

	vStr.clear();
	TextFileToVStr(strFile.c_str(), vStr);
	ToFilterItem(vStr, m_vContentFilter);
    
#ifdef _UNICODE
	assert(m_vContentFilter.size() == m_vContentFilterW.size());
	if (m_vContentFilter.size() != m_vContentFilterW.size())
		return false;
#endif

	return true;
}

bool CSpamLyricsFilter::SetNameFilter(LPCTSTR szFilter)
{
	tstring			strFile;
	stringex	buffer;
	vector<tstring>	vStr;

	m_strNameFilter = szFilter;

	strFile = GetAppResourceDir();
	strFile += strCfgFilePrefix + _T("SpamNameFilter.txt");
	ConvertStr(m_strNameFilter.c_str(), m_strNameFilter.size(), buffer);
	if (!SaveDataAsFile(strFile.c_str(), buffer.c_str(), buffer.size()))
	{
		ERRLOG1(_T("Can't Save Filter File: %s"), strFile.c_str());
		return false;
	}
	TextFileToVStr(strFile.c_str(), vStr);
	ToFilterItem(vStr, m_vNameFilter);

	return true;
}

LPCTSTR CSpamLyricsFilter::GetNameFilter()
{
	return m_strNameFilter.c_str();
}

void CSpamLyricsFilter::SetFilterExcept(LPCTSTR szExcept)
{
#ifdef _UNICODE
	m_strFilterExceptW = szExcept;
	ConvertStr(m_strFilterExceptW.c_str(), m_strFilterExceptW.size(), m_strFilterExcept);
	g_Profile.WriteString(_T("FilterExcept"), m_strFilterExceptW.c_str());
#else
	m_strFilterExcept = szExcept;
	ConvertStr(m_strFilterExcept.c_str(), m_strFilterExcept.size(), m_strFilterExceptW);
	g_Profile.WriteString(_T("FilterExcept"), m_strFilterExcept.c_str());
#endif
}

bool CSpamLyricsFilter::SetContentFilter(LPCTSTR szFilter)
{
	tstring			strFile;
	stringex	buffer;
	vector<string>	vStr;
	vector<wstring>	vStrW;

	m_strContentFilter = szFilter;

	strFile = GetAppResourceDir();
	strFile += strCfgFilePrefix + _T("SpamContentFilter.txt");
	ConvertStr(m_strContentFilter.c_str(), m_strContentFilter.size(), buffer);
	if (!SaveDataAsFile(strFile.c_str(), buffer.c_str(), buffer.size()))
	{
		ERRLOG1(_T("Can't Save Filter File: %s"), strFile.c_str());
		return false;
	}
#ifdef _UNICODE
	TextFileToVStr(strFile.c_str(), vStrW);
	ToFilterItem(vStrW, m_vContentFilterW);
#endif
	TextFileToVStr(strFile.c_str(), vStr);
	ToFilterItem(vStr, m_vContentFilter);

	return true;
}

LPCTSTR CSpamLyricsFilter::GetContentFilter()
{
	return m_strContentFilter.c_str();
}

void CSpamLyricsFilter::SetMinContentSize(int nSize)
{
	m_nMinContentSize = nSize;
	g_Profile.WriteInt(_T("MinContentSize"), m_nMinContentSize);
}

bool CSpamLyricsFilter::IsNameFiltered(LPCTSTR szArtist, LPCTSTR szAlbum, LPCTSTR szTitle, LPCTSTR szFileName, int &nLevel, tstring *pstrReason)
{
/*	if (_tcslen(szTitle) <= 1 && (unsigned)(*szTitle) <= 255)
	{
		if (pstrReason)
			*pstrReason = _T("The count of characters in title should be larger than 1.");
		if (_tcslen(szTitle) == 0)
			nLevel = SF_ABS_SPAM;
		else
			nLevel = SF_NAME_ONE_CHAR;
		return true;
	}*/

	if (szFileName && _tcslen(szFileName) <= 5)
	{
		if (pstrReason)
			*pstrReason = _T("The count of characters in file name should be larger than 5.");
		nLevel = SF_ABS_SPAM;
		return true;
	}

	for (int i = 0; i < (int)m_vNameFilter.size(); i++)
	{
#ifdef _UNICODE
		ITEMW		&item = m_vNameFilter[i];
#else
		ITEMA		&item = m_vNameFilter[i];
#endif
		if (stristr(szArtist, item.str.c_str()))
		{
			if (pstrReason)
				*pstrReason = _T("Artist should not contain these text: ") + item.str;
			nLevel = item.nLevel;
			return true;
		}

		if (stristr(szAlbum, item.str.c_str()))
		{
			if (pstrReason)
				*pstrReason = _T("Album should not contain these text: ") + item.str;
			nLevel = item.nLevel;
			return true;
		}

		if (stristr(szTitle, item.str.c_str()))
		{
			if (pstrReason)
				*pstrReason = _T("Title should not contain these text: ") + item.str;
			nLevel = item.nLevel;
			return true;
		}

		if (szFileName && stristr(szFileName, item.str.c_str()))
		{
			if (pstrReason)
				*pstrReason = _T("File name should not contain these text: ") + item.str;
			nLevel = item.nLevel;
			return true;
		}
	}

	return false;
}

bool CSpamLyricsFilter::IsContentFiltered(LYRICS_CONTENT_TYPE lyrContentType, FILE_ENCODING fileEncoding, LPCSTR szContent, int nLen, int &nLevel, tstring *pstrReason)
{
	if (m_strFilterExceptW.size())
	{
		// Do not filter lyrics with keywords.
		if (fileEncoding == FE_UCS2)
		{
			if (stristr((const WCHAR *)szContent, m_strFilterExceptW.c_str()))
				return false;
		}
		else
		{
			if (stristr(szContent, m_strFilterExcept.c_str()))
				return false;
		}
	}

	// filter by content length
	if (nLen < m_nMinContentSize)
	{
		if (pstrReason)
			*pstrReason = CStrPrintf(_T("Lyrics content Length: %d is less than %d"), nLen, m_nMinContentSize).c_str();
		nLevel = SF_FILE_SIZE;
		return true;
	}

	if (nLen > 1024 * 200)
	{
		if (pstrReason)
			*pstrReason = CStrPrintf(_T("Lyrics content Length: %d is larger than 200KB"), nLen).c_str();
		nLevel = SF_FILE_SIZE;
		return true;
	}

	// filter spam info?
	if (fileEncoding == FE_UCS2)
	{
#ifdef _UNICODE
		for (int i = 0; i < (int)m_vContentFilterW.size(); i++)
		{
			if (stristr((const WCHAR *)szContent, m_vContentFilterW[i].str.c_str()))
			{
				if (pstrReason)
#ifdef _UNICODE
					*pstrReason = _T("Lyrics content should not contain these text: ") + m_vContentFilterW[i].str;
#else
					*pstrReason = _T("Lyrics content should not contain these text: ") + m_vContentFilter[i].str;
#endif
				nLevel = m_vContentFilterW[i].nLevel;
				return true;
			}
		}
#endif
	}
	else
	{
		for (int i = 0; i < (int)m_vContentFilter.size(); i++)
		{
			if (stristr(szContent, m_vContentFilter[i].str.c_str()))
			{
				if (pstrReason)
#ifdef _UNICODE
					*pstrReason = _T("Lyrics content should not contain these text: ") + m_vContentFilterW[i].str;
#else
					*pstrReason = _T("Lyrics content should not contain these text: ") + m_vContentFilter[i].str;
#endif
				nLevel = m_vContentFilter[i].nLevel;
				return true;
			}
		}
	}

	if (lyrContentType == LCT_LRC)
	{
		// the lrc file hasn't any synchronized time tag.
		int		nTimeTagaccount;

		if (fileEncoding == FE_UCS2)
			GetLrcTimeTagCount((LPCWSTR)szContent, nTimeTagaccount);
		else
			GetLrcTimeTagCount(szContent, nTimeTagaccount);

		if (nTimeTagaccount < 3)
		{
			// time tag is less than 3
			if (pstrReason)
				*pstrReason = CStrPrintf(_T("The lrc file is fake, only %d time tag defined."), nTimeTagaccount).c_str();
			nLevel = SF_FAKE_LRC;
			return true;
		}
	}
	else
	{
		// Is the file a lrc?
		int		nTimeTagaccount;

		if (fileEncoding == FE_UCS2)
			GetLrcTimeTagCount((LPCWSTR)szContent, nTimeTagaccount);
		else
			GetLrcTimeTagCount(szContent, nTimeTagaccount);

		if (nTimeTagaccount > 0)
		{
			if (pstrReason)
				*pstrReason = CStrPrintf(_T("The txt file is a lrc file, %d time tag defined."), nTimeTagaccount).c_str();
			nLevel = SF_TXT_TO_LRC;
			return true;
		}
	}

	if (lyrContentType == LCT_TXT)
	{
		// Filter by lines, only for lrc files.
		int nLineCount, nMaxWdith;

		if (fileEncoding == FE_UCS2)
			GetLineInfo((const WCHAR *)szContent, nLineCount, nMaxWdith);
		else
			GetLineInfo(szContent, nLineCount, nMaxWdith);

		if (nLineCount < m_nMinContentLines)
		{
			if (pstrReason)
				*pstrReason = CStrPrintf(_T("The count of lyrics lines: %d is less than %d"), nLineCount, m_nMinContentLines).c_str();
			nLevel = SF_FILE_LINES_TOO_LESS;
			return true;
		}
		else if (nMaxWdith >= 130 && nMaxWdith > nLen / nLineCount * 2)
		{
			// ��еĳ�����ƽ��ֵ��x����
			if (pstrReason)
				*pstrReason = CStrPrintf(_T("One or more lyrics lines are too long, please format them well.")).c_str();
			nLevel = SF_FILE_LINES_TOO_LONG;
			return true;
		}
	}

	return false;
}
