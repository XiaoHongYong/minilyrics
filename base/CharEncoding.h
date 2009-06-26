// CharEncoding.h: interface for the CCharEncoding class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHARENCODING_H__69A9619B_C5CC_4C25_908F_FE7099D44717__INCLUDED_)
#define AFX_CHARENCODING_H__69A9619B_C5CC_4C25_908F_FE7099D44717__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum CHAR_ENCODING
{
	ED_SYSDEF,					// 系统缺省的编码
	ED_UNICODE,					// unicode,
	ED_UNICODE_BIG_ENDIAN,		// unicode big-endian,
	ED_UTF8,					// utf-8
	ED_ARABIC,					// windows-1256
	// ED_BALTIC_ISO,				// iso-8859-4
	ED_BALTIC_WINDOWS,			// windows-1257
	// ED_CENTRAL_EUROPEAN_ISO,	// iso-8859-2
	ED_CENTRAL_EUROPEAN_WINDOWS,// windows-1250,
	ED_GB2312,					// gb2312
	ED_BIG5,					// big5
	// ED_CYRILLIC,				// iso-8859-5
	ED_CYRILLIC_WINDOWS,		// windows-1251
	// ED_GREEK_ISO,				// iso-8859-7
	ED_GREEK_WINDOWS,			// windows-1253
	ED_HEBREW_WINDOWS,			// windows-1255
	ED_JAPANESE_SHIFT_JIS,		//shift_jis
	ED_KOREAN,					// ks_c_5601-1987
	ED_LATIN9_ISO,				// iso-8859-15,
	ED_THAI,					// windows-874,
	// ED_TURKISH_ISO,				// iso-8859-9
	ED_TURKISH_WINDOWS,			// windows-1254,
	ED_VIETNAMESE,				// windows-1258
	// ED_WESTERN_EUROPEAN_ISO,	// iso-8859-1
	ED_WESTERN_EUROPEAN_WINDOWS,// Windows-1252,
	ED_EASTERN_EUROPEAN_WINDOWS,// Windows-1250,
	ED_RUSSIAN_WINDOWS,			// Windows-1251,
	ED_END = ED_RUSSIAN_WINDOWS,
};

#define SZ_UTF8					"utf-8"
#define SZ_UNICODE				"unicode"

typedef struct _ENCODING_CODEPAGE
{
	// int			nId;
	// BYTE		dwCharset;
	CHAR_ENCODING		nEncodingId;
	int					nCodePage;		// windows codepage
	int					nFontCharset;
	LPCSTR				szEncoding;
	LPCSTR				szAlias;
	LPCSTR				szDesc;
#ifdef _LINUX
	LPCSTR				szIConvCode;
#endif
#ifdef _MAC_OSX
	LPCSTR				szIConvCode;
#endif
}ENCODING_CODEPAGE;


CHAR_ENCODING GetCharEncodingID(const char *szEncoding);
CHAR_ENCODING GetCharEncodingID(const wchar_t *szEncoding);

void EnumAllEncodingsName(vector<tstring> &vEncodingNames);

void EnumMBCSEncodingsName(vector<tstring> &vEncodingNames);

ENCODING_CODEPAGE &GetSysDefaultCharEncoding();

ENCODING_CODEPAGE &GetCharEncodingByID(CHAR_ENCODING encoding);

ENCODING_CODEPAGE &GetCharEncodingByName(LPCTSTR szEncoding);

void ConvertStr(const char *str, int nLen, wstringex &strOut, int nEncodingID = ED_SYSDEF);
void ConvertStr(const char *str, int nLen, stringex &strOut, int nEncodingID = ED_SYSDEF);
void ConvertStr(const WCHAR *str, int nLen, wstringex &strOut, int nEncodingID = ED_SYSDEF);
void ConvertStr(const WCHAR *str, int nLen, stringex &strOut, int nEncodingID = ED_SYSDEF);

void ConvertStr(const char *str, int nLen, wstring &strOut, int nEncodingID = ED_SYSDEF);
void ConvertStr(const char *str, int nLen, string &strOut, int nEncodingID = ED_SYSDEF);
void ConvertStr(const WCHAR *str, int nLen, wstring &strOut, int nEncodingID = ED_SYSDEF);
void ConvertStr(const WCHAR *str, int nLen, string &strOut, int nEncodingID = ED_SYSDEF);

void ConvertStr2(const char *str, int nLen, WCHAR *strOut, int nOut, int nEncodingID = ED_SYSDEF);
void ConvertStr2(const char *str, int nLen, char *strOut, int nOut, int nEncodingID = ED_SYSDEF);
void ConvertStr2(const WCHAR *str, int nLen, WCHAR *strOut, int nOut, int nEncodingID = ED_SYSDEF);
void ConvertStr2(const WCHAR *str, int nLen, char *strOut, int nOut, int nEncodingID = ED_SYSDEF);

void UCS2ToUtf8(const WCHAR *str, int nLen, stringex &strOut);
void Utf8ToUCS2(const char *str, int nLen, wstringex &strOut);
void MbcsToUtf8(const char *str, int nLen, stringex &strOut, int nEncodingID = ED_SYSDEF);
void Utf8ToMbcs(const char *str, int nLen, stringex &strOut, int nEncodingID = ED_SYSDEF);

void UCS2ToUtf8(const WCHAR *str, int nLen, string &strOut);
void Utf8ToUCS2(const char *str, int nLen, wstring &strOut);
void MbcsToUtf8(const char *str, int nLen, string &strOut, int nEncodingID = ED_SYSDEF);
void Utf8ToMbcs(const char *str, int nLen, string &strOut, int nEncodingID = ED_SYSDEF);

// 判断字符串中是否包含非ANSI编码的字符
bool IsAnsiStr(const WCHAR *szStr);
bool IsAnsiStr(const char *szStr);

#endif // !defined(AFX_CHARENCODING_H__69A9619B_C5CC_4C25_908F_FE7099D44717__INCLUDED_)
