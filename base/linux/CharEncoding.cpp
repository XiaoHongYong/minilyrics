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

#define  DBGLOG1		printf

//
// ED_XXX 的定义和 __encodingCodepage 的索引顺序是一直的。
// 即__encodingCodepage[ED_XXX].nEncodingId = ED_XXX
ENCODING_CODEPAGE	__encodingCodepage[] = 
{
	{ ED_SYSDEF, DEFAULT_CHARSET, "", "", "systemdefault", "ISO-8859-15" },
	{ ED_UNICODE, DEFAULT_CHARSET, "unicode", "", "Unicode", "unicode" },
	{ ED_UNICODE_BIG_ENDIAN, DEFAULT_CHARSET, "Unicode (Big-Endian)", "", "Unicode (Big-Endian)", "unicode" },
	{ ED_UTF8, DEFAULT_CHARSET, "utf-8", "", "Unicode (UTF-8)", "utf-8" },
	{ ED_ARABIC, ARABIC_CHARSET, "windows-1256", "arabic", "Arabic", "windows-1256" },
	{ ED_BALTIC_WINDOWS, BALTIC_CHARSET, "windows-1257", "baltic", "Baltic (Windows)", "windows-1257" },
	{ ED_CENTRAL_EUROPEAN_WINDOWS, DEFAULT_CHARSET, "windows-1250", "", "Central European (Windows)", "windows-1250" },
	{ ED_GB2312, GB2312_CHARSET, "gb2312", "gb2312", "Chinese Simplified (GB2312)", "gb2312" },
	{ ED_BIG5, CHINESEBIG5_CHARSET, "big5", "big5", "Chinese Traditional (Big5)", "big5" },
	{ ED_CYRILLIC_WINDOWS, DEFAULT_CHARSET, "windows-1251", "", "Cyrillic (Windows)", "windows-1251" },
	{ ED_GREEK_WINDOWS, GREEK_CHARSET, "windows-1253", "greek", "Greek (Windows)", "windows-1253" },
	{ ED_HEBREW_WINDOWS, HEBREW_CHARSET, "windows-1255", "hebrew", "Hebrew (Windows)", "windows-1255" },
	{ ED_JAPANESE_SHIFT_JIS, SHIFTJIS_CHARSET, "shift_jis", "shiftjis", "Japanese (Shift-JIS)", "shift_jis" },
	{ ED_KOREAN, HANGUL_CHARSET, "ks_c_5601-1987", "hangeul", "Korean", "CP949" },
	{ ED_LATIN9_ISO, DEFAULT_CHARSET, "iso-8859-15", "", "Latin 9 (ISO)", "iso-8859-15" },
	{ ED_THAI, THAI_CHARSET, "windows-874", "Thai", "Thai (Windows)", "CP874" },
	{ ED_TURKISH_WINDOWS, TURKISH_CHARSET, "windows-1254", "turkish", "Turkish (Windows)", "windows-1254" },
	{ ED_VIETNAMESE, VIETNAMESE_CHARSET, "windows-1258", "vietnamese", "Vietnamese (Windows)", "windows-1258" },
	{ ED_WESTERN_EUROPEAN_WINDOWS, DEFAULT_CHARSET, "Windows-1252", "", "Western European (Windows)", "Windows-1252" },
	{ ED_EASTERN_EUROPEAN_WINDOWS, EASTEUROPE_CHARSET, "Windows-1250", "easteurope", "Eastern European (Windows)", "Windows-1250" },
	{ ED_RUSSIAN_WINDOWS, RUSSIAN_CHARSET, "Windows-1251", "russian", "Russian (Windows)", "Windows-1251" },
};

#define 		__MaxEncodings		CountOf(__encodingCodepage)

int GetCharEncodingCount() { return __MaxEncodings; }

ENCODING_CODEPAGE &GetSysDefaultCharEncoding()
{
	return __encodingCodepage[ED_UTF8];
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
