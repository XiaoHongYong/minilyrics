#pragma once

DECLAR_CPPUNIT_TEST_REG(CharEncodingBase)

enum CHAR_ENCODING
{
	ED_SYSDEF,					// ϵͳȱʡ�ı���
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

struct ENCODING_CODEPAGE
{
	CHAR_ENCODING		encodingID;

#ifdef _MAC
	int					nCFStringEncoding;
#endif

#ifdef _WIN32
	int					nCodePage;		// windows codepage
	int					nFontCharset;
#endif

	LPCSTR				szEncoding;
	LPCSTR				szAlias;
	LPCSTR				szDesc;

#ifdef _LINUX
	LPCSTR				szIConvCode;
#endif

#ifdef _ANDROID
	LPCSTR				szIConvCode;
#endif

};

int GetCharEncodingCount();

FILE_ENCODING GetFileEncodingByBom(LPBYTE data);

CHAR_ENCODING GetCharEncodingID(const char *szEncoding);
CHAR_ENCODING GetCharEncodingID(const WCHAR *szEncoding);

ENCODING_CODEPAGE &GetSysDefaultCharEncoding();

ENCODING_CODEPAGE &GetCharEncodingByID(CHAR_ENCODING encoding);

//////////////////////////////////////////////////////////////////////////

FILE_ENCODING CharEncodingToFileEncoding(CHAR_ENCODING encoding);

CHAR_ENCODING FileEcodingToCharEncoding(FILE_ENCODING fileEncoding);

// Is the string only including ANSI characters.
bool IsAnsiStr(const WCHAR *szStr);
bool IsAnsiStr(const char *szStr);

//////////////////////////////////////////////////////////////////////////

int UCS2ToUtf8(const WCHAR *str, int nLen, char *strOut, int nOut);
int UCS2ToMbcs(const WCHAR *str, int nLen, char *strOut, int nOut, int encodingID = ED_SYSDEF);
int Utf8ToUCS2(const char *str, int nLen, WCHAR *strOut, int nOut);
int Utf8ToMbcs(const char *str, int nLen, char *strOut, int nOut, int encodingID = ED_SYSDEF);
int MbcsToUtf8(const char *str, int nLen, char *strOut, int nOut, int encodingID = ED_SYSDEF);
int MbcsToUCS2(const char *str, int nLen, WCHAR *strOut, int nOut, int encodingID = ED_SYSDEF);
int AnsiToUCS2(const char *str, int nLen, WCHAR *strOut, int nOut);
int UCS2ToAnsi(const WCHAR *str, int nLen, char *strOut, int nOut);

void UCS2ToUtf8(const WCHAR *str, int nLen, string &strOut);
void UCS2ToMbcs(const WCHAR *str, int nLen, string &strOut, int encodingID = ED_SYSDEF);
void MbcsToUtf8(const char *str, int nLen, string &strOut, int encodingID = ED_SYSDEF);
void MbcsToUCS2(const char *str, int nLen, wstring_t &strOut, int encodingID = ED_SYSDEF);
void Utf8ToMbcs(const char *str, int nLen, string &strOut, int encodingID = ED_SYSDEF);
void Utf8ToUCS2(const char *str, int nLen, wstring_t &strOut);
void AnsiToUCS2(const char *str, int nLen, wstring_t &strOut);
void UCS2ToAnsi(const WCHAR *str, int nLen, string &strOut);

void UCS2ToUtf8(const WCHAR *str, int nLen, stringex &strOut);
void UCS2ToMbcs(const WCHAR *str, int nLen, stringex &strOut, int encodingID = ED_SYSDEF);
void Utf8ToUCS2(const char *str, int nLen, wstringex &strOut);
void Utf8ToMbcs(const char *str, int nLen, stringex &strOut, int encodingID = ED_SYSDEF);
void MbcsToUtf8(const char *str, int nLen, stringex &strOut, int encodingID = ED_SYSDEF);
void MbcsToUCS2(const char *str, int nLen, wstringex &strOut, int encodingID = ED_SYSDEF);
void AnsiToUCS2(const char *str, int nLen, wstringex &strOut);
void UCS2ToAnsi(const WCHAR *str, int nLen, stringex &strOut);

void UCS2BEToLE(WCHAR *str, int nLen);
void UCS2LEToBE(WCHAR *str, int nLen);

void ConvertStr(const char *str, int nLen, wstringex &strOut, int encodingID = ED_SYSDEF);
void ConvertStr(const char *str, int nLen, stringex &strOut, int encodingID = ED_SYSDEF);
void ConvertStr(const WCHAR *str, int nLen, wstringex &strOut, int encodingID = ED_SYSDEF);
void ConvertStr(const WCHAR *str, int nLen, stringex &strOut, int encodingID = ED_SYSDEF);

void ConvertStr(const char *str, int nLen, wstring_t &strOut, int encodingID = ED_SYSDEF);
void ConvertStr(const char *str, int nLen, string &strOut, int encodingID = ED_SYSDEF);
void ConvertStr(const WCHAR *str, int nLen, wstring_t &strOut, int encodingID = ED_SYSDEF);
void ConvertStr(const WCHAR *str, int nLen, string &strOut, int encodingID = ED_SYSDEF);

void ConvertStr2(const char *str, int nLen, WCHAR *strOut, int nOut, int encodingID = ED_SYSDEF);
void ConvertStr2(const char *str, int nLen, char *strOut, int nOut, int encodingID = ED_SYSDEF);
void ConvertStr2(const WCHAR *str, int nLen, WCHAR *strOut, int nOut, int encodingID = ED_SYSDEF);
void ConvertStr2(const WCHAR *str, int nLen, char *strOut, int nOut, int encodingID = ED_SYSDEF);


//
// Current string encoding is one of the following types:
// 1) UNICODE: if UNICODE macro is defined
// 2) UTF8:    if UTF8 macro is defined
// 3) MBCS:    if the above two macros are not defined, and it is default.
//

template<class _tstring>
inline void UCS2ToTString(const WCHAR *str, int nLen, _tstring &strOut, int encodingID = ED_SYSDEF)
{
#ifdef UNICODE
	if (nLen == -1)
		strOut = str;
	else
	{
		strOut.clear();
		strOut.append(str, nLen);
	}
#elif defined(UTF8)
	UCS2ToUtf8(str, nLen, strOut);
#else
	UCS2ToMbcs(str, nLen, strOut, encodingID);
#endif
}

template<class _tstring>
inline void UTF8ToTString(const char *str, int nLen, _tstring &strOut, int encodingID = ED_SYSDEF)
{
#ifdef UNICODE
	Utf8ToUCS2(str, nLen, strOut);
#elif defined(UTF8)
	if (nLen == -1)
		strOut = str;
	else
	{
		strOut.clear();
		strOut.append(str, nLen);
	}
#else
	Utf8ToMbcs(str, nLen, strOut, encodingID);
#endif
}

template<class _tstring>
inline void MBCSToTString(const char *str, int nLen, _tstring &strOut, int encodingID = ED_SYSDEF)
{
#ifdef UNICODE
	MbcsToUCS2(str, nLen, strOut, encodingID);
#elif defined(UTF8)
	MbcsToUtf8(str, nLen, strOut, encodingID);
#else
	if (nLen == -1)
		strOut = str;
	else
	{
		strOut.clear();
		strOut.append(str, nLen);
	}
#endif
}

template<class _tstring>
inline void AnsiToTString(const char *str, int nLen, _tstring &strOut)
{
#ifdef UNICODE
	AnsiToUCS2(str, nLen, strOut);
#else
	if (nLen == -1)
		strOut = str;
	else
	{
		strOut.clear();
		strOut.append(str, nLen);
	}
#endif
}

template<class _wstring>
inline void TStringToUCS2(const TCHAR *str, int nLen, _wstring &strOut, int encodingID = ED_SYSDEF)
{
#ifdef UNICODE
	if (nLen == -1)
		strOut = str;
	else
	{
		strOut.clear();
		strOut.append(str, nLen);
	}
#elif defined(UTF8)
	Utf8ToUCS2(str, nLen, strOut);
#else
	MbcsToUCS2(str, nLen, strOut, encodingID);
#endif
}

template<class _string>
inline void TStringToUTF8(const TCHAR *str, int nLen, _string &strOut, int encodingID = ED_SYSDEF)
{
#ifdef UNICODE
	UCS2ToUtf8(str, nLen, strOut);
#elif defined(UTF8)
	if (nLen == -1)
		strOut = str;
	else
	{
		strOut.clear();
		strOut.append(str, nLen);
	}
#else
	MbcsToUtf8(str, nLen, strOut, encodingID);
#endif
}

template<class _string>
inline void TStringToMBCS(const TCHAR *str, int nLen, _string &strOut, int encodingID = ED_SYSDEF)
{
#ifdef UNICODE
	UCS2ToMbcs(str, nLen, strOut, encodingID);
#elif defined(UTF8)
	Utf8ToMbcs(str, nLen, strOut, encodingID);
#else
	if (nLen == -1)
		strOut = str;
	else
	{
		strOut.clear();
		strOut.append(str, nLen);
	}
#endif
}

template<class _string>
inline void TStringToAnsi(const TCHAR *str, int nLen, _string &strOut)
{
#ifdef UNICODE
	UCS2ToAnsi(str, nLen, strOut);
#else
	if (nLen == -1)
		strOut = str;
	else
	{
		strOut.clear();
		strOut.append(str, nLen);
	}
#endif
}

