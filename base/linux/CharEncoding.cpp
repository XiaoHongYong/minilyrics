// CharEncoding.cpp: implementation of the CCharEncoding class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <iconv.h>
#include "stringex_t.h"
#include "CharEncoding.h"
#include "stringex.h"
#include "xstr.h"
#include "safestr.h"

#ifndef WIN32

#ifndef HANGUL_CHARSET
#define HANGUL_CHARSET 129
#endif

#define CP_ACP                    0           // default to ANSI code page
#define CP_UTF8                   65001       // UTF-8 translation

	
#define ANSI_CHARSET            0
#define DEFAULT_CHARSET         1
#define SYMBOL_CHARSET          2
#define SHIFTJIS_CHARSET        128
#define HANGEUL_CHARSET         129
#define HANGUL_CHARSET          129
#define GB2312_CHARSET          134
#define CHINESEBIG5_CHARSET     136
#define OEM_CHARSET             255
#define JOHAB_CHARSET           130
#define HEBREW_CHARSET          177
#define ARABIC_CHARSET          178
#define GREEK_CHARSET           161
#define TURKISH_CHARSET         162
#define VIETNAMESE_CHARSET      163
#define THAI_CHARSET            222
#define EASTEUROPE_CHARSET      238
#define RUSSIAN_CHARSET         204

#define MAC_CHARSET             77
#define BALTIC_CHARSET          186

#endif

#define  DBGLOG1		printf

//
// ED_XXX 的定义和 __encodingCodepage 的索引顺序是一直的。
// 即__encodingCodepage[ED_XXX].nEncodingId = ED_XXX
ENCODING_CODEPAGE	__encodingCodepage[] = 
{
	{ ED_SYSDEF, CP_ACP, DEFAULT_CHARSET, "", "", "systemdefault", "ISO-8859-15" },
	{ ED_UNICODE, 0, DEFAULT_CHARSET, "unicode", "", "Unicode", "unicode" },
	{ ED_UNICODE_BIG_ENDIAN, 0, DEFAULT_CHARSET, "Unicode (Big-Endian)", "", "Unicode (Big-Endian)", "unicode" },
	{ ED_UTF8, CP_UTF8, DEFAULT_CHARSET, "utf-8", "", "Unicode (UTF-8)", "utf-8" },
	{ ED_ARABIC, 1256, ARABIC_CHARSET, "windows-1256", "arabic", "Arabic", "windows-1256" },
	//	{ ED_BALTIC_ISO, 28594, DEFAULT_CHARSET, "iso-8859-4", "", "Baltic (ISO)" },
	{ ED_BALTIC_WINDOWS, 1257, BALTIC_CHARSET, "windows-1257", "baltic", "Baltic (Windows)", "windows-1257" },
	//	{ ED_CENTRAL_EUROPEAN_ISO, 28592, DEFAULT_CHARSET, "iso-8859-2", "", "Central European (ISO)" },
	{ ED_CENTRAL_EUROPEAN_WINDOWS, 1250, DEFAULT_CHARSET, "windows-1250", "", "Central European (Windows)", "windows-1250" },
	{ ED_GB2312, 936, GB2312_CHARSET, "gb2312", "gb2312", "Chinese Simplified (GB2312)", "gb2312" },
	{ ED_BIG5, 950, CHINESEBIG5_CHARSET, "big5", "big5", "Chinese Traditional (Big5)", "big5" },
	//	{ ED_CYRILLIC, 28595, DEFAULT_CHARSET, "iso-8859-5", "", "Cyrillic (ISO)" },
	{ ED_CYRILLIC_WINDOWS, 1251, DEFAULT_CHARSET, "windows-1251", "", "Cyrillic (Windows)", "windows-1251" },
	//	{ ED_GREEK_ISO, 28597, DEFAULT_CHARSET, "iso-8859-7", "", "Greek (ISO)" },
	{ ED_GREEK_WINDOWS, 1253, GREEK_CHARSET, "windows-1253", "greek", "Greek (Windows)", "windows-1253" },
	{ ED_HEBREW_WINDOWS, 1255, HEBREW_CHARSET, "windows-1255", "hebrew", "Hebrew (Windows)", "windows-1255" },
	{ ED_JAPANESE_SHIFT_JIS, 932, SHIFTJIS_CHARSET, "shift_jis", "shiftjis", "Japanese (Shift-JIS)", "shift_jis" },
	{ ED_KOREAN, 949, HANGUL_CHARSET, "ks_c_5601-1987", "hangeul", "Korean", "CP949" },
	{ ED_LATIN9_ISO, 1252, DEFAULT_CHARSET, "iso-8859-15", "", "Latin 9 (ISO)", "iso-8859-15" },
	{ ED_THAI, 874, THAI_CHARSET, "windows-874", "Thai", "Thai (Windows)", "CP874" },
	//	{ ED_TURKISH_ISO, 28599, DEFAULT_CHARSET, "iso-8859-9", 0, "Turkish (ISO)" },
	{ ED_TURKISH_WINDOWS, 1254, TURKISH_CHARSET, "windows-1254", "turkish", "Turkish (Windows)", "windows-1254" },
	{ ED_VIETNAMESE, 1258, VIETNAMESE_CHARSET, "windows-1258", "vietnamese", "Vietnamese (Windows)", "windows-1258" },
	//	{ ED_WESTERN_EUROPEAN_ISO, 28591, DEFAULT_CHARSET, "iso-8859-1", "", "Western European (ISO)" },
	{ ED_WESTERN_EUROPEAN_WINDOWS, 1252, DEFAULT_CHARSET, "Windows-1252", "", "Western European (Windows)", "Windows-1252" },
	{ ED_EASTERN_EUROPEAN_WINDOWS, 1250, EASTEUROPE_CHARSET, "Windows-1250", "easteurope", "Eastern European (Windows)", "Windows-1250" },
	{ ED_RUSSIAN_WINDOWS, 1251, RUSSIAN_CHARSET, "Windows-1251", "russian", "Russian (Windows)", "Windows-1251" },
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
			__encodingCodepage[i].nEncodingId != ED_UNICODE)
		{
			_xcscpy(szEncoding, __encodingCodepage[i].szDesc);
			vEncodingNames.push_back(szEncoding);
		}
	}
}

ENCODING_CODEPAGE &GetSysDefaultCharEncoding()
{
	return __encodingCodepage[0];
}

ENCODING_CODEPAGE &GetCharEncodingByID(CHAR_ENCODING encoding)
{
	if (encoding >= __MaxEncodings || encoding < 0)
		encoding = ED_SYSDEF;

	ASSERT(__encodingCodepage[encoding].nEncodingId == encoding);
	return __encodingCodepage[encoding];

//	for (int i = 0; i < __MaxEncodings; i++)
//	{
//		if (encoding == __encodingCodepage[i].nEncodingId)
//			return __encodingCodepage[i];
//	}
//
//	return __encodingCodepage[0];
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

int ConvertMultiByteToWideChar(int nEncodingID, const char *lpByteStr, int nSizeMultibyte, WCHAR *lpWideStr, int nSizeWideChar)
{
	int		n;
	iconv_t	code;
	int		outBytesLeft;

	if (nEncodingID > __MaxEncodings || nEncodingID < 0)
		nEncodingID = 0;

	assert(nEncodingID == __encodingCodepage[nEncodingID].nEncodingId);

	code = iconv_open(SZ_UNICODE, __encodingCodepage[nEncodingID].szIConvCode);
	if (code == (iconv_t)-1)
	{
		lpWideStr[0] = '\0';
		return 0;
	}

	outBytesLeft = nSizeWideChar * sizeof(WCHAR);
	n = iconv(code, (char **)&lpByteStr, (size_t*)&nSizeMultibyte, (char **)(&lpWideStr), (size_t*)&outBytesLeft);
	if (n == -1)
	{
		DBGLOG1(_T("FAILED to convert str to unicode: %s"), lpByteStr);
		n = 0;
	}
	else
	{
		n = nSizeWideChar - outBytesLeft / sizeof(WCHAR);
		ASSERT(n >= 0);
		if (n < 0)
			n = 0;
	}

	lpWideStr[n] = '\0';
	iconv_close(code);

	return n;
}

int ConvertWideCharToMultiByte(int nEncodingID, const WCHAR *lpWideStr, int nSizeWideChar, char *lpByteStr, int nSizeMultibyte)
{
	int		n;
	iconv_t	code;
	int		outBytesLeft;

	if (nEncodingID > __MaxEncodings || nEncodingID < 0)
		nEncodingID = 0;

	assert(nEncodingID == __encodingCodepage[nEncodingID].nEncodingId);

	code = iconv_open(SZ_UNICODE, __encodingCodepage[nEncodingID].szIConvCode);
	if (code == (iconv_t)-1)
	{
		lpByteStr[0] = '\0';
		return 0;
	}

	outBytesLeft = nSizeMultibyte;
	n = iconv(code, (char **)&lpWideStr, (size_t*)&nSizeWideChar, (char **)(&lpByteStr), (size_t*)&outBytesLeft);
	if (n == -1)
		n = 0;
	else
		n = nSizeMultibyte - outBytesLeft;

	lpByteStr[n] = '\0';
	iconv_close(code);
	return n;
}

int ConvertMultiByteToUtf8(int nEncodingID, const char *lpByteStr, int nSizeMultibyte, char *lpUtf8Str, int nSizeUtf8)
{
	int		n;
	iconv_t	code;
	int		outBytesLeft;

	if (nEncodingID > __MaxEncodings || nEncodingID < 0)
		nEncodingID = 0;

	assert(nEncodingID == __encodingCodepage[nEncodingID].nEncodingId);

	code = iconv_open(__encodingCodepage[nEncodingID].szIConvCode, SZ_UTF8);
	if (code == (iconv_t)-1)
	{
		lpUtf8Str[0] = '\0';
		return 0;
	}

	outBytesLeft = nSizeUtf8;
	n = iconv(code, (char **)&lpByteStr, (size_t*)&nSizeMultibyte, (char **)(&lpUtf8Str), (size_t*)&outBytesLeft);
	if (n == -1)
	{
		DBGLOG1(_T("FAILED to convert str to unicode: %s"), lpByteStr);
		n = 0;
	}
	else
	{
		n = nSizeUtf8 - outBytesLeft;
		ASSERT(n >= 0);
		if (n < 0)
			n = 0;
	}

	lpUtf8Str[n] = '\0';
	iconv_close(code);

	return n;
}

int ConvertUtf8ToMultiByte(int nEncodingID, const char *lpUtf8Str, int nSizeUtf8, char *lpMbcsStr, int nSizeMultibyte)
{
	int		n;
	iconv_t	code;
	int		outBytesLeft;

	if (nEncodingID > __MaxEncodings || nEncodingID < 0)
		nEncodingID = 0;

	assert(nEncodingID == __encodingCodepage[nEncodingID].nEncodingId);

	code = iconv_open(SZ_UTF8, __encodingCodepage[nEncodingID].szIConvCode);
	if (code == (iconv_t)-1)
	{
		lpMbcsStr[0] = '\0';
		return 0;
	}

	outBytesLeft = nSizeMultibyte;
	n = iconv(code, (char **)&lpUtf8Str, (size_t*)&nSizeUtf8, (char **)(&lpMbcsStr), (size_t*)&outBytesLeft);
	if (n == -1)
	{
		DBGLOG1(_T("FAILED to convert str to unicode: %s"), lpUtf8Str);
		n = 0;
	}
	else
	{
		n = nSizeMultibyte - outBytesLeft;
		ASSERT(n >= 0);
		if (n < 0)
			n = 0;
	}

	lpMbcsStr[n] = '\0';
	iconv_close(code);

	return n;
}

void ConvertStr(const WCHAR *str, int nLen, stringex &strOut, int nEncodingID)
{
	int		n;

	if (nLen == -1)
		nLen = wcslen(str);

	strOut.reserve(nLen * 3);

	n = ConvertWideCharToMultiByte(nEncodingID, str, nLen, strOut.data(), strOut.capacity());

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

	n = ConvertMultiByteToWideChar(nEncodingID,
		str, nLen, strOut.data(), strOut.capacity());

	strOut.resize(n);
}

//////////////////////////////////////////////////////////////////////////
void ConvertStr(const WCHAR *str, int nLen, string &strOut, int nEncodingID)
{
	int		n;

	if (nLen == -1)
		nLen = wcslen(str);

	strOut.resize(nLen * 3);

	n = ConvertWideCharToMultiByte(nEncodingID, str, nLen, (char *)strOut.c_str(), strOut.size());

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

	n = ConvertMultiByteToWideChar(nEncodingID,
		str, nLen, (WCHAR *)strOut.c_str(), strOut.size());

	strOut.resize(n);
}

void ConvertStr2(const char *str, int nLen, WCHAR *strOut, int nOut, int nEncodingID)
{
	int		n;

	n = ConvertMultiByteToWideChar(nEncodingID,
		str, nLen, strOut, nOut);

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

	n = ConvertWideCharToMultiByte(nEncodingID, str, nLen, strOut, nOut);

	strOut[n] = '\0';
}

void UCS2ToUtf8(const WCHAR *str, int nLen, stringex &strOut)
{
	ConvertStr(str, nLen, strOut, ED_UTF8); //GetEncodingID("utf-8"));
}

void UCS2ToMbcs(const WCHAR *str, int nLen, stringex &strOut, int nEncodingID)
{
	ConvertStr(str, nLen, strOut, nEncodingID); //GetEncodingIdx("utf-8"));
}

void Utf8ToUCS2(const char *str, int nLen, wstringex &strOut)
{
	ConvertStr(str, nLen, strOut, ED_UTF8); // GetEncodingID("utf-8"));
}

void Utf8ToMbcs(const char *str, int nLen, stringex &strOut, int nEncodingID)
{
	if (nLen == -1)
		nLen = strlen(str);

	strOut.reserve(nLen + 1);

	int n = ConvertUtf8ToMultiByte(nEncodingID, str, nLen, strOut.data(), strOut.capacity());

	strOut.resize(n);
}

void MbcsToUtf8(const char *str, int nLen, stringex &strOut, int nEncodingID)
{
	if (nLen == -1)
		nLen = strlen(str);

	strOut.reserve(nLen * 3 / 2 + 3);

	int n = ConvertMultiByteToUtf8(nEncodingID, str, nLen, strOut.data(), strOut.capacity());

	strOut.resize(n);
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
	if (nLen == -1)
		nLen = strlen(str);

	strOut.reserve(nLen + 1);

	int n = ConvertUtf8ToMultiByte(nEncodingID, str, nLen, (char *)strOut.c_str(), strOut.capacity());

	strOut.resize(n);
}

void MbcsToUtf8(const char *str, int nLen, string &strOut, int nEncodingID)
{
	if (nLen == -1)
		nLen = strlen(str);

	strOut.reserve(nLen * 3 / 2 + 3);

	int n = ConvertMultiByteToUtf8(nEncodingID, str, nLen, (char *)strOut.c_str(), strOut.capacity());

	strOut.resize(n);
}

void MbcsToUCS2(const char *str, int nLen, string &strOut, int nEncodingID)
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

