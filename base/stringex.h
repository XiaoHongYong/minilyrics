#pragma once

#include "StrPrintf.h"

DECLAR_CPPUNIT_TEST_REG(StringEx)

FILE_ENCODING GetFileEncodingByBom(LPCVOID lpData);
int GetFileEncodingBomSize(FILE_ENCODING fe);
LPCSTR GetFileEncodingBom(FILE_ENCODING fe);

bool StrBeginWith(LPCSTR szText, LPCSTR szBeginWith);
bool StrBeginWith(LPCWSTR szText, LPCWSTR szBeginWith);
bool StrIBeginWith(LPCSTR szText, LPCSTR szBeginWith);
bool StrIBeginWith(LPCWSTR szText, LPCWSTR szBeginWith);

bool StrEndWith(LPCTSTR szText, LPCTSTR szEndWith);
bool StrIEndWith(LPCTSTR szText, LPCTSTR szEndWith);

const TCHAR * strignore(const TCHAR *str, const TCHAR *strIgnore);

void strdel(char *szStr, char ch);
void strdel(WCHAR *szStr, WCHAR ch);

TCHAR * __cdecl strrep(TCHAR * str, TCHAR chSrc, TCHAR chDest);

void strrep(tstring &str, TCHAR chSrc, TCHAR chDest);

void strrep(tstring &str, const TCHAR *szSrc, const TCHAR *szDest);

void strrep(wstringex &str, const WCHAR *szSrc, const WCHAR *szDest);
void strrep(stringex &str, const char *szSrc, const char *szDest);

#ifndef WIN32
char *_strupr(char *str);
#endif

WCHAR * __cdecl strnstr(
        const WCHAR * str1,
		int	nLen,
        const WCHAR * str2
        );

char * __cdecl strnstr(
        const char * str1,
		int	nLen,
        const char * str2
        );

char * __cdecl stristr (
        const char * str1,
        const char * str2
        );

WCHAR * __cdecl stristr (
        const WCHAR * str1,
        const WCHAR * str2
        );

TCHAR * __cdecl stristrtill(
						const TCHAR * str1,
						const TCHAR * str2,
						const TCHAR *strTill
						);

TCHAR * __cdecl stristrtill(
						const TCHAR * str1,
						const TCHAR * str2,
						TCHAR chTill
						);

TCHAR * __cdecl strchrtill (
        const TCHAR * string,
        int ch,
		int chTill
        );

TCHAR * __cdecl stristrignore (
						const TCHAR * str1,
						const TCHAR * str2
						);

void TrimStr(tstring &str, TCHAR ch = ' ');
void TrimStr(V_Str &vStr, TCHAR ch = ' ');
void TrimStr(LPTSTR pszString, TCHAR ch = ' ');
void TrimStrRight(LPTSTR pszString, LPCTSTR pszChars = _T(" "));

void UpperString(LPTSTR szString);
void LowerString(LPTSTR szString);

tstring ToUpper(LPCTSTR szString);
tstring ToLower(LPCTSTR szString);

#define CharUpr(ch)		(('a' <= ch && ch <= 'z') ? (ch + 'A' - 'a') : ch)
#define CharLwr(ch)		(('A' <= ch && ch <= 'Z') ? (ch + 'a' - 'A') : ch)

int FilterAllHTMLTags(LPTSTR szText);

inline bool IsDigit(int ch)
{
	return ('0' <= ch) && (ch <= '9');
}

inline bool IsAlpha(int ch)
{
	return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z');
}

inline bool IsAlphaDigit(int c)
{
	return IsAlpha(c) || IsDigit(c);
}

#ifdef _MAC
inline bool __isascii(int c) { return c >= 0 && c <= 0x7F; }
#endif


BOOL IsHexChar(char ch);

int HexToInt(char chHex);
uint32 HexToInt(LPCTSTR str);

// szUrl: scheme://domain:port/path?query_string#fragment_id
// If no port is specified, -1 will be set.
bool UrlParse(LPCTSTR szUrl, tstring &scheme, tstring &domain, int &port, tstring &path);

// COMMENT:
// rfc:  http://www.faqs.org/rfcs/rfc2396.html
void UriEscape(const char *szLocal, string &strInet);
void UriEscape(const char *szLocal, char *szInet, int nLenMax);
void UriEscape(const WCHAR *szLocal, WCHAR *szInet, int nLenMax, CHAR_ENCODING encoding);
void UriEscape(const WCHAR *szLocal, wstring_t &strInet, CHAR_ENCODING encoding);

// COMMENT:
//		将%20等类似的名字转换为对应的字符
void UriUnescape(const char *szInet, string &str);
void UriUnescape(const char *szInet, char *szLocal, int nLenMax);
void UriUnescape(const WCHAR *szInet, WCHAR *szLocal, int nLenMax, CHAR_ENCODING encoding);
void UriUnescape(const WCHAR *szInet, wstring_t &str, CHAR_ENCODING encoding);

BOOL IsUnicodeStr(LPCVOID lpData);

void ConvertStrByFileEncoding(CCharBuffer &outData, LPCTSTR szText, int nLen, FILE_ENCODING fileEncoding = FE_ANSI, int nCharEncodingID = ED_SYSDEF);

BOOL SaveDataAsFile(LPCTSTR szFile, const LPCVOID lpData, int nLen);

BOOL SaveStringAsFile(LPCTSTR szFile, LPCTSTR szText, int nLen, CHAR_ENCODING encoding = ED_SYSDEF);

#ifdef WIN32
BOOL OpenFileToBuffer(LPCWSTR szFile, CCharBuffer &str);
#endif

BOOL OpenFileToBuffer(LPCSTR szFile, CCharBuffer &str);

BOOL OpenFileToTString(LPCTSTR szFile, tstringex &str, CHAR_ENCODING encoding = ED_SYSDEF);

void MultiStrToVStr(LPCTSTR szText, vector<tstring> &vStr);

bool StrSplit(LPCTSTR szStr, TCHAR chSeperator, tstring &strLeft, tstring &strRight);

int StrSplit(LPCTSTR szStr, TCHAR chSeperator, vector<tstring> &containerStr);
int StrSplit(LPCTSTR szStr, TCHAR chSeperator, SetTStrNCase &containerStr);
#define StrBreak	StrSplit

template<class _iterator>
tstring StrJoin(_iterator first, _iterator end, LPCTSTR szFormat, LPCTSTR szSeperator)
{
	CStrPrintf	strFormat;
	tstring		str;
	for (; first != end; ++first)
	{
		if (str.size())
			str += szSeperator;
		strFormat.printf(szFormat, *first);
		str += strFormat.c_str();
	}

	return str;
}

template<class _iterator>
tstring StrJoin(_iterator first, _iterator end, LPCTSTR szSeperator)
{
	CStrPrintf	strFormat;
	tstring		str;
	for (; first != end; ++first)
	{
		if (str.size())
			str += szSeperator;
		str += *first;
	}

	return str;
}


LPCTSTR StringFeedUntil(LPCTSTR begin, LPCTSTR szUtil, tstring &buf);

template<class _TCHARPTR, class _int_t>
_TCHARPTR ReadInt_t(_TCHARPTR str, _int_t &nValue)
{
	bool		bNegative;

	nValue = 0;
	if (*str == '-')
	{
		bNegative = true;
		str++;
	}
	else
		bNegative = false;
	while (IsDigit(*str))
	{
		nValue *= 10;
		nValue += *str - '0';
		str++;
	}
	if (bNegative)
		nValue = -nValue;

	return str;
}

// szValue format: %d,%d,%d,%d
template<class TCHARDEF, class _int_t>
bool Scan4IntX(TCHARDEF szValue, _int_t &n1, _int_t &n2, _int_t &n3, _int_t &n4)
{
	szValue = ReadInt_t(szValue, n1);
	if (*szValue != ',')
		return false;
	szValue++;
	while (*szValue == ' ')
		szValue++;

	szValue = ReadInt_t(szValue, n2);
	if (*szValue != ',')
		return false;
	szValue++;
	while (*szValue == ' ')
		szValue++;

	szValue = ReadInt_t(szValue, n3);
	if (*szValue != ',')
		return false;
	szValue++;
	while (*szValue == ' ')
		szValue++;

	szValue = ReadInt_t(szValue, n4);

	return true;
}

// szValue format: %d,%d
template<class TCHARDEF, class _int_t>
bool Scan2IntX(TCHARDEF szValue, _int_t &n1, _int_t &n2)
{
	szValue = ReadInt_t(szValue, n1);
	if (*szValue != ',')
		return false;
	szValue++;
	while (*szValue == ' ')
		szValue++;

	ReadInt_t(szValue, n2);

	return true;
}

int StringToInt(LPCTSTR str, int nDefault = 0);
COLORREF StringToColor(LPCTSTR szColor, COLORREF nDefault = 0);

tstring StringFromInt(int n);
tstring StringFromColor(COLORREF clr);

void StringFromColor(TCHAR szStr[], COLORREF clr);

#ifndef _WIN32
// For gcc compiler, sizeof(wchar_t) == 4, need to reimplementation wcsxxx functions.
size_t	wcslen(const WCHAR *str);

int __cdecl wcsncmp (
        const WCHAR * first,
        const WCHAR * last,
        size_t count
        );

int wcscmp (const WCHAR * src, const WCHAR * dst);

WCHAR * __cdecl wcsstr (
        const WCHAR * wcs1,
        const WCHAR * wcs2
        );

WCHAR * __cdecl wcschr (
        const WCHAR * string,
        WCHAR ch
        );

#endif


inline void EnsureInputStrLen(const WCHAR *str, int &nLen)
{
	if (nLen == -1)
		nLen = (int)wcslen(str);
}

inline void EnsureInputStrLen(const char *str, int &nLen)
{
	if (nLen == -1)
		nLen = (int)strlen(str);
}

