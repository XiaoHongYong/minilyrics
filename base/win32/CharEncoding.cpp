// CharEncoding.cpp: implementation of the CCharEncoding class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "stringex_t.h"
#include "CharEncoding.h"
#include "stringex.h"
#include "safestr.h"
#include "xstr.h"

#ifndef HANGUL_CHARSET
#define HANGUL_CHARSET 129
#endif


CHAR_ENCODING FileEcodingToCharEncoding(FILE_ENCODING fileEncoding)
{
	switch (fileEncoding)
	{
	case FE_UCS2:
		return ED_UNICODE;
	case FE_UCS2_BE:
		return ED_UNICODE_BIG_ENDIAN;
	case FE_UTF8:
		return ED_UTF8;
	case FE_ANSI:
		return ED_SYSDEF;
	default:
		ASSERT(0);
		return ED_SYSDEF;
	}
}

FILE_ENCODING CharEncodingToFileEncoding(CHAR_ENCODING encoding)
{
	switch (encoding)
	{
	case ED_UNICODE:
		return FE_UCS2;
	case ED_UNICODE_BIG_ENDIAN:
		return FE_UCS2_BE;
	case ED_UTF8:
		return FE_UTF8;
	default:
		return FE_ANSI;
	}
}


//
// ED_XXX 的定义和 __encodingCodepage 的索引顺序是一直的。
// 即__encodingCodepage[ED_XXX].nEncodingId = ED_XXX
ENCODING_CODEPAGE	__encodingCodepage[] = 
{
	{ ED_SYSDEF, CP_ACP, DEFAULT_CHARSET, "", "", "systemdefault" },
	{ ED_UNICODE, 0, DEFAULT_CHARSET, "unicode", "", "Unicode" },
	{ ED_UNICODE_BIG_ENDIAN, 0, DEFAULT_CHARSET, "Unicode (Big-Endian)", "", "Unicode (Big-Endian)" },
	{ ED_UTF8, CP_UTF8, DEFAULT_CHARSET, "utf-8", "", "Unicode (UTF-8)" },
	{ ED_ARABIC, 1256, ARABIC_CHARSET, "windows-1256", "arabic", "Arabic" },
	//	{ ED_BALTIC_ISO, 28594, DEFAULT_CHARSET, "iso-8859-4", "", "Baltic (ISO)" },
	{ ED_BALTIC_WINDOWS, 1257, BALTIC_CHARSET, "windows-1257", "baltic", "Baltic (Windows)" },
	//	{ ED_CENTRAL_EUROPEAN_ISO, 28592, DEFAULT_CHARSET, "iso-8859-2", "", "Central European (ISO)" },
	{ ED_CENTRAL_EUROPEAN_WINDOWS, 1250, DEFAULT_CHARSET, "windows-1250", "", "Central European (Windows)" },
	{ ED_GB2312, 936, GB2312_CHARSET, "gb2312", "gb2312", "Chinese Simplified (GB2312)" },
	{ ED_BIG5, 950, CHINESEBIG5_CHARSET, "big5", "big5", "Chinese Traditional (Big5)" },
	//	{ ED_CYRILLIC, 28595, DEFAULT_CHARSET, "iso-8859-5", "", "Cyrillic (ISO)" },
	{ ED_CYRILLIC_WINDOWS, 1251, DEFAULT_CHARSET, "windows-1251", "", "Cyrillic (Windows)" },
	//	{ ED_GREEK_ISO, 28597, DEFAULT_CHARSET, "iso-8859-7", "", "Greek (ISO)" },
	{ ED_GREEK_WINDOWS, 1253, GREEK_CHARSET, "windows-1253", "greek", "Greek (Windows)" },
	{ ED_HEBREW_WINDOWS, 1255, HEBREW_CHARSET, "windows-1255", "hebrew", "Hebrew (Windows)" },
	{ ED_JAPANESE_SHIFT_JIS, 932, SHIFTJIS_CHARSET, "shift_jis", "shiftjis", "Japanese (Shift-JIS)" },
	{ ED_KOREAN, 949, HANGUL_CHARSET, "ks_c_5601-1987", "hangeul", "Korean" },
	{ ED_LATIN9_ISO, 1252, DEFAULT_CHARSET, "iso-8859-15", "", "Latin 9 (ISO)" },
	{ ED_THAI, 874, THAI_CHARSET, "windows-874", "Thai", "Thai (Windows)" },
	//	{ ED_TURKISH_ISO, 28599, DEFAULT_CHARSET, "iso-8859-9", 0, "Turkish (ISO)" },
	{ ED_TURKISH_WINDOWS, 1254, TURKISH_CHARSET, "windows-1254", "turkish", "Turkish (Windows)" },
	{ ED_VIETNAMESE, 1258, VIETNAMESE_CHARSET, "windows-1258", "vietnamese", "Vietnamese (Windows)" },
	//	{ ED_WESTERN_EUROPEAN_ISO, 28591, DEFAULT_CHARSET, "iso-8859-1", "", "Western European (ISO)" },
	{ ED_WESTERN_EUROPEAN_WINDOWS, 1252, DEFAULT_CHARSET, "Windows-1252", "", "Western European (Windows)" },
	{ ED_EASTERN_EUROPEAN_WINDOWS, 1250, EASTEUROPE_CHARSET, "Windows-1250", "easteurope", "Eastern European (Windows)" },
	{ ED_RUSSIAN_WINDOWS, 1251, RUSSIAN_CHARSET, "Windows-1251", "russian", "Russian (Windows)" },
};

#define 		__MaxEncodings		CountOf(__encodingCodepage)


CHAR_ENCODING GetCharEncodingID(const char *szEncoding)
{
	assert(ED_END + 1 == CountOf(__encodingCodepage));
	for (int i = 0; i < __MaxEncodings; i++)
	{
		if (_stricmp(szEncoding, __encodingCodepage[i].szEncoding) == 0)
			return __encodingCodepage[i].nEncodingId;
		if (_stricmp(szEncoding, __encodingCodepage[i].szAlias) == 0)
			return __encodingCodepage[i].nEncodingId;
	}

	return ED_SYSDEF;
}

CHAR_ENCODING GetCharEncodingID(const WCHAR *szEncoding)
{
	for (int i = 0; i < __MaxEncodings; i++)
	{
		if (_xcsicmp(szEncoding, __encodingCodepage[i].szEncoding) == 0)
			return __encodingCodepage[i].nEncodingId;
		if (_xcsicmp(szEncoding, __encodingCodepage[i].szAlias) == 0)
			return __encodingCodepage[i].nEncodingId;
		if (_xcsicmp(szEncoding, __encodingCodepage[i].szDesc) == 0)
			return __encodingCodepage[i].nEncodingId;
	}

	return ED_SYSDEF;
}

void EnumAllEncodingsName(vector<tstring> &vEncodingNames)
{
	TCHAR		szEncoding[256];

	for (int i = 0; i < __MaxEncodings; i++)
	{
		_xcscpy(szEncoding, __encodingCodepage[i].szDesc);
		vEncodingNames.push_back(szEncoding);
	}
}

void EnumMBCSEncodingsName(vector<tstring> &vEncodingNames)
{
	TCHAR		szEncoding[256];

	for (int i = 0; i < __MaxEncodings; i++)
	{
		if (__encodingCodepage[i].nEncodingId != ED_UTF8 && 
			__encodingCodepage[i].nEncodingId != ED_UNICODE_BIG_ENDIAN && 
			__encodingCodepage[i].nEncodingId != ED_UNICODE)
		{
			_xcscpy(szEncoding, __encodingCodepage[i].szDesc);
			vEncodingNames.push_back(szEncoding);
		}
	}
}

ENCODING_CODEPAGE &GetSysDefaultCharEncoding()
{
	UINT		nCodePage;

	nCodePage = GetACP();
	for (int i = 0; i < __MaxEncodings; i++)
	{
		if (nCodePage == __encodingCodepage[i].nCodePage)
			return __encodingCodepage[i];
	}

	return __encodingCodepage[0];
}

ENCODING_CODEPAGE &GetCharEncodingByID(CHAR_ENCODING encoding)
{
	if (encoding >= __MaxEncodings || encoding < 0)
		encoding = ED_SYSDEF;

	ASSERT(__encodingCodepage[encoding].nEncodingId == encoding);
	return __encodingCodepage[encoding];
}

ENCODING_CODEPAGE &GetCharEncodingByName(LPCTSTR szEncoding)
{
	for (int i = 0; i < __MaxEncodings; i++)
	{
		if (_xcsicmp(szEncoding, __encodingCodepage[i].szDesc) == 0)
			return __encodingCodepage[i];
		if (_xcsicmp(szEncoding, __encodingCodepage[i].szEncoding) == 0)
			return __encodingCodepage[i];
		if (_xcsicmp(szEncoding, __encodingCodepage[i].szAlias) == 0)
			return __encodingCodepage[i];
	}

	return __encodingCodepage[0];
}

void ConvertStr(const WCHAR *str, int nLen, stringex &strOut, int nEncodingID)
{
	int		n;

	if (nLen == -1)
		nLen = wcslen(str);

	strOut.reserve(nLen * 3);

	if (nEncodingID >= __MaxEncodings || nEncodingID < 0)
		nEncodingID = 0;
	assert(nEncodingID == __encodingCodepage[nEncodingID].nEncodingId);

	n = WideCharToMultiByte(__encodingCodepage[nEncodingID].nCodePage, 0,
		str, nLen, strOut.data(), strOut.capacity(), NULL, NULL);

	strOut.resize(n);
}

void ConvertStr(const WCHAR *str, int nLen, wstringex &strOut, int nEncodingID)
{
	if (nLen >= 0)
		strOut.NCopy(str, str + nLen);
	else
		strOut = str;
}

void ConvertStr(const char *str, int nLen, stringex &strOut, int nEncodingID)
{
	if (nLen >= 0)
		strOut.NCopy(str, str + nLen);
	else
		strOut = str;
}

void ConvertStr(const char *str, int nLen, wstringex &strOut, int nEncodingID)
{
	int			n;

	if (nLen == -1)
		nLen = strlen(str);

	strOut.reserve(nLen);

	if (nEncodingID >= __MaxEncodings || nEncodingID < 0)
		nEncodingID = 0;
	assert(nEncodingID == __encodingCodepage[nEncodingID].nEncodingId);
	ASSERT(nEncodingID != ED_UNICODE && nEncodingID != ED_UNICODE_BIG_ENDIAN);

	n = MultiByteToWideChar(__encodingCodepage[nEncodingID].nCodePage, 0,
		str, nLen, strOut.data(), strOut.capacity());
	if (n == 0 && GetLastError() == ERROR_INVALID_PARAMETER)
	{
		n = MultiByteToWideChar(CP_ACP, 0,
			str, nLen, strOut.data(), strOut.capacity());
	}

	strOut.resize(n);
}

//////////////////////////////////////////////////////////////////////////
void ConvertStr(const WCHAR *str, int nLen, string &strOut, int nEncodingID)
{
	int		n;

	if (nLen == -1)
		nLen = wcslen(str);

	strOut.resize(nLen * 3);

	if (nEncodingID >= __MaxEncodings || nEncodingID < 0)
		nEncodingID = 0;
	assert(nEncodingID == __encodingCodepage[nEncodingID].nEncodingId);

	n = WideCharToMultiByte(__encodingCodepage[nEncodingID].nCodePage, 0,
		str, nLen, (LPSTR)strOut.c_str(), strOut.size(), NULL, NULL);

	strOut.resize(n);
}

void ConvertStr(const WCHAR *str, int nLen, wstring_t &strOut, int nEncodingID)
{
	if (nLen >= 0)
	{
		strOut.resize(0);
		strOut.append(str, nLen);
	}
	else
		strOut = str;
}

void ConvertStr(const char *str, int nLen, string &strOut, int nEncodingID)
{
	if (nLen >= 0)
	{
		strOut.resize(0);
		strOut.append(str, nLen);
	}
	else
		strOut = str;
}

void ConvertStr(const char *str, int nLen, wstring_t &strOut, int nEncodingID)
{
	int			n;

	if (nLen == -1)
		nLen = strlen(str);

	strOut.resize(nLen);

	if (nEncodingID >= __MaxEncodings || nEncodingID < 0)
		nEncodingID = 0;
	assert(nEncodingID == __encodingCodepage[nEncodingID].nEncodingId);

	n = MultiByteToWideChar(__encodingCodepage[nEncodingID].nCodePage, 0,
		str, nLen, (LPWSTR)strOut.c_str(), strOut.size());
	if (n == 0 && GetLastError() == ERROR_INVALID_PARAMETER)
	{
		n = MultiByteToWideChar(CP_ACP, 0,
			str, nLen, (LPWSTR)strOut.c_str(), strOut.size());
	}

	strOut.resize(n);
}

void ConvertStr2(const char *str, int nLen, WCHAR *strOut, int nOut, int nEncodingID)
{
	int		n;

	if (nEncodingID >= __MaxEncodings || nEncodingID < 0)
		nEncodingID = 0;
	assert(nEncodingID == __encodingCodepage[nEncodingID].nEncodingId);

	n = MultiByteToWideChar(__encodingCodepage[nEncodingID].nCodePage, 0,
		str, nLen, strOut, nOut);
	if (n == 0 && GetLastError() == ERROR_INVALID_PARAMETER)
	{
		n = MultiByteToWideChar(CP_ACP, 0,
			str, nLen, strOut, nOut);
	}

	if (n < nOut)
		strOut[n] = '\0';
}

void ConvertStr2(const char *str, int nLen, char *strOut, int nOut, int nEncodingID)
{
	strcpy_safe(strOut, nLen, str);
}

void ConvertStr2(const WCHAR *str, int nLen, WCHAR *strOut, int nOut, int nEncodingID)
{
	wcscpy_safe(strOut, nLen, str);
}

void ConvertStr2(const WCHAR *str, int nLen, char *strOut, int nOut, int nEncodingID)
{
	int			n;

	if (nEncodingID >= __MaxEncodings || nEncodingID < 0)
		nEncodingID = 0;
	assert(nEncodingID == __encodingCodepage[nEncodingID].nEncodingId);

	n = WideCharToMultiByte(__encodingCodepage[nEncodingID].nCodePage, 0,
		str, nLen, strOut, nOut, NULL, NULL);

	if (n < nOut)
		strOut[n] = '\0';
}

void UCS2ToUtf8(const WCHAR *str, int nLen, stringex &strOut)
{
	ConvertStr(str, nLen, strOut, ED_UTF8); //GetEncodingIdx("utf-8"));
}

void UCS2ToMbcs(const WCHAR *str, int nLen, stringex &strOut, int nEncodingID)
{
	ConvertStr(str, nLen, strOut, nEncodingID); //GetEncodingIdx("utf-8"));
}

void Utf8ToUCS2(const char *str, int nLen, wstringex &strOut)
{
	ConvertStr(str, nLen, strOut, ED_UTF8); // GetEncodingIdx("utf-8"));
}

void Utf8ToMbcs(const char *str, int nLen, stringex &strOut, int nEncodingID)
{
	wstringex		strW;

	Utf8ToUCS2(str, nLen, strW);

	ConvertStr(strW.c_str(), strW.size(), strOut, nEncodingID);
}

void MbcsToUtf8(const char *str, int nLen, stringex &strOut, int nEncodingID)
{
	wstringex		strW;

	ConvertStr(str, nLen, strW, nEncodingID);

	UCS2ToUtf8(strW.c_str(), strW.size(), strOut);
}

void MbcsToUCS2(const char *str, int nLen, wstringex &strOut, int nEncodingID)
{
	ConvertStr(str, nLen, strOut, nEncodingID);
}

//////////////////////////////////////////////////////////////////////////
void UCS2ToUtf8(const WCHAR *str, int nLen, string &strOut)
{
	ConvertStr(str, nLen, strOut, ED_UTF8); //GetEncodingIdx("utf-8"));
}

void UCS2ToMbcs(const WCHAR *str, int nLen, string &strOut, int nEncodingID)
{
	ConvertStr(str, nLen, strOut, nEncodingID);
}

void Utf8ToUCS2(const char *str, int nLen, wstring_t &strOut)
{
	ConvertStr(str, nLen, strOut, ED_UTF8); // GetEncodingIdx("utf-8"));
}

void Utf8ToMbcs(const char *str, int nLen, string &strOut, int nEncodingID)
{
	wstringex		strW;

	Utf8ToUCS2(str, nLen, strW);

	ConvertStr(strW.c_str(), strW.size(), strOut, nEncodingID);
}

void MbcsToUtf8(const char *str, int nLen, string &strOut, int nEncodingID)
{
	wstringex		strW;

	ConvertStr(str, nLen, strW, nEncodingID);

	UCS2ToUtf8(strW.c_str(), strW.size(), strOut);
}

void MbcsToUCS2(const char *str, int nLen, wstring &strOut, int nEncodingID)
{
	ConvertStr(str, nLen, strOut, nEncodingID);
}

//
// 判断字符串中是否包含非ANSI编码的字符
//
bool IsAnsiStr(const WCHAR *szStr)
{
	while (*szStr)
	{
		if (*szStr >= 128)
			return false;
		szStr++;
	}

	return true;
}

bool IsAnsiStr(const char *szStr)
{
	while (*szStr)
	{
		if ((unsigned char)*szStr >= 128)
			return false;
		szStr++;
	}

	return true;
}

void UCS2BEToLE(WCHAR *str, int nLen)
{
	// Swap the char of every WCHAR.
	if (nLen == -1)
		nLen = wcslen(str);

	byte	*p = (byte *)str;
	int		lenBytes = nLen * sizeof(WCHAR);
	for (int i = 0; i < lenBytes; i += 2)
	{
		byte t = p[i];
		p[i] = p[i + 1];
		p[i + 1] = t;
	}
}

void UCS2LEToBE(WCHAR *str, int nLen)
{
	UCS2BEToLE(str, nLen);
}

/*


//
// 将MultibyeChar转换未UTF-8码
int MultibyteToUTF8(LPCSTR szInput, LPSTR szOutput, int nOutputSize)
{
	WCHAR		*wchBegin;
	LPSTR		szOutStr;
	int			nOutPos = 0;
	WCHAR		wUnicode[1024];

	if (MultiByteToWideChar(CP_ACP, 0, szInput, -1,
		wUnicode, 1024) == 0)
	{
		assert(GetLastError() != ERROR_INSUFFICIENT_BUFFER);
		assert(0);
		return 0;
	}
	
	wchBegin = (WCHAR *)wUnicode;
	szOutStr = szOutput;
	while (*wchBegin && nOutPos < nOutputSize - 3)
	{
		if (*wchBegin < 0x80)
		{
			szOutStr[nOutPos] = (BYTE)*wchBegin;
			nOutPos++;
			wchBegin++;
		}
		else if (*wchBegin < 0x0800)
		{
			szOutStr[nOutPos] = (BYTE)((*wchBegin >> 6) | 0xC0);
			nOutPos++;
			szOutStr[nOutPos] = (BYTE)((*wchBegin & 0x3F) | 0x80);
			nOutPos++;
		}
		else
		{
			szOutStr[nOutPos] = (BYTE)((*wchBegin >> 12) | 0xE0);
			nOutPos++;
			szOutStr[nOutPos] = (BYTE)((*wchBegin >> 6 & 0x3F) | 0x80);
			nOutPos++;
			szOutStr[nOutPos] = (BYTE)((*wchBegin & 0x3F) | 0x80);
			nOutPos++;
			wchBegin ++;
  		}
	}
	szOutStr[nOutPos] = '\0';

	return nOutPos;
}

#define MXS(c,x,s)			(((c) - (x)) <<  (s))
#define M80S(c,s)			MXS(c,0x80,s)
#define UTF8_1_to_UCS2(in)	((WCHAR) (in)[0])
#define UTF8_2_to_UCS2(in)	((WCHAR) (MXS((in)[0],0xC0, 6) | M80S((in)[1], 0)))
#define UTF8_3_to_UCS2(in)	((WCHAR) (MXS((in)[0],0xE0,12) | M80S((in)[1], 6) | M80S((in)[2], 0) ))

int UTF8ToUCS2(unsigned char* text, int length, WCHAR *ucs2, int ubuflen)
{
	unsigned char* p = text;
	unsigned char* last = text + length;
	int		ul;
	for(p = text, ul = 0; (p < last) && (ul < ubuflen) ; )
	{
		if( (*p) < 0x80)
		{	
			ucs2[ul++] = UTF8_1_to_UCS2(p);
			p += 1;
		}
		else if( (*p) < 0xc0 )
		{	
			assert( (*p) );	// Invalid UTF8 First Byte
			ucs2[ul++] = '?';
			p += 1;
		}
		else if( (*p) < 0xe0)
		{
			ucs2[ul++] = UTF8_2_to_UCS2(p);
			p += 2;
		}
		else if( (*p) < 0xf0)
		{
			ucs2[ul++] = UTF8_3_to_UCS2(p);
			p += 3;
		}
		else if( (*p) < 0xf8)
		{
			ucs2[ul++] = '?';
			p += 4;
		}
		else if( (*p) < 0xfc)
		{
			ucs2[ul++] = '?';
			p += 5;
		}
		else if( (*p) < 0xfe)
		{
			ucs2[ul++] = '?';
			p += 6;
		}
		else
		{
			assert( (*p) );	// Invalid UTF8 First Byte
			ucs2[ul++] = '?';
			p += 1;
		}
	}
	return ul;
}

int UTF8ToMultibyte(unsigned char* text, char *szOut, int nOutBuffLen)
{
	WCHAR		wchWideChar[1024];

	UTF8ToUCS2(text, strlen((char *)text) + 1, wchWideChar, 1024);

	return WideCharToMultiByte(CP_ACP, 0, wchWideChar, -1, szOut, nOutBuffLen, NULL, NULL);
}
*/
