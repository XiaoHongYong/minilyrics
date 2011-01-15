// LyricsTagReader.cpp: implementation of the CLyricsTagReader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LrcTag.h"
#include "LrcParserHelper.h"

void TStringToStr(LPCTSTR str, int nLen, wstringex &strOut, int nEncodingID)
{
	TStringToUCS2(str, nLen, strOut, nEncodingID);
}

void TStringToStr(LPCTSTR str, int nLen, stringex &strOut, int nEncodingID)
{
	TStringToMBCS(str, nLen, strOut, nEncodingID);
}

void StrToTString(LPCSTR str, int nLen, tstringex &strOut, int nEncodingID)
{
	MBCSToTString(str, nLen, strOut, nEncodingID);
}

void StrToTString(LPCWSTR str, int nLen, tstringex &strOut, int nEncodingID)
{
	UCS2ToTString(str, nLen, strOut, nEncodingID);
}

bool IsTxtTagAtTailOfFile(LyrTagType tagType)
{
	return (tagType & (LTT_OFFSET | LTT_MEDIA_LENGTH | LTT_ID | LTT_ENCODING)) != 0;
}

int FormtLrcTimeTag(int nTime, TCHAR szBuff[])
{
	int		nMin, nSec, nMs;
	nTime += 10 / 2;

	nMs = (nTime / 10) % 100;
	nSec = (nTime / 1000) % 60;
	nMin = nTime / (1000 * 60);

	return _stprintf_s(szBuff, 16, _T("[%02d:%02d.%02d]"), nMin, nSec, nMs);
}

void FormatPlayTime(int nmsTime, TCHAR szBuf[])
{
	int		nMinute, nSec, nHour;

	nSec = nmsTime / 1000;
	nMinute = nSec / 60;
	nSec %= 60;
	nHour = nMinute / 60;
	nMinute %= 60;

	if (nHour > 0)
		_stprintf_s(szBuf, 32, _T("%d:%02d:%02d"), nHour, nMinute, nSec);
	else
		_stprintf_s(szBuf, 32, _T("%02d:%02d"), nMinute, nSec);
}

bool GetLRCTagValue(LPCSTR szLyrics, LPCSTR szTagPropName, tstring &strValue, CHAR_ENCODING encoding)
{
	const string	strPropName = szTagPropName;
	int		nValPosBeg, nValPosEnd;

	if (!SearchLrcPropTag(szLyrics, strPropName, nValPosBeg, nValPosEnd))
		return false;

	strValue.clear();
	MBCSToTString(szLyrics + nValPosBeg, nValPosEnd - nValPosBeg, strValue, encoding);

	TrimStr(strValue);

	return true;
}

bool GetTxtTagValue(LPCSTR szLyrics, LPCSTR szTagPropName, tstring &strValue, CHAR_ENCODING encoding)
{
	const string	strPropName = szTagPropName;
	int		nValPosBeg, nValPosEnd;

	if (!SearchTxtPropTag(szLyrics, strPropName, nValPosBeg, nValPosEnd))
		return false;

	strValue.clear();
	MBCSToTString(szLyrics + nValPosBeg, nValPosEnd - nValPosBeg, strValue, encoding);

	TrimStr(strValue);

	return true;
}

CHAR_ENCODING GetLyricsTextEncoding(LPBYTE &szLrc, int &nLen)
{
	CHAR_ENCODING	encoding;
	FILE_ENCODING	fe = GetStrEncodingType(szLrc);

	if (fe == FE_UTF8)
	{
		szLrc += 3;
		nLen -= 3;
		encoding = ED_UTF8;
	}
	else if (fe == FE_UCS2)
	{
		szLrc += 2;
		nLen -= 2;
		encoding = ED_UNICODE;
	}
	else if (fe == FE_UCS2_BE)
	{
		szLrc += 2;
		nLen -= 2;
		encoding = ED_UNICODE_BIG_ENDIAN;
	}
	else
	{
#ifdef _WIN32
		if (strlen((const char *)szLrc) <= 3 && nLen >= 10)
		{
			int		nFlag = IS_TEXT_UNICODE_UNICODE_MASK;
			if (IsTextUnicode((void *)szLrc, nLen, &nFlag))
			{
				encoding = ED_UNICODE;
				return encoding;
			}
		}
#endif

		// ANSI text, Retrieve the encoding in lyrics.
		tstring	strValue;
		if (GetLRCTagValue((const char *)szLrc, SZ_LRC_ENCODING, strValue, ED_SYSDEF))
			encoding = GetCharEncodingID(strValue.c_str());
		else if (GetTxtTagValue((const char *)szLrc, SZ_TXT_ENCODING, strValue, ED_SYSDEF))
			encoding = GetCharEncodingID(strValue.c_str());
		else
			encoding = ED_SYSDEF;
	}

	return encoding;
}

// Remove extra \0\0 inside of lyrics.
inline void BugFixOfExtraStrEnding(stringex &str)
{
	for (uint i = 0; i < str.size(); i++)
	{
		if (str[i] != '\0')
			continue;
		i++;
		if (i >= str.size())
			break;
		if (str[i] == '\0')
		{
			str.erase(i - 1, 2);
			i -= 2;
		}
	}
}

//////////////////////////////////////////////////////////////////////////

bool LyricsProperties::Equal(const LyricsProperties &other) const
{
	return m_strArtist == other.m_strArtist &&
		m_strTitle == other.m_strTitle &&
		m_strAlbum == other.m_strAlbum &&
		m_strBy == other.m_strBy &&
		m_strTimeOffset == other.m_strTimeOffset &&
		m_strMediaLength == other.m_strMediaLength &&
		m_nEncodingID == other.m_nEncodingID; 
}

void LyricsProperties::clear()
{
	m_strTitle.resize(0);
	m_strArtist.resize(0);
	m_strAlbum.resize(0);
	m_strBy.resize(0);
	m_strEncoding.resize(0);
	m_nTimeOffset = 0;
	m_strTimeOffset.resize(0);
	m_strMediaLength.resize(0);
	m_nEncodingID = ED_SYSDEF;
}

void LyricsProperties::SetEncoding(CHAR_ENCODING nEncodingID)
{
	m_nEncodingID = nEncodingID;
	m_strEncoding.clear();

	if (nEncodingID != ED_SYSDEF)
	{
		tstringex		str;
		MBCSToTString(GetCharEncodingByID(nEncodingID).szEncoding, -1, str);
		m_strEncoding = str.c_str();
	}
}

void LyricsProperties::SetEncoding(LPCTSTR szEncoding)
{
	m_strEncoding = szEncoding;

	m_nEncodingID = GetCharEncodingID(szEncoding);
}

void LyricsProperties::SetOffsetTime(int nOffsetTime)
{
	m_nTimeOffset = nOffsetTime;
	ASSERT(m_strTimeOffset.empty() || IsDigit(m_strTimeOffset.c_str()[0]) || m_strTimeOffset.c_str()[0] == '-');
	m_strTimeOffset = CStrPrintf(_T("%d"), nOffsetTime).c_str();
}

void LyricsProperties::SetMediaLength(int nMediaLength)
{
	if (nMediaLength > 0)
	{
		TCHAR	szBuf[64] = { 0 };
		FormatPlayTime(nMediaLength * 1000, szBuf);
		m_strMediaLength = szBuf;
	}
}

int LyricsProperties::GetMediaLengthInt() const
{
	if (m_strMediaLength.size() < 4)
		return 0;

	//_stprintf_s(szBuf, 32, _T("%d:%02d:%02d"), nHour, nMinute, nSec);
	// _stprintf_s(szBuf, 32, _T("%02d:%02d"), nMinute, nSec);

	int		n1, n2, n3;
	LPCTSTR	p = m_strMediaLength.c_str();
	p = ReadInt_t(p, n1);
	if (*p != ':')
		return 0;
	p++;

	p = ReadInt_t(p, n2);
	if (*p == '\0')
		return n1 * 60 +n2;

	if (*p != ':')
		return 0;
	p++;
	p = ReadInt_t(p, n3);

	return (n1 * 60 + n2) * 60 + n3;
}

//////////////////////////////////////////////////////////////////////////

CLyrTagNameValueMap::CLyrTagNameValueMap(LyricsProperties &prop, uint32 tagMask)
	: m_prop(prop)
{
	m_tagMask = tagMask;
	m_strEncoding = prop.GetEncoding();
	m_strOffset = prop.GetOffsetTimeStr();
	m_encoding = prop.GetEncodingIndex();

	// If offset is 0, don't write it.
	if (_tcscmp(m_strOffset.c_str(), _T("0")) == 0)
		m_strOffset.clear();

	Item		vNames[] = {
		{ SZ_LRC_AR,		SZ_TXT_AR,		LTT_ARTIST,		&(prop.m_strArtist) },
		{ SZ_LRC_TI,		SZ_TXT_TI,		LTT_TITLE,		&(prop.m_strTitle) },
		{ SZ_LRC_AL,		SZ_TXT_AL,		LTT_ALBUM,		&(prop.m_strAlbum) },
		{ SZ_LRC_BY,		"",				LTT_BY,			&(prop.m_strBy) },
		{ SZ_LRC_ENCODING,	SZ_TXT_ENCODING,LTT_ENCODING,	&m_strEncoding },
		{ SZ_LRC_OFFSET,	SZ_TXT_OFFSET,	LTT_OFFSET,		&m_strOffset },
		{ SZ_LRC_LENGTH,	SZ_TXT_LENGTH,	LTT_MEDIA_LENGTH,&(prop.m_strMediaLength) },
		{ SZ_LRC_ID,		SZ_TXT_ID,		LTT_ID,			&(prop.m_strId) },
	};

	for (int i = 0; i < CountOf(vNames); i++)
	{
		if (tagMask & vNames[i].tagType)
			m_vItems.push_back(vNames[i]);
	}
}

LyrTagType CLyrTagNameValueMap::GetTagType(int nIndex)
{
	ASSERT(nIndex >= 0 && nIndex < (int)m_vItems.size());
	return m_vItems[nIndex].tagType;
}

const string &CLyrTagNameValueMap::GetLrcName(int nIndex)
{
	ASSERT(nIndex >= 0 && nIndex < (int)m_vItems.size());
	return m_vItems[nIndex].strLrcName;
}

const string &CLyrTagNameValueMap::GetTxtName(int nIndex)
{
	ASSERT(nIndex >= 0 && nIndex < (int)m_vItems.size());
	return m_vItems[nIndex].strTxtName;
}

void CLyrTagNameValueMap::SetValue(int nIndex, LPCWSTR szValue, int nLen)
{
	ASSERT(nIndex >= 0 && nIndex < (int)m_vItems.size());

	tstring *pValue = m_vItems[nIndex].pstrValue;
	ASSERT(pValue);
	if (pValue->empty())
	{
		UCS2ToTString(szValue, nLen, *pValue, m_encoding);
		OnSetValue(nIndex);
	}
}

void CLyrTagNameValueMap::SetValue(int nIndex, LPCSTR szValue, int nLen)
{
	ASSERT(nIndex >= 0 && nIndex < (int)m_vItems.size());

	tstring *pValue = m_vItems[nIndex].pstrValue;
	if (pValue->empty())
	{
		MBCSToTString(szValue, nLen, *pValue, m_encoding);
		OnSetValue(nIndex);
	}
}

void CLyrTagNameValueMap::OnSetValue(int nIndex)
{
	ASSERT(nIndex >= 0 && nIndex < (int)m_vItems.size());

	tstring *pValue = m_vItems[nIndex].pstrValue;
	ASSERT(pValue);
	TrimStr(*pValue);

	if (m_vItems[nIndex].tagType == LTT_OFFSET)
		m_prop.SetOffsetTime(pValue->c_str());
	else if (m_vItems[nIndex].tagType == LTT_ENCODING)
		m_prop.SetEncoding(pValue->c_str());
}

const tstring &CLyrTagNameValueMap::GetValue(int nIndex)
{
	ASSERT(nIndex >= 0 && nIndex < (int)m_vItems.size());

	tstring *pValue = m_vItems[nIndex].pstrValue;
	return *pValue;
}

tstring CLyrTagNameValueMap::GetLrcTagString(int nIndex, bool bIncLineRet)
{
	ASSERT(nIndex >= 0 && nIndex < (int)m_vItems.size());

	tstring		strTag;
	tstring		strName;

	MBCSToTString(m_vItems[nIndex].strLrcName.c_str(), m_vItems[nIndex].strLrcName.size(), strName);

	strTag += _T("[");
	strTag += strName;
	strTag += _T(" ");
	strTag += *(m_vItems[nIndex].pstrValue);
	if (bIncLineRet)
		strTag += _T("]\r\n");
	else
		strTag += _T("]");
	return strTag;
}

tstring CLyrTagNameValueMap::GetTxtTagString(int nIndex, bool bIncLineRet)
{
	ASSERT(nIndex >= 0 && nIndex < (int)m_vItems.size());

	tstring		strTag;
	tstring		strName;

	MBCSToTString(m_vItems[nIndex].strTxtName.c_str(), m_vItems[nIndex].strTxtName.size(), strName);

	strTag += strName;
	strTag += _T(" ");
	strTag += *m_vItems[nIndex].pstrValue;
	if (bIncLineRet)
		strTag += _T("\r\n");
	return strTag;
}

void CLyrTagNameValueMap::Remove(int nIndex)
{
	ASSERT(nIndex >= 0 && nIndex < (int)m_vItems.size());

	m_vItems.erase(m_vItems.begin() + nIndex);
}

//////////////////////////////////////////////////////////////////////////

CLrcTag::CLrcTag(LyricsProperties &orgProp, LyricsProperties &newProp, uint32 tagMask)
{
	SetEncoding(newProp.GetEncodingIndex());

	m_tagMask = 0;
	if (newProp.m_strArtist != orgProp.m_strArtist)
	{
		m_strArtist = newProp.m_strArtist;
		m_tagMask |= LTT_ARTIST;
	}
	if (newProp.m_strTitle != orgProp.m_strTitle)
	{
		m_strTitle = newProp.m_strTitle;
		m_tagMask |= LTT_TITLE;
	}
	if (newProp.m_strAlbum != orgProp.m_strAlbum)
	{
		m_strAlbum = newProp.m_strAlbum;
		m_tagMask |= LTT_ALBUM;
	}
	if (newProp.m_strBy != orgProp.m_strBy)
	{
		m_strBy = newProp.m_strBy;
		m_tagMask |= LTT_BY;
	}
	if (_tcscmp(newProp.GetOffsetTimeStr(), orgProp.GetOffsetTimeStr()) != 0)
	{
		m_strTimeOffset = newProp.GetOffsetTimeStr();
		m_tagMask |= LTT_OFFSET;
	}
	if (_tcscmp(newProp.GetEncoding(), orgProp.GetEncoding()) != 0)
	{
		SetEncoding(newProp.GetEncoding());
		m_tagMask |= LTT_ENCODING;
	}
	if (newProp.m_strId != orgProp.m_strId)
	{
		m_strId = newProp.m_strId;
		m_tagMask |= LTT_ID;
	}
	if (newProp.m_strMediaLength != orgProp.m_strMediaLength)
	{
		m_strMediaLength = newProp.m_strMediaLength;
		m_tagMask |= LTT_MEDIA_LENGTH;
	}

	m_tagMask &= tagMask;
}


int CLrcTag::ReadFromFile(LPCTSTR szFile)
{
	stringex 		lyrics;

	if (!IsFileExist(szFile))
		return ERR_FILE_NOT_EXIST;

	if (!OpenFileToBuffer(szFile, lyrics))
		return ERR_READ_FILE;

	ReadFromText((LPBYTE)lyrics.data(), lyrics.size(), false, ED_SYSDEF);

	return ERR_OK;
}

bool CLrcTag::ReadFromText(LPBYTE str, int nLen, bool bUseSpecifiedEncoding, CHAR_ENCODING encoding)
{
	if (!bUseSpecifiedEncoding)
		encoding = GetLyricsTextEncoding(str, nLen);

	stringex	str2;

	if (encoding == ED_UNICODE_BIG_ENDIAN)
	{
		str2.append((LPCSTR)str, nLen);
		str2.append("\0\0", 2);
		str2.resize(str2.size() - 2);
		str = (LPBYTE)str2.data();
		UCS2BEToLE((WCHAR *)str, nLen / sizeof(WCHAR));
	}

	SetEncoding(encoding);

	if (encoding == ED_UNICODE || encoding == ED_UNICODE_BIG_ENDIAN)
		return ReadFromText((LPCWSTR)str, nLen / sizeof(WCHAR));
	else
		return ReadFromText((LPCSTR)str, nLen);
}

int CLrcTag::WriteToFile(LPCTSTR szFile, bool bLrcTag)
{
	stringex 		lyrics;

	if (!IsFileExist(szFile))
		return ERR_FILE_NOT_EXIST;

	if (!OpenFileToBuffer(szFile, lyrics))
		return ERR_READ_FILE;

	WriteToBuffer(lyrics, bLrcTag);

	if (!SaveDataAsFile(szFile, lyrics.c_str(), lyrics.size()))
		return ERR_OPEN_FILE;

	return ERR_OK;
}

void CLrcTag::WriteToBuffer(CCharBuffer &bufData, bool bLrcTag)
{
	BugFixOfExtraStrEnding(bufData);

	LPBYTE	pData = (LPBYTE)bufData.c_str();
	int		nLen = bufData.size();

	CHAR_ENCODING encoding = GetLyricsTextEncoding(pData, nLen);
	SetEncoding(encoding);

	if (encoding == ED_UNICODE || encoding == ED_UNICODE_BIG_ENDIAN)
	{
		wstringex		strW;
		strW.append((LPCWSTR)pData, nLen / sizeof(WCHAR));

		if (encoding == ED_UNICODE_BIG_ENDIAN)
			UCS2BEToLE(strW.data(), strW.size());

		WriteToText(strW, bLrcTag);

		if (encoding == ED_UNICODE_BIG_ENDIAN)
			UCS2LEToBE(strW.data(), strW.size());

		bufData.clear();
		if (encoding == ED_UNICODE_BIG_ENDIAN)
			bufData.append(SZ_FE_UCS2_BE, 2);
		else
			bufData.append(SZ_FE_UCS2, 2);
		bufData.append((LPCSTR)strW.c_str(), strW.size() * sizeof(WCHAR));
	}
	else
	{
		if (pData == (LPBYTE)bufData.data())
		{
			WriteToText(bufData, bLrcTag);
		}
		else
		{
			stringex	str;
			str.append((LPCSTR)pData, nLen);
			WriteToText(str, bLrcTag);
			bufData.clear();
			if (encoding == ED_UTF8)
				bufData.append(SZ_FE_UTF8, 3);
			bufData.append(str.c_str(), str.size());
		}
	}
}

template<class _TCHAR>
void TrimAppend(stringex_t<_TCHAR> &strTarget, const _TCHAR *str, int nLen)
{
	while (nLen > 0 && (str[nLen - 1] == '\r' || str[nLen - 1] == '\n' || str[nLen - 1] == ' '))
		nLen--;
	while (nLen > 0 && (*str == '\r' || *str == '\n' || *str == ' '))
	{
		str++;
		nLen--;
	}

	strTarget.append(str, nLen);
}

template<class _TCHAR>
bool LrcTagReadFromText(const _TCHAR *str, int nLen, CLyrTagNameValueMap &propNameValueMap, stringex_t<_TCHAR> &strLyrContent)
{
	strLyrContent.clear();

	const _TCHAR *szLine = str;
	bool	bFoundTag = false;

	while (szLine)
	{
		const _TCHAR * szNextLine = NextLine(szLine);
		szLine = IgnoreSpaces(szLine);
		int nLineLen = (szNextLine != NULL) ? int(szNextLine - szLine) : _xcslen(szLine);
		LyrTagType tagType = LTT_UNKNOWN;

		if (*szLine == '[')
		{
			if (IsDigit(szLine[1]) && propNameValueMap.ShouldReadLyrContentType())
			{
				if (IsLrcTimeTag(szLine))
					propNameValueMap.SetLyrContentType(LCT_LRC);
			}

			int		nValBeg, nValEnd;
			int		i;
			for (i = 0; i < propNameValueMap.GetCount(); i++)
			{
				if (ParseLrcPropTag(szLine, nLineLen, 
					propNameValueMap.GetLrcName(i), nValBeg, nValEnd))
				{
					bFoundTag = true;
					propNameValueMap.SetValue(i, szLine + nValBeg, nValEnd - nValBeg);
					tagType = propNameValueMap.GetTagType(i);
					break;
				}
			}
		}

		if (propNameValueMap.ShouldReadMd5() && tagType != LTT_ID)
			TrimAppend(strLyrContent, szLine, nLineLen);

		szLine = szNextLine;
	}

	return bFoundTag;
}

template<class _TCHAR>
bool TxtTagReadFromText(const _TCHAR *str, int nLen, CLyrTagNameValueMap &propNameValueMap, stringex_t<_TCHAR> &strLyrContent)
{
	strLyrContent.clear();

	const _TCHAR *szLine = str;
	bool	bFoundTag = false;

	while (szLine)
	{
		const _TCHAR * szNextLine = NextLine(szLine);
		IgnoreSpaces(szLine);
		int nLineLen = (szNextLine != NULL) ? int(szNextLine - szLine) : _xcslen(szLine);
		LyrTagType tagType = LTT_UNKNOWN;

		for (int i = 0; i < propNameValueMap.GetCount(); i++)
		{
			int		nValBeg, nValEnd;
			if (ParseTxtPropTag(szLine, nLineLen, 
				propNameValueMap.GetTxtName(i), nValBeg, nValEnd))
			{
				bFoundTag = true;
				propNameValueMap.SetValue(i, szLine + nValBeg, nValEnd - nValBeg);
				tagType = propNameValueMap.GetTagType(i);
				break;
			}
		}

		if (propNameValueMap.ShouldReadMd5() && tagType != LTT_ID)
			TrimAppend(strLyrContent, szLine, nLineLen);

		szLine = szNextLine;
	}

	return bFoundTag;
}

bool CLrcTag::ReadFromText(const char *str, int nLen)
{
	CLyrTagNameValueMap	propNameValueMap(*this, m_tagMask);

	stringex	strLyrContent;
	bool bFoundTag = LrcTagReadFromText(str, nLen, propNameValueMap, strLyrContent);
	if (!bFoundTag)
	{
		if (propNameValueMap.ShouldReadLyrContentType())
			propNameValueMap.SetLyrContentType(LCT_TXT);
		bFoundTag = TxtTagReadFromText(str, nLen, propNameValueMap, strLyrContent);
	}

	if (propNameValueMap.ShouldReadMd5())
		MD5Str((LPCSTR)strLyrContent.c_str(), strLyrContent.size() * sizeof(_TCHAR), m_szMd5OfLyrContent);

	return bFoundTag;
}

bool CLrcTag::ReadFromText(LPCWSTR str, int nLen)
{
	CLyrTagNameValueMap	propNameValueMap(*this, m_tagMask);

	wstringex	strLyrContent;
	bool bFoundTag = LrcTagReadFromText(str, nLen, propNameValueMap, strLyrContent);
	if (!bFoundTag)
	{
		if (propNameValueMap.ShouldReadLyrContentType())
			propNameValueMap.SetLyrContentType(LCT_TXT);
		bFoundTag = TxtTagReadFromText(str, nLen, propNameValueMap, strLyrContent);
	}

	if (propNameValueMap.ShouldReadMd5())
		MD5Str((LPCSTR)strLyrContent.c_str(), strLyrContent.size() * sizeof(_TCHAR), m_szMd5OfLyrContent);

	return bFoundTag;
}

template<class _TCHAR>
void LrcTagWriteToText(stringex_t<_TCHAR> &str, CLyrTagNameValueMap &propNameValueMap)
{
	typedef stringex_t<_TCHAR>	_tstringex;

	const _TCHAR *szLine = str.c_str();
	bool	bFoundTag = false;
	_tstringex	strValue;
	_TCHAR SZ_SPACE[] = { ' ', 0 };

	// Find existing tag, and replace it.
	while (szLine)
	{
		const _TCHAR * szNextLine = NextLine(szLine);
		const _TCHAR *szLineBeg = szLine;

		szLine = IgnoreSpaces(szLine);
		if (*szLine == '[' && !IsDigit(szLine[1]))
		{
			int nLineLen;
			if (szNextLine)
				nLineLen = int(szNextLine - szLine);
			else
				nLineLen = _xcslen(szLine);

			for (int i = 0; i < propNameValueMap.GetCount(); i++)
			{
				int		nValBeg, nValEnd;
				if (ParseLrcPropTag(szLine, nLineLen, 
					propNameValueMap.GetLrcName(i), nValBeg, nValEnd))
				{
					bFoundTag = true;
					const tstring &tstrValue = propNameValueMap.GetValue(i);

					if (tstrValue.empty())
					{
						// Remove the whole line
						int	nLineOffset = int(szLineBeg - str.c_str());
						str.erase(nLineOffset, nLineLen);
						szNextLine = str.c_str() + nLineOffset;
					}
					else
					{
						// Convert to current encoding string.
						TStringToStr(tstrValue.c_str(), tstrValue.size(), strValue, propNameValueMap.GetEncoding());
						strValue.insert(0, SZ_SPACE, 1);

						// Replace
						int	nLineOffset = int(szLine - str.c_str());
						str.replace(nLineOffset + nValBeg, nValEnd - nValBeg, strValue.c_str(), strValue.size());

						// Set Next line
						szLine = str.c_str() + nLineOffset + nValBeg;
						szNextLine = NextLine(szLine);
					}
					// Remove updated.
					propNameValueMap.Remove(i);
					break;
				}
			}
		}
		szLine = szNextLine;
	}

	// For left properties, insert at the head of file.
	_tstringex	strTagsAtHead;
	for (int i = propNameValueMap.GetCount() - 1; i >= 0; i--)
	{
		if (propNameValueMap.GetValue(i).empty())
			continue;

		tstring	strTag = propNameValueMap.GetLrcTagString(i);

		TStringToStr(strTag.c_str(), strTag.size(),
			strValue, propNameValueMap.GetEncoding());

		strTagsAtHead.append(strValue.c_str(), strValue.size());
	}
	if (strTagsAtHead.size())
		str.insert(0, strTagsAtHead.c_str(), strTagsAtHead.size());
}

template<class _TCHAR>
bool IsStrEndWithNewLine(stringex_t<_TCHAR> &str)
{
	return str.size() > 0 
		&& str[str.size() - 1] != '\r'
		&& str[str.size() - 1] != '\n';		
}

template<class _TCHAR>
void TxtTagWriteToText(stringex_t<_TCHAR> &str, CLyrTagNameValueMap &propNameValueMap)
{
	typedef stringex_t<_TCHAR>	_tstringex;

	const _TCHAR *szLine = str.c_str();
	bool	bFoundTag = false;
	_tstringex	strValue;
	_TCHAR SZ_SPACE[] = { ' ', 0 };
	_TCHAR _SZ_RETURN[] = { '\r', '\n', 0 };

	// Find existing tag, and replace it.
	while (szLine)
	{
		const _TCHAR * szNextLine = NextLine(szLine);
		const _TCHAR *szLineBeg = szLine;

		IgnoreSpaces(szLine);
		if (IsCharAlpha((byte)*szLine))
		{
			int nLineLen;
			if (szNextLine)
				nLineLen = int(szNextLine - szLine);
			else
				nLineLen = _xcslen(szLine);

			int		nValBeg, nValEnd;
			int		i;
			for (i = 0; i < propNameValueMap.GetCount(); i++)
			{
				if (ParseTxtPropTag(szLine, nLineLen, 
					propNameValueMap.GetTxtName(i), nValBeg, nValEnd))
				{
					bFoundTag = true;
					const tstring &tstrValue = propNameValueMap.GetValue(i);

					if (tstrValue.empty())
					{
						// Remove the whole line
						int	nLineOffset = int(szLineBeg - str.c_str());
						str.erase(nLineOffset, nLineLen);
						szNextLine = str.c_str() + nLineOffset;
					}
					else
					{
						// Convert to current encoding string.
						TStringToStr(tstrValue.c_str(), tstrValue.size(), strValue, propNameValueMap.GetEncoding());
						strValue.insert(0, SZ_SPACE, 1);

						// Replace
						int	nLineOffset = int(szLine - str.c_str());
						str.replace(nLineOffset + nValBeg, nValEnd - nValBeg, strValue.c_str(), strValue.size());

						// Set Next line
						szLine = str.c_str() + nLineOffset + nValBeg;
						szNextLine = NextLine(szLine);
					}
					// Remove updated.
					propNameValueMap.Remove(i);
					break;
				}
			}
		}
		szLine = szNextLine;
	}

	_tstringex	strTagsAtHead, strTagsAtTail;

	// For left properties, insert at the head or tail of file.
	for (int i = propNameValueMap.GetCount() - 1; i >= 0; i--)
	{
		if (propNameValueMap.GetValue(i).empty())
			continue;

		tstring	strTag = propNameValueMap.GetTxtTagString(i);

		TStringToStr(strTag.c_str(), strTag.size(),
			strValue, propNameValueMap.GetEncoding());

		if (IsTxtTagAtTailOfFile(propNameValueMap.GetTagType(i)))
			strTagsAtTail.append(strValue.c_str(), strValue.size());
		else
			strTagsAtHead.append(strValue.c_str(), strValue.size());
	}

	if (strTagsAtHead.size())
		str.insert(0, strTagsAtHead.c_str(), strTagsAtHead.size());

	if (strTagsAtTail.size())
	{
		// Append return if needed.
		if (IsStrEndWithNewLine(str))
			str.append(_SZ_RETURN);
		str.append(strTagsAtTail.c_str(), strTagsAtTail.size());
	}
}

void CLrcTag::WriteToText(wstringex &str, bool bLrcTag)
{
	CLyrTagNameValueMap	propNameValueMap(*this, m_tagMask);

	if (bLrcTag)
		LrcTagWriteToText(str, propNameValueMap);
	else
		TxtTagWriteToText(str, propNameValueMap);
}

void CLrcTag::WriteToText(stringex &str, bool bLrcTag)
{
	CLyrTagNameValueMap	propNameValueMap(*this, m_tagMask);

	if (bLrcTag)
		LrcTagWriteToText(str, propNameValueMap);
	else
		TxtTagWriteToText(str, propNameValueMap);
}

#ifdef _CPPUNIT_TEST

IMPLEMENT_CPPUNIT_TEST_REG(LrcTag)

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

class CTestCaseLrcTag : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(CTestCaseLrcTag);
	CPPUNIT_TEST(TestParseLrcTimeTag);
	CPPUNIT_TEST(TestParseLrcPropTag);
	CPPUNIT_TEST(TestParseTxtPropTag);
	CPPUNIT_TEST(TestReadLine);
	CPPUNIT_TEST(TestSearchLrcPropTag);
	CPPUNIT_TEST(TestSearchTxtPropTag);
	CPPUNIT_TEST(TestGetLRCTagValue);
	CPPUNIT_TEST(TestGetLyricsTextEncoding);
	CPPUNIT_TEST(TestBugFixOfExtraStrEnding);
	CPPUNIT_TEST(TestCLyrTagNameValueMap);
	CPPUNIT_TEST(TestCLrcTag_ReadFromText_Lrc);
	CPPUNIT_TEST(TestCLrcTag_WriteToText_Lrc);
	CPPUNIT_TEST(TestCLrcTag_WriteToFile_Lrc);
	CPPUNIT_TEST(TestCLrcTag_ReadFromText_Txt);
	CPPUNIT_TEST(TestCLrcTag_WriteToText_Txt);
	CPPUNIT_TEST_SUITE_END();

protected:

public:
	void setUp()
	{
	}
	void tearDown() 
	{
	}

protected:
	void TestParseLrcTimeTag()
	{
		// These are succeeded case.
		LPCTSTR		vTestTag[] = { _T("[1:30.001]"), _T("[ 10 : 03 . 01 ] abc"), _T("[01:30.1][abc") };
		int			vTime[] = { (1 * 60 + 30 ) * 1000 + 1, (10 * 60 + 3 ) * 1000 + 1 * 10, (1 * 60 + 30 ) * 1000 + 1 * 100 };
		LPCTSTR		vNextPosStr[] = { _T(""), _T(" abc"), _T("[abc") };

		int			nNextPos, nTime;
		for (int i = 0; i < CountOf(vTestTag); i++)
		{
			bool bRet = ParseLrcTimeTag(vTestTag[i], nNextPos, nTime);
			if (!bRet)
				CPPUNIT_FAIL_T(CStrPrintf(_T("ParseLrcTimeTag: failed, case: %d, %s"), i, vTestTag[i]).c_str());

			if (_tcsncmp(vTestTag[i] + nNextPos, vNextPosStr[i], _tcslen(vNextPosStr[i])) != 0)
				CPPUNIT_FAIL_T(CStrPrintf(_T("ParseLrcTimeTag: Wrong NextPos: %d, case: %d, %s"), nNextPos, i, vTestTag[i]).c_str());

			if (nTime != vTime[i])
				CPPUNIT_FAIL_T(CStrPrintf(_T("ParseLrcTimeTag: Wrong Time: %d, case: %d, %s"), nNextPos, i, vTestTag[i]).c_str());
		}

		// Failed case
		LPCTSTR		vTestTagFailed[] = { _T("[1:30.001 abc"), _T("[ 10a : 03 . 01 ]"), _T("[01:30.1x]") };
		for (int i = 0; i < CountOf(vTestTagFailed); i++)
		{
			bool bRet = ParseLrcTimeTag(vTestTagFailed[i], nNextPos, nTime);
			if (bRet)
				CPPUNIT_FAIL_T(CStrPrintf(_T("ParseLrcTimeTag(Failure test): failed, case: %d, %s"), i, vTestTagFailed[i]).c_str());
		}
	}

	void TestParseLrcPropTag()
	{
		// These are succeeded case.
		LPCTSTR		vTestTag[] = { _T("[ar:value"), _T("[ar:value]"), _T("[ar:]"), _T("[ ar:  value]aaaa"), _T("[ ar:v[xxx]alue]") };
		LPCTSTR		vValue[] = { _T("value"), _T("value"), _T(""), _T("  value"), _T("v[xxx]alue") };

		string		strPropName = SZ_LRC_AR;
		int			nValBegPos, nValEndPos;
		for (int i = 0; i < CountOf(vTestTag); i++)
		{
			bool bRet = ParseLrcPropTag(vTestTag[i], _tcslen(vTestTag[i]), strPropName, nValBegPos, nValEndPos);
			if (!bRet)
				CPPUNIT_FAIL_T(CStrPrintf(_T("ParseLrcPropTag: failed, case: %d, %s"), i, vTestTag[i]).c_str());

			if (_tcsncmp(vValue[i], vTestTag[i] + nValBegPos, int(nValEndPos - nValBegPos)) != 0
				|| _tcslen(vValue[i]) != int(nValEndPos - nValBegPos))
				CPPUNIT_FAIL_T(CStrPrintf(_T("ParseLrcPropTag: Wrong Pos: %d, %d, case: %d, %s"), nValBegPos, nValEndPos, i, vTestTag[i]).c_str());
		}

		// Failed case
		LPCTSTR		vTestTagFailed[] = { _T("[a r:value]aaaa"), _T("[a:rxxx]"), _T("[ar :rxxx]") };
		for (int i = 0; i < CountOf(vTestTagFailed); i++)
		{
			bool bRet = ParseLrcPropTag(vTestTagFailed[i], _tcslen(vTestTagFailed[i]), strPropName, nValBegPos, nValEndPos);
			if (bRet)
				CPPUNIT_FAIL_T(CStrPrintf(_T("ParseLrcPropTag(Failure test): failed, case: %d, %s"), i, vTestTagFailed[i]).c_str());
		}
	}

	void TestParseTxtPropTag()
	{
		// These are succeeded case.
		LPCTSTR		vTestTag[] = { _T("Artist: value"), _T("artist:value"), _T("Artist:") };
		LPCTSTR		vValue[] = { _T(" value"), _T("value"), _T("") };

		string		strPropName = SZ_TXT_AR;
		int			nValBegPos, nValEndPos;
		for (int i = 0; i < CountOf(vTestTag); i++)
		{
			bool bRet = ParseTxtPropTag(vTestTag[i], _tcslen(vTestTag[i]), strPropName, nValBegPos, nValEndPos);
			if (!bRet)
				CPPUNIT_FAIL_T(CStrPrintf(_T("ParseTxtPropTag: failed, case: %d, %s"), i, vTestTag[i]).c_str());

			if (_tcsncmp(vValue[i], vTestTag[i] + nValBegPos, int(nValEndPos - nValBegPos)) != 0)
				CPPUNIT_FAIL_T(CStrPrintf(_T("ParseTxtPropTag: Wrong Pos: %d, %d, case: %d, %s"), nValBegPos, nValEndPos, i, vTestTag[i]).c_str());
		}

		// Failed case
		LPCTSTR		vTestTagFailed[] = { _T("Artist :value"), _T("Artist"), _T("A rtist:") };
		for (int i = 0; i < CountOf(vTestTagFailed); i++)
		{
			bool bRet = ParseTxtPropTag(vTestTagFailed[i], _tcslen(vTestTagFailed[i]), strPropName, nValBegPos, nValEndPos);
			if (bRet)
				CPPUNIT_FAIL_T(CStrPrintf(_T("ParseTxtPropTag(Failure test): failed, case: %d, %s"), i, vTestTagFailed[i]).c_str());
		}
	}

	void TestReadLine()
	{
#define SZ_TEXT_TEST	_T("L1\r\nL2 \n L3 \nL4\nL5\n\n")

		LPCTSTR		vValue[] = { _T("L1"), _T("L2 "), _T(" L3 "), _T("L4"), _T("L5"), _T("") };
		LPCTSTR		szLine = SZ_TEXT_TEST;

		tstringex	strLine;
		int			i = 0;
		while ((szLine = ReadLine(szLine, strLine)) != NULL)
		{
			if (_tcscmp(strLine.c_str(), vValue[i]) != 0)
				CPPUNIT_FAIL_T(CStrPrintf(_T("ReadLine: failed, case: %d, %s, value: %s"), i, szLine, strLine.c_str()).c_str());
			i++;
		}
		if (i != CountOf(vValue))
			CPPUNIT_FAIL_T(CStrPrintf(_T("ReadLine: Readed line Count NOT match: %d, %d"), i, CountOf(vValue)).c_str());

		// Test for NextLine
		szLine = SZ_TEXT_TEST;
		i = 0;
		while (szLine && *szLine)
		{
			if (_tcsncmp(szLine, vValue[i], _tcslen(vValue[i])) != 0)
				CPPUNIT_FAIL_T(CStrPrintf(_T("NextLine: failed, case: %d, %s, value: %s"), i, szLine, vValue[i]).c_str());
			szLine = NextLine(szLine);
			i++;
		}

		if (i != CountOf(vValue))
			CPPUNIT_FAIL_T(CStrPrintf(_T("NextLine: Readed line Count NOT match: %d, %d"), i, CountOf(vValue)).c_str());
	}

	void TestSearchLrcPropTag()
	{
		// These are succeeded cases.
		LPCTSTR		vTestTag[] = { _T("[ ti:value]\n[ar:val2]\n[ti:title]"), _T("   [TI:val ue]"), _T("[xx]\n  [Ti:value]\n[ar:val]") };
		LPCTSTR		vValue[] = { _T("value"), _T("val ue"), _T("value") };

		string		strPropName = SZ_LRC_TI;
		int			nValBegPos, nValEndPos;
		for (int i = 0; i < CountOf(vTestTag); i++)
		{
			bool bRet = SearchLrcPropTag(vTestTag[i], strPropName, nValBegPos, nValEndPos);
			if (!bRet)
				CPPUNIT_FAIL_T(CStrPrintf(_T("SearchLrcPropTag: failed, case: %d, %s"), i, vTestTag[i]).c_str());

			if (_tcsncmp(vValue[i], vTestTag[i] + nValBegPos, int(nValEndPos - nValBegPos)) != 0)
				CPPUNIT_FAIL_T(CStrPrintf(_T("SearchLrcPropTag: Wrong Pos: %d, %d, case: %d, %s"), nValBegPos, nValEndPos, i, vTestTag[i]).c_str());
		}

		// Failed case
		LPCTSTR		vTestTagFailed[] = { _T("[t i:value]\n[ar:val2]"), _T("  [ti val ue]") };
		for (int i = 0; i < CountOf(vTestTagFailed); i++)
		{
			bool bRet = SearchLrcPropTag(vTestTagFailed[i], strPropName, nValBegPos, nValEndPos);
			if (bRet)
				CPPUNIT_FAIL_T(CStrPrintf(_T("ParseTxtPropTag(Failure test): failed, case: %d, %s"), i, vTestTagFailed[i]).c_str());
		}
	}

	void TestSearchTxtPropTag()
	{
		// These are succeeded cases. encoding: iso-8859-15\r\nArtist: Alexandra Burke\r\nTitle: Candyman
		LPCTSTR		vTestTag[] = { _T("Title:value\nArtist:val2\nTitle:title"), _T("   title:val ue"), _T("[xx]\n  Title:value\nArtist:val") };
		LPCTSTR		vValue[] = { _T("value"), _T("val ue"), _T("value") };

		string		strPropName = SZ_TXT_TI;
		int			nValBegPos, nValEndPos;
		for (int i = 0; i < CountOf(vTestTag); i++)
		{
			bool bRet = SearchTxtPropTag(vTestTag[i], strPropName, nValBegPos, nValEndPos);
			if (!bRet)
				CPPUNIT_FAIL_T(CStrPrintf(_T("SearchTxtPropTag: failed, case: %d, %s"), i, vTestTag[i]).c_str());

			if (_tcsncmp(vValue[i], vTestTag[i] + nValBegPos, int(nValEndPos - nValBegPos)) != 0)
				CPPUNIT_FAIL_T(CStrPrintf(_T("SearchTxtPropTag: Wrong Pos: %d, %d, case: %d, %s"), nValBegPos, nValEndPos, i, vTestTag[i]).c_str());
		}

		// Failed case
		LPCTSTR		vTestTagFailed[] = { _T("Ti tle:value\nAr:val2"), _T("Title val ue") };
		for (int i = 0; i < CountOf(vTestTagFailed); i++)
		{
			bool bRet = SearchTxtPropTag(vTestTagFailed[i], strPropName, nValBegPos, nValEndPos);
			if (bRet)
				CPPUNIT_FAIL_T(CStrPrintf(_T("SearchTxtPropTag(Failure test): failed, case: %d, %s"), i, vTestTagFailed[i]).c_str());
		}
	}

	void TestGetLRCTagValue()
	{
		// These are succeeded cases.
		LPCSTR		vTestTag[] = { "[ ti:value]\n[ar:val2]\n[ti:title]", "   [ti:val ue]\n[ti:title]", "[xx]\n  [ti:value]\n[ar:val]" };
		LPCTSTR		vValue[] = { _T("value"), _T("val ue"), _T("value") };

		tstring		strValue;
		for (int i = 0; i < CountOf(vTestTag); i++)
		{
			bool bRet = GetLRCTagValue(vTestTag[i], SZ_LRC_TI, strValue, ED_SYSDEF);
			if (!bRet)
				CPPUNIT_FAIL_T(CStrPrintf(_T("GetLRCTagValue: failed, case: %d, %s"), i, vTestTag[i]).c_str());

			if (_tcscmp(vValue[i], strValue.c_str()) != 0)
				CPPUNIT_FAIL_T(CStrPrintf(_T("GetLRCTagValue: Wrong Value: %s, case: %d, %s"), strValue.c_str(), i, vTestTag[i]).c_str());
		}

		// Failed case
		LPCSTR		vTestTagFailed[] = {"[t i:value]\n[ar:val2]", "  [ti val ue]" };
		for (int i = 0; i < CountOf(vTestTagFailed); i++)
		{
			bool bRet = GetLRCTagValue(vTestTagFailed[i], SZ_LRC_TI, strValue, ED_SYSDEF);
			if (bRet)
				CPPUNIT_FAIL_T(CStrPrintf(_T("GetLRCTagValue(Failure test): failed, case: %d, %s"), i, vTestTagFailed[i]).c_str());
		}
	}

	void TestGetLyricsTextEncoding()
	{
		// These are succeeded cases.
		{
			// ANSI file testing...
			LPCSTR		vTestTag[] = { "[ encoding:gb2312]\n[ar:val2]\n[ti:title]", "   [encoding:big5]", "[xx]\n  [encoding:utf-8]\n[ar:val]\n[ti:title]",
				" encoding:gb2312\nArtist:val2\n[", "   encoding:big5", "[xx]\n  encoding:utf-8\n",
				SZ_FE_UCS2 "abc", SZ_FE_UCS2_BE "abc", SZ_FE_UTF8 "abc", ""
				};
			CHAR_ENCODING	vEncoding[] = { ED_GB2312, ED_BIG5, ED_UTF8,
				ED_GB2312, ED_BIG5, ED_UTF8,
				ED_UNICODE, ED_UNICODE_BIG_ENDIAN, ED_UTF8, ED_SYSDEF
				};

			tstring		strValue;
			for (int i = 0; i < CountOf(vTestTag); i++)
			{
				LPBYTE p = (LPBYTE)vTestTag[i];
				int	nLen = strlen(vTestTag[i]);
				CHAR_ENCODING encoding = GetLyricsTextEncoding(p, nLen);
				if (encoding != vEncoding[i])
					CPPUNIT_FAIL_T(CStrPrintf(_T("GetLyricsTextEncoding(ANSI): failed, case: %d, %s"), i, vTestTag[i]).c_str());
			}
		}
	}

	void TestBugFixOfExtraStrEnding()
	{
#define SZ_1ZERO	"abc\0a"
#define SZ_2ZERO	"abc\0\0a"
#define SZ_3ZERO	"abc\0\0\0a"

		stringex	str;

		str.clear();
		str.append(SZ_1ZERO, CountOf(SZ_1ZERO));
		BugFixOfExtraStrEnding(str);
		if (str.size() != CountOf(SZ_1ZERO))
			CPPUNIT_FAIL("BugFixOfExtraStrEnding: failed, case 1");

		str.clear();
		str.append(SZ_2ZERO, CountOf(SZ_2ZERO));
		BugFixOfExtraStrEnding(str);
		if (str.size() != CountOf(SZ_2ZERO) - 2)
			CPPUNIT_FAIL("BugFixOfExtraStrEnding: failed, case 2");

		str.clear();
		str.append(SZ_3ZERO, CountOf(SZ_3ZERO));
		BugFixOfExtraStrEnding(str);
		if (str.size() != CountOf(SZ_3ZERO) - 2)
			CPPUNIT_FAIL("BugFixOfExtraStrEnding: failed, case 3");
	}

	void TestCLyrTagNameValueMap()
	{
		LyricsProperties		prop;

		prop.m_strArtist = _T("Artist");
		prop.m_strTitle = _T("Title");
		prop.m_strAlbum = _T("Album");
		prop.m_strBy = _T("By");
		prop.m_strId = _T("Id");
		// prop.SetEncoding(ED_GB2312);
		prop.SetOffsetTime(1000);

		prop.m_strMediaLength = _T("01:32");
		CPPUNIT_ASSERT(prop.GetMediaLengthInt() == 60 + 32);

		prop.m_strMediaLength = _T("2:11:32");
		CPPUNIT_ASSERT(prop.GetMediaLengthInt() == (2 * 60 + 11) * 60 + 32);

		tstring					strEncoding = prop.GetEncoding(), strOffset = prop.GetOffsetTimeStr();
		CLyrTagNameValueMap		nameValueMap(prop);

		CLyrTagNameValueMap::Item		vNames[] = {
			{ SZ_LRC_AR,		SZ_TXT_AR,		LTT_ARTIST,		&(prop.m_strArtist) },
			{ SZ_LRC_TI,		SZ_TXT_TI,		LTT_TITLE,		&(prop.m_strTitle) },
			{ SZ_LRC_AL,		SZ_TXT_AL,		LTT_ALBUM,		&(prop.m_strAlbum) },
			{ SZ_LRC_BY,		"",				LTT_BY,			&(prop.m_strBy) },
			{ SZ_LRC_ENCODING,	SZ_TXT_ENCODING,LTT_ENCODING,	&strEncoding },
			{ SZ_LRC_OFFSET,	SZ_TXT_OFFSET,	LTT_OFFSET,		&strOffset },
			{ SZ_LRC_LENGTH,	SZ_TXT_LENGTH,	LTT_MEDIA_LENGTH,&(prop.m_strMediaLength) },
			{ SZ_LRC_ID,		SZ_TXT_ID,		LTT_ID,			&(prop.m_strId) },
		};

		ASSERT(nameValueMap.GetTagType(4) == LTT_ENCODING);
		nameValueMap.SetValue(4, "big5", -1);
		strEncoding = _T("big5");

		for (int k = 0; k < nameValueMap.GetCount(); k++)
		{
			LyrTagType tagType = nameValueMap.GetTagType(k);
			bool		bFound = false;
			for (int i = 0; i < CountOf(vNames); i++)
			{
				if (tagType != vNames[i].tagType)
					continue;

				// make sure 
				if (strcmp(vNames[i].strLrcName.c_str(), nameValueMap.GetLrcName(k).c_str()) != 0)
					CPPUNIT_FAIL_T(CStrPrintf(_T("CLyrTagNameValueMap: case: %d, %d"), k, i).c_str());

				if (strcmp(vNames[i].strTxtName.c_str(), nameValueMap.GetTxtName(k).c_str()) != 0)
					CPPUNIT_FAIL_T(CStrPrintf(_T("CLyrTagNameValueMap: case: %d, %d"), k, i).c_str());

				if (_tcscmp(vNames[i].pstrValue->c_str(), nameValueMap.GetValue(k).c_str()) != 0)
					CPPUNIT_FAIL_T(CStrPrintf(_T("CLyrTagNameValueMap: case: %d, %d"), k, i).c_str());

				tstring strLrcTag = nameValueMap.GetLrcTagString(i, false);
				tstring	strName;
				MBCSToTString(vNames[i].strLrcName.c_str(), vNames[i].strLrcName.size(), strName);
				CStrPrintf	strLrcTag2(_T("[%s %s]"), strName.c_str(), vNames[i].pstrValue->c_str());
				if (_tcscmp(strLrcTag.c_str(), strLrcTag2.c_str()) != 0)
					CPPUNIT_FAIL_T(CStrPrintf(_T("CLyrTagNameValueMap: case: %d, %d"), k, i).c_str());

				tstring strTxtTag = nameValueMap.GetTxtTagString(i, false);
				MBCSToTString(vNames[i].strTxtName.c_str(), vNames[i].strTxtName.size(), strName);
				CStrPrintf	strTxtTag2(_T("%s %s"), strName.c_str(), vNames[i].pstrValue->c_str());
				if (_tcscmp(strTxtTag.c_str(), strTxtTag2.c_str()) != 0)
					CPPUNIT_FAIL_T(CStrPrintf(_T("CLyrTagNameValueMap: case: %d, %d"), k, i).c_str());

				bFound = true;
				break;
			}
			if (!bFound)
			{
				// NOT FOUND.
				CPPUNIT_FAIL_T(_T("CLyrTagNameValueMap: Not found"));
			}
		}

		nameValueMap.Remove(0);
		nameValueMap.Remove(nameValueMap.GetCount() - 1);
	}

	void TestCLrcTag_ReadFromText_Lrc()
	{
		// Test cases for CLrcTag.

		// Construct lyrics
#undef SZ_LYRICS
#define SZ_LYRICS	"[ti:title]\r\n\r\n[00:01.20]\r\n[encoding:gb2312]sxx\r\n[ar:artist]"

		LPCTSTR		valArtist = _T("artist");
		LPCTSTR		valTitle = _T("title");
		wstringex	wStr;
		MbcsToUCS2(SZ_LYRICS, -1, wStr);

		wstringex	wLyrics;
		wLyrics.append((const WCHAR *)SZ_FE_UCS2, 1);
		wLyrics.append(wStr.c_str(), wStr.size());

		wstringex	wLyricsBe;
		wLyricsBe.append(wLyrics.c_str(), wLyrics.size());
		UCS2LEToBE(wLyricsBe.data(), wLyricsBe.size());

		stringex	utf8Lyrics;
		utf8Lyrics.append(SZ_FE_UTF8);
		utf8Lyrics.append(SZ_LYRICS);

		LPBYTE		vLyrics[] = { (LPBYTE)SZ_LYRICS, (LPBYTE)wLyrics.c_str(),		(LPBYTE)wLyricsBe.c_str(),			(LPBYTE)utf8Lyrics.c_str(), };
		int			vLen[] = { sizeof(SZ_LYRICS),	wLyrics.size() * sizeof(WCHAR), wLyricsBe.size() * sizeof(WCHAR),	utf8Lyrics.size(), };
		CHAR_ENCODING	vEncoding[] = { ED_GB2312,	ED_UNICODE,						ED_UNICODE_BIG_ENDIAN,				ED_UTF8, };

		for (int i = 0; i < CountOf(vLyrics); i++)
		{
			CLrcTag		tag(LTT_ALL);
			CPPUNIT_ASSERT(tag.ReadFromText(vLyrics[i], vLen[i]));
			{
				CPPUNIT_ASSERT(_tcscmp(tag.m_strArtist.c_str(), valArtist) == 0);
				CPPUNIT_ASSERT(_tcscmp(tag.m_strTitle.c_str(), valTitle) == 0);
				CPPUNIT_ASSERT(tag.GetEncodingIndex() == vEncoding[i]);
				CPPUNIT_ASSERT(tag.m_lyrContentType == LCT_LRC);
			}
		}

	}

	void TestCLrcTag_WriteToText_Lrc()
	{
		// Test cases for CLrcTag.

		// Construct lyrics
#undef SZ_LYRICS
#define SZ_LYRICS	"[ti: title]\r\n\r\n[00:01.20]\r\n[encoding: gb2312]sxx\r\n[ar: artist]"

		LPCTSTR		valArtist = _T("artist");
		LPCTSTR		valTitle = _T("title");
		wstringex	wStr;
		MbcsToUCS2(SZ_LYRICS, -1, wStr);

		stringex	ansiLyrics;
		ansiLyrics.append(SZ_LYRICS);

		wstringex	wLyrics;
		wLyrics.append(wStr.c_str(), wStr.size());

		LyricsProperties	orgProp, newProp;

		orgProp.m_strArtist = _T("artist");
		orgProp.m_strTitle = _T("title");
		orgProp.SetEncoding(ED_GB2312);

		newProp.m_strArtist = _T("newAr");
		newProp.m_strTitle = _T("newTi");
		newProp.m_strId = _T("newID");
		newProp.m_strMediaLength = _T("newMeidaLen");
		newProp.SetEncoding(ED_BIG5);

		//
		// Test on ANSI lyrics
		//
		{
			// Write new
			CLrcTag		tag(orgProp, newProp);
			tag.WriteToText(ansiLyrics, true);

			// Read new
			CLrcTag		tagReader;
			CPPUNIT_ASSERT(tagReader.ReadFromText((LPBYTE)ansiLyrics.c_str(), ansiLyrics.size()));

			// Is same?
			CLrcTag		tagCmp(tagReader, newProp);
			CPPUNIT_ASSERT(tagReader.m_strArtist == newProp.m_strArtist);
			CPPUNIT_ASSERT(tagReader.m_strTitle == newProp.m_strTitle);
			CPPUNIT_ASSERT(!tagCmp.IsTagChanged());

			// Write old,
			CLrcTag		tag2(newProp, orgProp);
			tag2.WriteToText(ansiLyrics, true);

			// Is same with original?
			CPPUNIT_ASSERT(strcmp(SZ_LYRICS, ansiLyrics.c_str()) == 0);
		}

		//
		// Test on UCS2 lyrics
		//
		{
			// Write new
			CLrcTag		tag(orgProp, newProp);
			tag.WriteToText(wLyrics, true);

			// Read new
			CLrcTag		tagReader;
			CPPUNIT_ASSERT(tagReader.ReadFromText((LPBYTE)wLyrics.c_str(), wLyrics.size() * sizeof(WCHAR)));

			// Is same?
			CPPUNIT_ASSERT(tagReader.m_strArtist == newProp.m_strArtist);
			CPPUNIT_ASSERT(tagReader.m_strTitle == newProp.m_strTitle);

			// Write old,
			CLrcTag		tag2(newProp, orgProp);
			tag2.WriteToText(wLyrics, true);

			// Is same with org?
			CPPUNIT_ASSERT(wcscmp(wStr.c_str(), wLyrics.c_str()) == 0);
		}
	}

	void TestCLrcTag_WriteToFile_Lrc()
	{
		// Test cases for CLrcTag.

		// Construct lyrics
#undef SZ_LYRICS
#define SZ_LYRICS	"[ti: title]\r\n\r\n[00:01.20]\r\n[encoding: gb2312]sxx\r\n[ar: artist]"

		LPCTSTR		valArtist = _T("artist");
		LPCTSTR		valTitle = _T("title");
		wstringex	wStr;
		MbcsToUCS2(SZ_LYRICS, -1, wStr);

		stringex	ansiLyrics;
		ansiLyrics.append(SZ_LYRICS);

		wstringex	wLyrics;
		wLyrics.append((const WCHAR *)SZ_FE_UCS2, 1);
		wLyrics.append(wStr.c_str(), wStr.size());

		wstringex	wLyricsBe;
		wLyricsBe.append(wLyrics.c_str(), wLyrics.size());
		UCS2LEToBE(wLyricsBe.data(), wLyricsBe.size());

		stringex	utf8Lyrics;
		utf8Lyrics.append(SZ_FE_UTF8);
		utf8Lyrics.append(SZ_LYRICS);

		LyricsProperties	orgProp, newProp;

		orgProp.m_strArtist = _T("artist");
		orgProp.m_strTitle = _T("title");

		newProp.m_strArtist = _T("newAr");
		newProp.m_strTitle = _T("newTi");

		LPBYTE		vLyrics[] = { (LPBYTE)SZ_LYRICS, (LPBYTE)wLyrics.c_str(),		(LPBYTE)wLyricsBe.c_str(),			(LPBYTE)utf8Lyrics.c_str(), };
		int			vLen[] = { sizeof(SZ_LYRICS),	wLyrics.size() * sizeof(WCHAR), wLyricsBe.size() * sizeof(WCHAR),	utf8Lyrics.size(), };

		tstring		strFile = GetWorkingFolder();
		strFile += _T("unit_test.lrc");
		for (int i = 0; i < CountOf(vLen); i++)
		{
			CPPUNIT_ASSERT(SaveDataAsFile(strFile.c_str(), vLyrics[i], vLen[i]));

			CLrcTag		tag(orgProp, newProp);
			CPPUNIT_ASSERT(tag.WriteToFile(strFile.c_str(), true) == ERR_OK);

			CLrcTag		tagReader;
			CPPUNIT_ASSERT(tagReader.ReadFromFile(strFile.c_str()) == ERR_OK);

			CPPUNIT_ASSERT(tagReader.m_strArtist == newProp.m_strArtist);
			CPPUNIT_ASSERT(tagReader.m_strTitle == newProp.m_strTitle);

			CLrcTag		tag2(newProp, orgProp);
			CPPUNIT_ASSERT(tag2.WriteToFile(strFile.c_str(), true) == ERR_OK);

			CCharBuffer	newContent;
			CPPUNIT_ASSERT(OpenFileToBuffer(strFile.c_str(), newContent));
			CPPUNIT_ASSERT(memcmp(newContent.c_str(), vLyrics[i], vLen[i]) == 0);
			CPPUNIT_ASSERT(newContent.size() == vLen[i]);
		}

		DeleteFile(strFile.c_str());
	}


	void TestCLrcTag_ReadFromText_Txt()
	{
		// Test cases for CLrcTag.

		// Construct lyrics
#undef SZ_LYRICS
#define SZ_LYRICS	"Title:title\r\n\r\nabc\r\nEncoding:gb2312\r\nArtist:artist"

		LPCTSTR		valArtist = _T("artist");
		LPCTSTR		valTitle = _T("title");
		wstringex	wStr;
		MbcsToUCS2(SZ_LYRICS, -1, wStr);

		wstringex	wLyrics;
		wLyrics.append((const WCHAR *)SZ_FE_UCS2, 1);
		wLyrics.append(wStr.c_str(), wStr.size());

		wstringex	wLyricsBe;
		wLyricsBe.append(wLyrics.c_str(), wLyrics.size());
		UCS2LEToBE(wLyricsBe.data(), wLyricsBe.size());

		stringex	utf8Lyrics;
		utf8Lyrics.append(SZ_FE_UTF8);
		utf8Lyrics.append(SZ_LYRICS);

		LPBYTE		vLyrics[] = { (LPBYTE)SZ_LYRICS, (LPBYTE)wLyrics.c_str(),		(LPBYTE)wLyricsBe.c_str(),			(LPBYTE)utf8Lyrics.c_str(), };
		int			vLen[] = { sizeof(SZ_LYRICS),	wLyrics.size() * sizeof(WCHAR), wLyricsBe.size() * sizeof(WCHAR),	utf8Lyrics.size(), };
		CHAR_ENCODING	vEncoding[] = { ED_GB2312,	ED_UNICODE,						ED_UNICODE_BIG_ENDIAN,				ED_UTF8, };

		for (int i = 0; i < CountOf(vLyrics); i++)
		{
			CLrcTag		tag(LTT_ALL);
			CPPUNIT_ASSERT(tag.ReadFromText(vLyrics[i], vLen[i]));
			{
				CPPUNIT_ASSERT(_tcscmp(tag.m_strArtist.c_str(), valArtist) == 0);
				CPPUNIT_ASSERT(_tcscmp(tag.m_strTitle.c_str(), valTitle) == 0);
				CPPUNIT_ASSERT(tag.GetEncodingIndex() == vEncoding[i]);
				CPPUNIT_ASSERT(tag.m_lyrContentType == LCT_TXT);
			}
		}

	}

	void TestCLrcTag_WriteToText_Txt()
	{
		// Test cases for CLrcTag.

		// Construct lyrics
#undef SZ_LYRICS
#define SZ_LYRICS	"Title: title\r\n\r\nabc\r\nEncoding: gb2312\r\nArtist: artist"

		LPCTSTR		valArtist = _T("artist");
		LPCTSTR		valTitle = _T("title");
		wstringex	wStr;
		MbcsToUCS2(SZ_LYRICS, -1, wStr);

		stringex	ansiLyrics;
		ansiLyrics.append(SZ_LYRICS);

		wstringex	wLyrics;
		wLyrics.append(wStr.c_str(), wStr.size());

		LyricsProperties	orgProp, newProp;


		orgProp.m_strArtist = _T("artist");
		orgProp.m_strTitle = _T("title");
		orgProp.SetEncoding(ED_GB2312);

		newProp.m_strArtist = _T("newAr");
		newProp.m_strTitle = _T("newTi");
		newProp.m_strId = _T("newID");
		newProp.m_strMediaLength = _T("newMeidaLen");
		newProp.SetEncoding(ED_BIG5);

		//
		// Test on ANSI lyrics
		//
		{
			// Write new
			CLrcTag		tag(orgProp, newProp);
			tag.WriteToText(ansiLyrics, false);

			// Read new
			CLrcTag		tagReader;
			CPPUNIT_ASSERT(tagReader.ReadFromText((LPBYTE)ansiLyrics.c_str(), ansiLyrics.size()));

			// Is same?
			CLrcTag		tagCmp(tagReader, newProp);
			CPPUNIT_ASSERT(tagReader.m_strArtist == newProp.m_strArtist);
			CPPUNIT_ASSERT(tagReader.m_strTitle == newProp.m_strTitle);
			CPPUNIT_ASSERT(!tagCmp.IsTagChanged());

			// Write old,
			CLrcTag		tag2(newProp, orgProp);
			tag2.WriteToText(ansiLyrics, false);

			// Is same with org?
			// CLrcTag may append \r\n at the end of lyrics, don't compare it
			CPPUNIT_ASSERT(strncmp(SZ_LYRICS, ansiLyrics.c_str(), strlen(SZ_LYRICS)) == 0);
			CPPUNIT_ASSERT(ansiLyrics.size() <= strlen(SZ_LYRICS) + 2);
		}

		//
		// Test on UCS2 lyrics
		//
		{
			// Write new
			CLrcTag		tag(orgProp, newProp);
			tag.WriteToText(wLyrics, false);

			// Read new
			CLrcTag		tagReader;
			CPPUNIT_ASSERT(tagReader.ReadFromText((LPBYTE)wLyrics.c_str(), wLyrics.size() * sizeof(WCHAR)));

			// Is same?
			CPPUNIT_ASSERT(tagReader.m_strArtist == newProp.m_strArtist);
			CPPUNIT_ASSERT(tagReader.m_strTitle == newProp.m_strTitle);

			// Write old,
			CLrcTag		tag2(newProp, orgProp);
			tag2.WriteToText(wLyrics, false);

			// Is same with org?
			// CLrcTag may append \r\n at the end of lyrics, don't compare it
			CPPUNIT_ASSERT(wcsncmp(wStr.c_str(), wLyrics.c_str(), wStr.size()) == 0);
			CPPUNIT_ASSERT(wLyrics.size() <= wStr.size() + 2);
		}
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseLrcTag);

#endif // _CPPUNIT_TEST
