#include "stdafx.h"
#include "error.h"
#include "xstr.h"
#include "stringex_t.h"
#include "CharEncoding.h"
#include "stringex.h"
#include "safestr.h"

#ifdef _UNICODE
typedef	WCHAR			UTCHAR;
#else
typedef	unsigned char	UTCHAR;
#endif

int isCharWordBoundary(WCHAR c) {
	return ((c >= 0x4E00 && c <= 0x9FFF)
		|| (c >= 0x3000 && c <= 0x303F) // Chinese punctuation
		|| (c >= 0x3400 && c <= 0x4DFF)
		|| (c >= 0x20000 && c <= 0x2A6DF)
		|| (c >= 0xF900 && c <= 0xFAFF)
		|| (c >= 0xF900 && c <= 0xFAFF)
		|| (c >= 0x2F800 && c <= 0x2FA1F)
		|| (!IsAlphaDigit(c) && c != '\'' && c != '-' && c != '_'));
}

bool StrBeginWith(LPCSTR szText, LPCSTR szBeginWith)
{
	while (*szBeginWith == *szText && *szBeginWith && *szText)
	{
		szBeginWith++;
		szText++;
	}

	return *szBeginWith == '\0';
}

bool StrBeginWith(LPCWSTR szText, LPCWSTR szBeginWith)
{
	while (*szBeginWith == *szText && *szBeginWith && *szText)
	{
		szBeginWith++;
		szText++;
	}

	return *szBeginWith == '\0';
}

bool StrIBeginWith(LPCSTR szText, LPCSTR szBeginWith)
{
	while (CharLwr(*szBeginWith) == CharLwr(*szText) && *szBeginWith && *szText)
	{
		szBeginWith++;
		szText++;
	}

	return *szBeginWith == '\0';
}

bool StrIBeginWith(LPCWSTR szText, LPCWSTR szBeginWith)
{
	while (CharLwr(*szBeginWith) == CharLwr(*szText) && *szBeginWith && *szText)
	{
		szBeginWith++;
		szText++;
	}

	return *szBeginWith == '\0';
}

bool StrEndWith(LPCTSTR szText, LPCTSTR szEndWith)
{
	size_t lenText = _tcslen(szText);
	size_t lenEndWith = _tcslen(szEndWith);

	if (lenText < lenEndWith)
		return false;

	return _tcscmp(szText + lenText - lenEndWith, szEndWith) == 0;
}

bool StrIEndWith(LPCTSTR szText, LPCTSTR szEndWith)
{
	size_t lenText = _tcslen(szText);
	size_t lenEndWith = _tcslen(szEndWith);

	if (lenText < lenEndWith)
		return false;

	return _tcsicmp(szText + lenText - lenEndWith, szEndWith) == 0;
}

FILE_ENCODING GetFileEncodingByBom(LPCVOID lpData)
{
	BYTE						*szBuffer;

	szBuffer = (BYTE *)lpData;

	// FF FE
	if (szBuffer[0] == 0xFF && szBuffer[1] == 0xFE)
		return FE_UCS2;
	// FE FF
	else if (szBuffer[0] == 0xFE && szBuffer[1] == 0xFF)
		return FE_UCS2_BE;
	// EF BB BF
	else if (szBuffer[0] == 0xEF && szBuffer[1] == 0xBB && szBuffer[2] == 0xBF)
		return FE_UTF8;
	else
		return FE_ANSI;
}

int GetFileEncodingBomSize(FILE_ENCODING fe)
{
	if (fe == FE_UCS2 || fe == FE_UCS2_BE)
		return 2;
	else if (fe == FE_UTF8)
		return 3;
	else
		return 0;
}

LPCSTR GetFileEncodingBom(FILE_ENCODING fe)
{
	switch (fe)
	{
	case FE_UCS2: return SZ_FE_UCS2;
	case FE_UCS2_BE: return SZ_FE_UCS2_BE;
	case FE_UTF8: return SZ_FE_UTF8;
	default: return "";
	}
}

void strrep(tstring &str, const TCHAR *szSrc, const TCHAR *szDest)
{
	size_t		nPos;
	size_t		nLenSrc = _tcslen(szSrc);
	size_t		nLenDest = _tcslen(szDest);
	LPCTSTR	szPos;

	nPos = 0;
	while (nPos < str.size())
	{
		szPos = _tcsstr(str.c_str() + nPos, szSrc);
		if (szPos)
		{
			nPos = size_t(szPos - str.c_str());
			str.erase(str.begin() + nPos, str.begin() + nPos + nLenSrc);
			str.insert(str.begin() + nPos, szDest, szDest + nLenDest);
			nPos += nLenDest;
		}
		else
			break;
	}
}

/*
void strrep(tstringex &str, const TCHAR *szSrc, const TCHAR *szDest)
{
	int		nPos;
	int		nLenSrc = _tcslen(szSrc);
	int		nLenDest = _tcslen(szDest);
	LPCTSTR	szPos;

	nPos = 0;
	while (nPos < str.size())
	{
		szPos = _tcsstr(str.c_str() + nPos, szSrc);
		if (szPos)
		{
			nPos = int(szPos - str.c_str());
			str.replace(nPos, nLenSrc, szDest, nLenDest);
			nPos += nLenDest;
		}
		else
			break;
	}
}
*/

void strrep(stringex &str, const char *szSrc, const char *szDest)
{
	size_t		nPos;
	size_t		nLenSrc = strlen(szSrc);
	size_t		nLenDest = strlen(szDest);
	LPCSTR	szPos;

	nPos = 0;
	while (nPos < str.size())
	{
		szPos = strstr(str.c_str() + nPos, szSrc);
		if (szPos)
		{
			nPos = size_t(szPos - str.c_str());
			str.replace(nPos, nLenSrc, szDest, nLenDest);
			nPos += nLenDest;
		}
		else
			break;
	}
}

void strrep(wstringex &str, const WCHAR *szSrc, const WCHAR *szDest)
{
	size_t		nPos;
	size_t		nLenSrc = wcslen(szSrc);
	size_t		nLenDest = wcslen(szDest);
	LPCWSTR	szPos;

	nPos = 0;
	while (nPos < str.size())
	{
		szPos = wcsstr(str.c_str() + nPos, szSrc);
		if (szPos)
		{
			nPos = size_t(szPos - str.c_str());
			str.replace(nPos, nLenSrc, szDest, nLenDest);
			nPos += nLenDest;
		}
		else
			break;
	}
}

TCHAR * __cdecl strrep(TCHAR * str, TCHAR chSrc, TCHAR chDest)
{
    TCHAR *cp = (TCHAR *) str;
	while (*cp)
	{
		if (*cp == chSrc)
			*cp = chDest;
		cp++;
	}

	return str;
}

void strrep(tstring &str, TCHAR chSrc, TCHAR chDest)
{
	for (size_t i = 0; i < str.size(); i++)
	{
		if (str[i] == chSrc)
			str.replace(i, 1, 1, chDest);
	}
}

#ifndef WIN32
char *_strupr(char *str)
{
	char	*s = str;
	while (*s)
	{
		int		f = (unsigned char)(*(s));
		if ( (f >= 'a') && (f <= 'z') )
		{
			*s -= ('a' - 'A');
		}
		s++;
	}
	return str;
}
#endif

const TCHAR * strignore(const TCHAR *str, const TCHAR *strIgnore)
{
    const TCHAR *cp = (TCHAR *) str;
	while (*cp)
	{
		const TCHAR *cp2 = strIgnore;
		while (*cp2 && *cp2 != *cp)
			cp2++;
		if (*cp2 == '\0')
			return cp;
		cp++;
	}

	return cp;
}

void strdel(tstring &str, TCHAR ch)
{
	for (uint i = 0; i < str.size(); )
	{
		if (str[i] == ch)
			str.erase(i, 1);
		else
			i++;
	}
}

void strdel(char *szStr, char ch)
{
	char	*szPos;
	char	*szPos2;
	szPos = szStr;
	szPos2 = szStr;
	while (*szPos)
	{
		if (*szPos != ch)
		{
			*szPos2 = *szPos;
			szPos2++;
		}
		szPos++;
	}
	*szPos2 = '\0';
}

void strdel(WCHAR *szStr, WCHAR ch)
{
	WCHAR	*szPos;
	WCHAR	*szPos2;
	szPos = szStr;
	szPos2 = szStr;
	while (*szPos)
	{
		if (*szPos != ch)
		{
			*szPos2 = *szPos;
			szPos2++;
		}
		szPos++;
	}
	*szPos2 = '\0';
}


TCHAR * __cdecl stristrignore (
						const TCHAR * str1,
						const TCHAR * str2
						)
{
	TCHAR *cp = (TCHAR *) str1;
	TCHAR *s1, *s2;
	int f,l;
	
	if ( !*str2 )
		return((TCHAR *)str1);
	
	while (*cp)
	{
		s1 = cp;
		s2 = (TCHAR *) str2;
		
		while ( *s1 && *s2)
		{
			while (*s1 == '\r' || *s1 == '\n' || *s1 == ' ')
				s1++;
			while (*s2 == '\r' || *s2 == '\n' || *s2 == ' ')
				s2++;

			if ( ((f = (UTCHAR)(*(s1))) >= 'A') && (f <= 'Z') )
				f -= ('A' - 'a');
			
			if ( ((l = (UTCHAR)(*(s2))) >= 'A') && (l <= 'Z') )
				l -= ('A' - 'a');
			if (f == l)
				s1++, s2++;
			else
				break;
		}
		
		if (!*s2)
			return(cp);
		
		cp++;
	}
	
	return(NULL);
}


TCHAR * __cdecl strchrtill (
        const TCHAR * string,
        int ch,
		int chTill
        )
{
        while (*string && *string != (TCHAR)ch && *string != (TCHAR)chTill)
                string++;

		if (*string == (TCHAR)chTill)
			return NULL;

        if (*string == (TCHAR)ch)
                return((TCHAR *)string);
        return(NULL);
}

//
// search for string2 in string1 till strTill
TCHAR * __cdecl stristrtill(
						const TCHAR * str1,
						const TCHAR * str2,
						const TCHAR *strTill
						)
{
	TCHAR *cp = (TCHAR *) str1;
	TCHAR *s1, *s2, *s3;
	int f,l,m;
	
	if ( !*str2 )
		return((TCHAR *)str1);
	
	while (*cp)
	{
		s1 = cp;
		s2 = (TCHAR *) str2;
		s3 = (TCHAR *) strTill;
		
		while ( *s1 && *s2)
		{
			if ( ((f = (UTCHAR)(*(s1))) >= 'A') && (f <= 'Z') )
				f -= ('A' - 'a');
			
			if ( ((m = (UTCHAR)(*(s3))) >= 'A') && (m <= 'Z') )
				m -= ('A' - 'a');

			if (f == m)
				s1++, s3++;
			else
				s3 = (TCHAR *) strTill;

			if (!*s3)	// to till
				return NULL;
			
			if ( ((l = (UTCHAR)(*(s2))) >= 'A') && (l <= 'Z') )
				l -= ('A' - 'a');

			if (f == l)
				s1++, s2++;
			else
				break;
		}
		
		if (!*s2)
			return(cp);
		
		cp++;
	}
	
	return(NULL);
}

//
// search for string2 in string1 till chTill
TCHAR * __cdecl stristrtill(
						const TCHAR * str1,
						const TCHAR * str2,
						TCHAR chTill
						)
{
	TCHAR *cp = (TCHAR *) str1;
	TCHAR *s1, *s2;
	int f,l;
	
	if ( !*str2 )
		return((TCHAR *)str1);

	if (( chTill >= 'A') && (chTill <= 'Z') )
		chTill -= ('A' - 'a');
	
	while (*cp)
	{
		s1 = cp;
		s2 = (TCHAR *) str2;
		
		while ( *s1 && *s2)
		{
			if ( ((f = (UTCHAR)(*(s1))) >= 'A') && (f <= 'Z') )
				f -= ('A' - 'a');
			
			if ( ((l = (UTCHAR)(*(s2))) >= 'A') && (l <= 'Z') )
				l -= ('A' - 'a');

			if (f == l)
				s1++, s2++;
			else
				break;

			if (f == chTill && *s2)	// to till
				return NULL;
		}
		
		if (!*s2)
			return(cp);
		
		cp++;
	}
	
	return(NULL);
}

//////////////////////////////////////////////////
// functions

/***
*TCHAR *stristr(string1, string2) - search for string2 in string1
*
*Purpose:
*       finds the first occurrence of string2 in string1
*
*Entry:
*       TCHAR *string1 - string to search in
*       TCHAR *string2 - string to search for
*
*Exit:
*       returns a pointer to the first occurrence of string2 in
*       string1, or NULL if string2 does not occur in string1
*******************************************************************************/

char * __cdecl stristr (
        const char * str1,
        const char * str2
        )
{
        char *cp = (char *) str1;
        char *s1, *s2;
		int f,l;

        if ( !*str2 )
            return((char *)str1);

        while (*cp)
        {
                s1 = cp;
                s2 = (char *) str2;

                while ( *s1 && *s2)
				{
					if ( ((f = (unsigned char)(*(s1))) >= 'A') && (f <= 'Z') )
						f -= ('A' - 'a');

					if ( ((l = (unsigned char)(*(s2))) >= 'A') && (l <= 'Z') )
						l -= ('A' - 'a');
					if (f == l)
                        s1++, s2++;
					else
						break;
				}

                if (!*s2)
                        return(cp);

                cp++;
        }

        return(NULL);
}

WCHAR * __cdecl stristr (
        const WCHAR * str1,
        const WCHAR * str2
        )
{
        WCHAR *cp = (WCHAR *) str1;
        WCHAR *s1, *s2;
		int f,l;

        if ( !*str2 )
            return((WCHAR *)str1);

        while (*cp)
        {
                s1 = cp;
                s2 = (WCHAR *) str2;

                while ( *s1 && *s2)
				{
					if ( ((f = (WCHAR)(*(s1))) >= 'A') && (f <= 'Z') )
						f -= ('A' - 'a');

					if ( ((l = (WCHAR)(*(s2))) >= 'A') && (l <= 'Z') )
						l -= ('A' - 'a');
					if (f == l)
                        s1++, s2++;
					else
						break;
				}

                if (!*s2)
                        return(cp);

                cp++;
        }

        return(NULL);
}


/***
*char *strnstr(string1, string2) - search for string2 in string1
*
*Purpose:
*       finds the first occurrence of string2 in string1
*
*Entry:
*       char *string1 - string to search in
*		int nLen	  - length of string1
*       char *string2 - string to search for
*
*Exit:
*       returns a pointer to the first occurrence of string2 in
*       string1, or NULL if string2 does not occur in string1
*
*Uses:
*
*Exceptions:
*
*******************************************************************************/
char * __cdecl strnstr(
        const char * str1,
		int	nLen,
        const char * str2
        )
{
	char *cp = (char *) str1;
	char *s1, *s2;

	if ( !*str2 )
		return((char *)str1);

	while (nLen > 0)
	{
		s1 = cp;
		s2 = (char *) str2;

		// while ( *s1 && *s2 && !(*s1-*s2) )
		while (*s1 && *s1 == *s2)
			s1++, s2++;

		if (!*s2)
			return(cp);

		cp++;
		nLen --;
	}
	
	return(NULL);
}

/***
*WCHAR *strstr(string1, string2) - search for string2 in string1
*
*Purpose:
*       finds the first occurrence of string2 in string1
*
*Entry:
*       WCHAR *string1 - string to search in
*		int nLen	  - length of string1
*       WCHAR *string2 - string to search for
*
*Exit:
*       returns a pointer to the first occurrence of string2 in
*       string1, or NULL if string2 does not occur in string1
*
*Uses:
*
*Exceptions:
*
*******************************************************************************/
WCHAR * __cdecl strnstr(
        const WCHAR * str1,
		int	nLen,
        const WCHAR * str2
        )
{
	WCHAR *cp = (WCHAR *) str1;
	WCHAR *s1, *s2;
	
	if ( !*str2 )
		return((WCHAR *)str1);
	
	while (nLen > 0)
	{
		s1 = cp;
		s2 = (WCHAR *) str2;
		
		// while ( *s1 && *s2 && !(*s1-*s2) )
		while (*s1 && *s1 == *s2)
			s1++, s2++;
		
		if (!*s2)
			return(cp);
		
		cp++;
		nLen --;
	}

	return(NULL);
}

void TrimStr(tstring &str, LPCTSTR szChars)
{
	tstring::iterator	it;
	long		i;

	for (i = str.size() - 1; i >= 0; i--)
	{
		if (!_tcschr(szChars, str[i]))
			break;
	}
	if (i != str.size() - 1)
		str.erase(str.begin() + 1 + i, str.end());

	for (it = str.begin(); it != str.end(); it++)
	{
		if (!_tcschr(szChars, *it))
			break;
	}
	if (it != str.begin())
		str.erase(str.begin(), it);
}

void TrimStr(tstring &str, TCHAR ch)
{
	tstring::iterator	it;
	long		i;

	for (i = str.size() - 1; i >= 0; i--)
	{
		if (str[i] != ch)
			break;
	}
	if (i != str.size() - 1)
		str.erase(str.begin() + 1 + i, str.end());

	for (it = str.begin(); it != str.end(); it++)
	{
		if (*it != ch)
			break;
	}
	if (it != str.begin())
		str.erase(str.begin(), it);
}

void TrimStr(V_Str &vStr, TCHAR ch)
{
	for (size_t i = 0; i < vStr.size(); i++)
		TrimStr(vStr[i], ch);
}

//
// 去掉字符串两端的空格
void TrimStr(LPTSTR szString, TCHAR ch)
{
	LPTSTR	szBeg;
	szBeg = szString;

	while (*szBeg == ch)
		szBeg++;

	long nLen = _tcslen(szBeg);
	if (nLen > 0)
	{
		while (szBeg[nLen - 1] == ch)
			nLen--;
		szBeg[nLen] = '\0';
	}

	if (szBeg != szString)
		memmove(szString, szBeg, sizeof(TCHAR) * (nLen + 1));
}

void TrimStrRight(LPTSTR pszStr, LPCTSTR pszChars/* = _T(" ")*/)
{
	if (NULL == pszStr || NULL == pszChars)
		return;

	LPTSTR p = pszStr + _tcslen(pszStr) - 1;
	while (p >= pszStr)
	{
		LPCTSTR pCh = pszChars;

		// Matching taget char set.
		while (*pCh)
		{
			if (*pCh == *p)
			{
				*p = 0;
				break;
			}
			++pCh;
		}

		// Not matched?
		if (*p != 0)
			return;
		--p;
	}
}

// Make string upper
void UpperString(LPTSTR szString)
{
	for (int i = 0; szString[i] != '\0'; i ++)
		if ('a' <= szString[i]  && szString[i] <= 'z')
			szString[i] += 'A' - 'a';
}

// make string lower
void LowerString(LPWSTR szString)
{
	for (int i = 0; szString[i] != '\0'; i ++)
		if ('A' <= szString[i]  && szString[i] <= 'Z')
			szString[i] += 'a' - 'A';
}

// make string lower
void LowerString(LPSTR szString)
{
	for (int i = 0; szString[i] != '\0'; i ++)
		if ('A' <= szString[i]  && szString[i] <= 'Z')
			szString[i] += 'a' - 'A';
}

tstring ToUpper(LPCTSTR szString)
{
	tstring		str = szString;

	for (int i = 0; szString[i] != '\0'; i ++)
		if ('a' <= szString[i]  && szString[i] <= 'z')
			str[i] += 'A' - 'a';

	return str;
}

tstring ToLower(LPCTSTR szString)
{
	tstring		str = szString;

	str = szString;
	for (int i = 0; szString[i] != '\0'; i ++)
		if ('A' <= szString[i]  && szString[i] <= 'Z')
			str[i] += 'a' - 'A';

	return str;
}

// Return true if:
// * Has at least one number.
// * No other chars.
bool IsStrNumeric(LPCSTR szString)
{
	LPCTSTR		szTemp;
	szTemp = szString;

	while (IsDigit(*szTemp))
		szTemp++;
	
	if (*szTemp != '\0')
		return FALSE;

	// Empty String
	if (szString == szTemp)
		return FALSE;

	return TRUE;
}

// Return true if:
// * Has at least one number.
// * No other chars.
bool IsStrNumeric(LPCWSTR szString)
{
	LPCWSTR		szTemp;
	szTemp = szString;
	
	while (IsDigit(*szTemp))
		szTemp++;
	
	if (*szTemp != '\0')
		return FALSE;
	
	// Empty String
	if (szString == szTemp)
		return FALSE;
	
	return TRUE;
}

bool IsHexChar(int ch)
{
	return (IsDigit(ch) || 
		(ch >= 'a' && ch <= 'f') ||
		(ch >= 'A' && ch <= 'F'));
}

int HexToInt(int chHex)
{
	if (IsDigit(chHex))
	{
		return chHex - '0';
	}
	else if (chHex >= 'a' && chHex <= 'f')
		return (chHex - 'a' + 10);
	else if (chHex >= 'A' && chHex <= 'F')
		return (chHex - 'A' + 10);
	else
		return 0;
}

uint32 HexToInt(LPCTSTR str)
{
	if (_tcsnicmp(str, _T("0x"), 2) == 0)
		str += 2;

	size_t len = _tcslen(str);
	uint32	val = 0;

	for (size_t i = 0; i < len; i++)
	{
		if ('0' <= str[i] && str[i] <= '9')
			val = val * 16 + str[i] - '0';
		else if ('a' <= str[i] && str[i] <= 'f')
			val = val * 16 + str[i] - 'a' + 10;
		else if ('A' <= str[i] && str[i] <= 'F')
			val = val * 16 + str[i] - 'A' + 10;
	}

	return val;
}

// szUrl: scheme://domain:port/path?query_string#fragment_id
bool UrlParse(LPCTSTR szUrl, tstring &scheme, tstring &domain, int &port, tstring &path)
{
	LPCTSTR begin = szUrl;
	LPCTSTR p;

	// Scheme
	p = StringFeedUntil(begin, _T("://"), scheme);
	if (!p)
		return false;
	begin = p;

	// domain
	p = StringFeedUntil(begin, _T(":"), domain);
	if (p != NULL)
	{
		begin = p;

		// Port
		port = _ttoi(begin);

		tstring strPort;
		p = StringFeedUntil(begin, _T("/"), strPort);
	}
	else
	{
		port = -1;
		p = StringFeedUntil(begin, _T("/"), domain);
	}

	if (p)
		path = p;
	else
		path.clear();

	return true;
}

bool UriIsQuoted(LPCTSTR str)
{
	return _tcschr(str, '%') != NULL;
}

// Convert %20 etc to blank space...
tstring UriUnquote(LPCTSTR str)
{
	tstring out;

	string tempAnsi;
	tstring temp;
	LPCTSTR p = str;
	while (*p)
	{
		if (*p == '%' && IsHexChar(p[1]) && IsHexChar(p[2]))
		{
			int a = HexToInt(p[1]);
			int b = HexToInt(p[2]);
			tempAnsi.insert(tempAnsi.end(), (char)(byte)(a * 16 + b));
			p += 2;
		}
		else
		{
			if (tempAnsi.size() > 0)
			{
				UTF8ToTString(tempAnsi.c_str(), tempAnsi.size(), temp);
				out.append(temp);
				tempAnsi.clear();
				temp.clear();
			}
			out.insert(out.end(), *p);
		}

		p++;
	}

	if (tempAnsi.size() > 0)
	{
		UTF8ToTString(tempAnsi.c_str(), tempAnsi.size(), temp);
		out.append(temp);
		tempAnsi.clear();
		temp.clear();
	}
	out.insert(out.end(), *p);

	return out;
}

// COMMENT:
//		将汉字、' '、'	'等转换为 %20....等
void UriQuote(const char *szLocal, string &strInet)
{
	size_t	i = 0;
	char	szTemp[256];

	size_t nLen = strlen(szLocal);
	strInet = "";

	while (i < nLen)
	{
		if (BYTE(szLocal[i]) > 127)
		{
			sprintf_s(szTemp, CountOf(szTemp), "%%%2X", (BYTE)szLocal[i]);
			strInet += szTemp;
			i++;
			if (i < nLen)
			{
				sprintf_s(szTemp, CountOf(szTemp), "%%%2X", (BYTE)szLocal[i]);
				strInet += szTemp;
				i++;
			}
		}
		else if (szLocal[i] == '%' && (!IsHexChar(szLocal[i + 1]) || !IsHexChar(szLocal[i + 2])))
		{
			sprintf_s(szTemp, CountOf(szTemp), "%%%2X", (BYTE)szLocal[i]);
			strInet += szTemp;
			i++;
		}
		else if (szLocal[i] == ' ' || szLocal[i] == '\"' ||
			szLocal[i] == '#' ||
			szLocal[i] == '<' || szLocal[i] == '>')
		{
			sprintf_s(szTemp, CountOf(szTemp), "%%%2X", (BYTE)szLocal[i]);
			strInet += szTemp;
			i++;
		}
		else
		{
			strInet += szLocal[i];
			i++;
		}
	}
}

void UriQuote(const char *szLocal, char *szInet, int nLenMax)
{
	string		strTemp;

	UriQuote(szLocal, strTemp);

	strcpy_safe(szInet, nLenMax, strTemp.c_str());
}

void UriQuote(const WCHAR *szLocal, WCHAR *szInet, int nLenMax, CHAR_ENCODING encoding)
{
	stringex		strIn;
	string				str;
	wstringex		strOutW;

	ConvertStr(szLocal, -1, strIn, encoding);

	UriQuote(strIn.c_str(), str);
	ConvertStr(str.c_str(), str.size(), strOutW);

	wcscpy_safe(szInet, nLenMax, strOutW.c_str());
}

void UriQuote(const WCHAR *szLocal, wstring_t &strInet, CHAR_ENCODING encoding)
{
	stringex		strIn;
	string				str;
	wstringex		strOutW;

	ConvertStr(szLocal, -1, strIn, encoding);

	UriQuote(strIn.c_str(), str);
	ConvertStr(str.c_str(), str.size(), strOutW);

	strInet = strOutW.c_str();
}

bool IsUnicodeStr(LPCVOID lpData)
{
	BYTE						*szBuffer;

	szBuffer = (BYTE *)lpData;

	// EF BB BF
	return (szBuffer[0] == 0xFF && szBuffer[1] == 0xFE);
}

//////////////////////////////////////////////////////////////////////////
//

void ConvertStrByFileEncoding(CCharBuffer &outData, LPCTSTR szText, int nLen, FILE_ENCODING fileEncoding, int nCharEncodingID)
{
	assert(nLen >= 0);
	outData.clear();

	if (fileEncoding == FE_UCS2)
	{
		BYTE		byTag[2];
		byTag[0] = 0xFF;
		byTag[1] = 0xFE;

		outData.append((const char *)byTag, 2);

#ifdef _UNICODE
		outData.append((const char *)szText, sizeof(TCHAR) * nLen);
#else
		wstringex	strW;
		ConvertStr(szText, nLen, strW, nCharEncodingID);
		outData.append((const char *)strW.c_str(), sizeof(WCHAR) * strW.size());
#endif
	}
	else if (fileEncoding == FE_UTF8)
	{
		// EF BB BF
		BYTE		byTag[3];
		stringex	str;
		byTag[0] = 0xEF;
		byTag[1] = 0xBB;
		byTag[2] = 0xBF;

		outData.append((const char *)byTag, 3);

#ifdef _UNICODE
		UCS2ToUtf8(szText, nLen, str);
		outData.append(str.c_str(), sizeof(char) * str.size());
#else
		MbcsToUtf8(szText, nLen, str, nCharEncodingID);
		outData.append(str.c_str(), sizeof(char) * str.size());
#endif
	}
	else
	{
#ifdef _UNICODE
		stringex	str;
		ConvertStr(szText, nLen, str, nCharEncodingID);
		outData.append(str.c_str(), sizeof(char) * str.size());
#else
		outData.append(szText, sizeof(TCHAR) * nLen);
#endif
	}
}

bool SaveDataAsFile(LPCTSTR szFile, const LPCVOID lpData, int nLen)
{
	FILE	*fp;

	fp = _tfopen(szFile, _T("wb"));
	if (fp == NULL)
		return FALSE;

	if (fwrite(lpData, 1, nLen, fp) != nLen)
	{
		fclose(fp);
		return FALSE;
	}

	fclose(fp);

	return TRUE;
}

bool SaveStringAsFile(LPCTSTR szFile, LPCTSTR szText, int nLen, CHAR_ENCODING encoding)
{
	if (nLen < 0)
		nLen = _tcslen(szText);

	FILE	*fp;
	bool	bRet = true;

	fp = _tfopen(szFile, _T("wb"));
	if (fp == NULL)
		return FALSE;

	if (encoding == ED_UNICODE)
	{
		if (fwrite(SZ_FE_UCS2, 1, 2, fp) != 2)
		{
			fclose(fp);
			return FALSE;
		}

#ifdef _UNICODE
		if (fwrite(szText, sizeof(TCHAR), nLen, fp) != nLen)
			bRet = false;
#else
		wstringex	strW;
		TStringToUCS2(szText, nLen, strW, encoding);
		if (fwrite(strW.c_str(), sizeof(WCHAR), strW.size(), fp) != strW.size())
			bRet = false;
#endif
	}
	else if (encoding == ED_UTF8)
	{
		if (fwrite(SZ_FE_UTF8, 1, 3, fp) != 3)
		{
			fclose(fp);
			return FALSE;
		}

#ifdef _UNICODE
		stringex	str;
		UCS2ToUtf8(szText, nLen, str);
		if (fwrite(str.c_str(), sizeof(char), str.size(), fp) != str.size())
			bRet = false;
#else
		stringex	str;
		TStringToUTF8(szText, nLen, str);
		if (fwrite(str.c_str(), sizeof(char), str.size(), fp) != str.size())
			bRet = false;
#endif
	}
	else
	{
		stringex	str;
		TStringToMBCS(szText, nLen, str, encoding);
		if (fwrite(str.c_str(), sizeof(char), str.size(), fp) != str.size())
			bRet = false;
	}

	fclose(fp);

	return bRet;
}

bool AppendFileToBuffer(FILE *fp, CCharBuffer &str)
{
	if (fseek(fp, 0, SEEK_END) != 0)
		return false;

	long nLen = ftell(fp);
	if (nLen == 0)
		return true;

	fseek(fp, 0, SEEK_SET);

	str.reserve(str.size() + nLen + 3);
	char *szBuffer = str.data() + str.size();
	if (fread(szBuffer, 1, nLen, fp) != nLen)
		return false;

	str.setSize(str.size() + nLen);
	// 确保如果读入的是unicode编码的文本也有以'\0'结尾的字符
	szBuffer[nLen] = '\0';
	szBuffer[nLen + 1] = '\0';
	szBuffer[nLen + 2] = '\0';

	return true;
}

#ifdef WIN32
bool OpenFileToBuffer(LPCWSTR szFile, CCharBuffer &str)
{
	FILE	*fp;

	str.clear();

	fp = _wfopen(szFile, L"rb");
	if (fp == NULL)
		return FALSE;

	bool bRet = AppendFileToBuffer(fp, str);

	fclose(fp);
	return bRet;
}

#endif

bool OpenFileToBuffer(LPCSTR szFile, CCharBuffer &str)
{
	FILE	*fp;

	str.clear();

	fp = fopen(szFile, "rb");
	if (fp == NULL)
		return FALSE;

	bool bRet = AppendFileToBuffer(fp, str);

	fclose(fp);
	return bRet;
}

bool OpenFileToTString(LPCTSTR szFile, tstringex &str, CHAR_ENCODING encoding)
{
	CCharBuffer		buff;
	FILE_ENCODING	fileEncoding;

	if (!OpenFileToBuffer(szFile, buff))
		return false;

	fileEncoding = GetFileEncodingByBom(buff.c_str());
#ifdef _UNICODE
	if (fileEncoding == FE_ANSI)
		ConvertStr(buff.c_str(), buff.size(), str, encoding);
	else if (fileEncoding == FE_UTF8)
		Utf8ToUCS2(buff.c_str() + 3, buff.size() - 3, str);
	else
	{
		str.clear();
		str.append((WCHAR *)buff.c_str() + 1, buff.size() / sizeof(WCHAR) - 1);
		str += (WCHAR)'\0';
	}
#else
	// Assume using UTF-8 encoding
	if (fileEncoding == FE_ANSI)
		MbcsToUtf8(buff.c_str(), buff.size(), str, encoding);
	else if (fileEncoding == FE_UTF8)
		str = buff.c_str() + 3;
	else
	{
		UCS2ToUtf8((WCHAR *)buff.c_str() + 1, buff.size() / sizeof(WCHAR) - 1, str);
	}
#endif

	return TRUE;
}

/*
bool OpenFileToBuffer(LPCTSTR szFile, CString &str)
{
	FILE	*fp;
	int		nLen;
	LPTSTR	szBuffer;

	fp = _tfopen(szFile, _T("rb"));
	if (fp == NULL)
		return FALSE;

	if (fseek(fp, 0, SEEK_END) != 0)
		return FALSE;

	nLen = ftell(fp);
	ASSERT(nLen > 0);

	fseek(fp, 0, SEEK_SET);

	szBuffer = str.GetBuffer(nLen + 1);
	szBuffer[nLen] = '\0';
	if (fread(szBuffer, 1, nLen, fp) != nLen)
	{
		str.ReleaseBuffer(nLen);
		fclose(fp);
		return FALSE;
	}
	str.ReleaseBuffer(nLen);

	fclose(fp);

	return TRUE;
}

UINT GetTempFileName(
  LPTSTR lpTempFileName 
                       // pointer to buffer that receives the new 
                       // filename
)
{
	int		nRet;
	nRet = GetTempPath(MAX_PATH, lpTempFileName);
	ASSERT(nRet > 0 && nRet <= MAX_PATH);
	if (nRet == 0 || nRet > MAX_PATH)
		return 0;

	return GetTempFileName(lpTempFileName, _T("mg"), 0, lpTempFileName);
}

bool SaveBufferAsTempFile(LPTSTR szTempFile, LPVOID lpBuffer, int nLen)
{
	if (GetTempFileName(szTempFile) == 0)
	{
		LOG1(LOG_LVL_ERROR, _T("GetTempFileName() FAILED! LastError: %d"), GetLastError());
		return FALSE;
	}

	CFile	file;
	if (!file.Open(szTempFile, CFile::modeWrite))
	{
		LOG1(LOG_LVL_ERROR, _T("Open Temp File FAILED! LastError: %d"), GetLastError());
		return FALSE;
	}
	try
	{
		file.Write(lpBuffer, nLen);
	}
	catch (CFileException *)
	{
		LOG1(LOG_LVL_ERROR, _T("Write File FAILED! LastError: %d"), GetLastError());
		return FALSE;
	}
	file.Close();
	return TRUE;
}

*/

bool TextFileToVStr(LPCTSTR szFile, vector<string> &vStr)
{
	FILE	*fp;
	char	szBuff[1024];

	fp = _tfopen(szFile, _T("rt"));
	if (!fp)
		return false;

	while (fgets(szBuff, CountOf(szBuff) - 1, fp))
	{
		size_t nLen = strlen(szBuff);
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

void MultiStrToVStr(LPCTSTR szText, vector<tstring> &vStr)
{
	while (*szText)
	{
		vStr.push_back(szText);
		while (*szText != '\0')
			szText++;
		szText++;
	}
}

/*
void VStrToMultiStr(const vector<tstring> &vStr, tstring &strMulti)
{
	for (int i = 0; i < vStr.size(); i++)
	{
		strMulti.append(vStr[i].c_str(), vStr[i].size() + 1);
	}
	strMulti.append('\0');
}
*/

bool StrSplit(LPCTSTR szStr, TCHAR chSeperator, tstring &strLeft, tstring &strRight)
{
	strLeft.clear();
	strRight.clear();

	LPCTSTR p = _tcschr(szStr, chSeperator);
	if (p == NULL)
	{
		strLeft = szStr;
		return false;
	}

	strLeft.append(szStr, (size_t)(p - szStr));
	strRight = p + 1;
	return true;
}

bool StrSplit(LPCTSTR szStr, LPCTSTR szSeperator, tstring &strLeft, tstring &strRight)
{
	strLeft.clear();
	strRight.clear();

	LPCTSTR p = _tcsstr(szStr, szSeperator);
	if (p == NULL)
	{
		strLeft = szStr;
		return false;
	}

	strLeft.append(szStr, (size_t)(p - szStr));
	strRight = p + _tcslen(szSeperator);
	return true;
}

class _SetInsert
{
public:
	void add(tstring &strVal)
	{
		m_setStr->insert(strVal);
	}
	SetTStrNCase	*m_setStr;
};

class _VectorInsert
{
public:
	void add(tstring &strVal)
	{
		m_vecStr->push_back(strVal);
	}
	vector<tstring>	*m_vecStr;
};

template<class _VectorInsert>
void _StrSplit(LPCTSTR szStr, TCHAR chSeperator, _VectorInsert &containerStr)
{
	if (IsEmptyString(szStr))
		return;

	LPCTSTR		szPos, szPrev;

	szPrev = szPos = szStr;
	tstring		strTemp;
	while (szPos && *szPos)
	{
		szPos = _tcschr(szPos, chSeperator);
		if (szPos)
		{
			strTemp.resize(0);
			strTemp.append(szPrev, int(szPos - szPrev));
			containerStr.add(strTemp);
			szPos++;
		}
		else
		{
			strTemp = szPrev;
			containerStr.add(strTemp);
			return;
		}

		szPrev = szPos;
	}

	strTemp = _T("");
	containerStr.add(strTemp);
}

int StrSplit(LPCTSTR szStr, TCHAR chSeperator, SetTStrNCase &containerStr)
{
	_SetInsert	set;
	set.m_setStr = &containerStr;
	_StrSplit(szStr, chSeperator, set);

	return (int)containerStr.size();
}

int StrSplit(LPCTSTR szStr, TCHAR chSeperator, vector<tstring> &containerStr)
{
	_VectorInsert vec;
	vec.m_vecStr = &containerStr;
	_StrSplit(szStr, chSeperator, vec);

	return (int)containerStr.size();
}

LPCTSTR StringFeedUntil(LPCTSTR begin, LPCTSTR szUtil, tstring &buf)
{
	LPCTSTR p = _tcsstr(begin, szUtil);
	if (!p)
		return NULL;

	buf.clear();
	buf.append(begin, (size_t)(p - begin));
	return p + _tcslen(szUtil);
}

// "#FFFFFF"
template<class _TCHARPTR, class _int_t>
_TCHARPTR ReadColorValue_t(_TCHARPTR szColor, _int_t &nClr)
{
	nClr = 0;

	if (*szColor == '#')
		szColor++;

	int		n;

	for (int i = 0; i < 6 && *szColor; i++)
	{
		if (*szColor >= '0' && *szColor <= '9')
			n = *szColor - '0';
		else if (*szColor >= 'a' && *szColor <= 'f')
			n = 10 + *szColor - 'a';
		else if (*szColor >= 'A' && *szColor <= 'F')
			n = 10 + *szColor - 'A';
		else
			break;

		nClr = nClr * 16 + n;
		szColor++;
	}

	nClr = ((nClr & 0xFF) << 16) + (nClr & 0xFF00) + ((nClr & 0xFF0000) >> 16);

	return szColor;
}

int StringToInt(LPCTSTR str, int nDefault)
{
	int nValue = 0;
	LPCTSTR end = ReadInt_t(str, nValue);
	if (end == str)
		return nDefault;

	return nValue;
}

// "#FFFFFF"
COLORREF StringToColor(LPCTSTR szColor, COLORREF nDefault)
{
	if (IsEmptyString(szColor))
		return nDefault;

	UINT		clr;
	LPCTSTR p = ReadColorValue_t(szColor, clr);
	if (*p != '\0')
		return nDefault;
	else
		return clr;
}

tstring StringFromInt(int value)
{
	TCHAR		szBuff[32];

	_itot_s(value, szBuff, CountOf(szBuff), 10);
	return szBuff;
}

tstring StringFromColor(COLORREF clr)
{
	TCHAR	szColor[32];

	StringFromColor(szColor, clr);
	return szColor;
}

void StringFromColor(TCHAR szStr[], COLORREF clr)
{
	_stprintf_s(szStr, 9, _T("#%02X%02X%02X"), clr & 0xFF, (clr >> 8) & 0xff, (clr >> 16) & 0xff);
}

#ifndef _WIN32
// For gcc compiler, sizeof(wchar_t) == 4, need to reimplementation wcsxxx functions.

size_t	wcslen(const WCHAR *str)
{
        const WCHAR *eos = str;

        while( *eos++ ) ;

        return( (size_t)(eos - str - 1) );
}

int __cdecl wcsncmp (
        const WCHAR * first,
        const WCHAR * last,
        size_t count
        )
{
        if (!count)
                return(0);

        while (--count && *first && *first == *last)
        {
                first++;
                last++;
        }

        return((int)(*first - *last));
}

int wcscmp (
        const WCHAR * src,
        const WCHAR * dst
        )
{
        int ret = 0 ;

        while( ! (ret = (int)(*src - *dst)) && *dst)
                ++src, ++dst;

        if ( ret < 0 )
                ret = -1 ;
        else if ( ret > 0 )
                ret = 1 ;

        return( ret );
}

WCHAR * __cdecl wcsstr (
        const WCHAR * wcs1,
        const WCHAR * wcs2
        )
{
        WCHAR *cp = (WCHAR *) wcs1;
        WCHAR *s1, *s2;

        if ( !*wcs2)
            return (WCHAR *)wcs1;

        while (*cp)
        {
                s1 = cp;
                s2 = (WCHAR *) wcs2;

                while ( *s1 && *s2 && !(*s1-*s2) )
                        s1++, s2++;

                if (!*s2)
                        return(cp);

                cp++;
        }

        return(NULL);
}

WCHAR * __cdecl wcschr (
        const WCHAR * string,
        WCHAR ch
        )
{
        while (*string && *string != (WCHAR)ch)
                string++;

        if (*string == (WCHAR)ch)
                return((WCHAR *)string);
        return(NULL);
}

#endif // #ifndef _WIN32

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

#ifdef _CPPUNIT_TEST

IMPLEMENT_CPPUNIT_TEST_REG(StringEx)

class CTestCaseStringEx : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(CTestCaseStringEx);
	CPPUNIT_TEST(TestStringFeedUntil);
	CPPUNIT_TEST(TestUrlParse);
	CPPUNIT_TEST(TestReadInt_t);
	CPPUNIT_TEST(TestReadColorValue_t);
	CPPUNIT_TEST(TestTrimStrRight);
	CPPUNIT_TEST(TestStrSplit);
	CPPUNIT_TEST(TestStrSplit2);
	CPPUNIT_TEST(TestStrJoin);
	CPPUNIT_TEST(TestStrReplace);
	CPPUNIT_TEST(TestStrBeginWith);
	CPPUNIT_TEST(TestStrEndWith);
	CPPUNIT_TEST(Test_xcsncpy);
	CPPUNIT_TEST(TestUriUnquote);
	CPPUNIT_TEST_SUITE_END();

protected:
	void TestStringFeedUntil()
	{
		LPCTSTR p, begin, szUtil;
		tstring buf;

		begin = _T("http://domain");
		szUtil = _T(":");
		p = StringFeedUntil(begin, szUtil, buf);
		CPPUNIT_ASSERT(p == begin + 5);
		CPPUNIT_ASSERT(buf == _T("http"));

		p = StringFeedUntil(begin, _T("*"), buf);
		CPPUNIT_ASSERT(p == NULL);

		p = StringFeedUntil(begin, _T("h"), buf);
		CPPUNIT_ASSERT(p == begin + 1);
		CPPUNIT_ASSERT(buf == _T(""));
	}

	void TestUrlParse()
	{
		LPCTSTR szUrl;

		tstring scheme, domain, path;
		int port;

		szUrl = _T("scheme://domain:10/path?query_string#fragment_id");
		CPPUNIT_ASSERT(UrlParse(szUrl, scheme, domain, port, path));
		CPPUNIT_ASSERT(scheme == _T("scheme"));
		CPPUNIT_ASSERT(domain == _T("domain"));
		CPPUNIT_ASSERT(path == _T("path?query_string#fragment_id"));
		CPPUNIT_ASSERT(port == 10);

		szUrl = _T("scheme://domain/path?query_string#fragment_id");
		CPPUNIT_ASSERT(UrlParse(szUrl, scheme, domain, port, path));
		CPPUNIT_ASSERT(scheme == _T("scheme"));
		CPPUNIT_ASSERT(domain == _T("domain"));
		CPPUNIT_ASSERT(path == _T("path?query_string#fragment_id"));
		CPPUNIT_ASSERT(port == -1);

		szUrl = _T("scheme://domain");
		CPPUNIT_ASSERT(UrlParse(szUrl, scheme, domain, port, path));
		CPPUNIT_ASSERT(scheme == _T("scheme"));
		CPPUNIT_ASSERT(domain == _T("domain"));
		CPPUNIT_ASSERT(path == _T(""));
		CPPUNIT_ASSERT(port == -1);

		szUrl = _T("scheme://domain:12");
		CPPUNIT_ASSERT(UrlParse(szUrl, scheme, domain, port, path));
		CPPUNIT_ASSERT(scheme == _T("scheme"));
		CPPUNIT_ASSERT(domain == _T("domain"));
		CPPUNIT_ASSERT(path == _T(""));
		CPPUNIT_ASSERT(port == 12);
	}

	void TestTrimStrRight()
	{
		TCHAR szStr[1024] = _T("abcdef1213   ");
		TrimStrRight(szStr);
		CPPUNIT_ASSERT(_tcscmp(szStr, _T("abcdef1213")) == 0);
		TrimStrRight(szStr);
		CPPUNIT_ASSERT(_tcscmp(szStr, _T("abcdef1213")) == 0);
		TrimStrRight(szStr, _T("314"));
		CPPUNIT_ASSERT(_tcscmp(szStr, _T("abcdef12")) == 0);
		TrimStrRight(szStr, _T("cd21f"));
		CPPUNIT_ASSERT(_tcscmp(szStr, _T("abcde")) == 0);
		TrimStrRight(szStr, _T("cd21f"));
		CPPUNIT_ASSERT(_tcscmp(szStr, _T("abcde")) == 0);
		TrimStrRight(szStr, _T("cde21f"));
		CPPUNIT_ASSERT(_tcscmp(szStr, _T("ab")) == 0);
		TrimStrRight(szStr, _T(" ab+1234fo"));
		CPPUNIT_ASSERT(_tcscmp(szStr, _T("")) == 0);
	}

	void TestReadInt_t()
	{
		int			nValue;
		LPCTSTR		p;

		p = ReadInt_t(_T("10"), nValue);
		CPPUNIT_ASSERT(*p == '\0' && nValue == 10);

		p = ReadInt_t(_T("-110"), nValue);
		CPPUNIT_ASSERT(*p == '\0' && nValue == -110);

		p = ReadInt_t(_T("99c"), nValue);
		CPPUNIT_ASSERT(*p == 'c' && nValue == 99);

		p = ReadInt_t(_T(""), nValue);
		CPPUNIT_ASSERT(*p == '\0' && nValue == 0);

		p = ReadInt_t(_T("a1bc"), nValue);
		CPPUNIT_ASSERT(*p == 'a' && nValue == 0);
	}

	void TestReadColorValue_t()
	{
		COLORREF	clr;
		LPCTSTR		p;

		p = ReadColorValue_t(_T("#576C91"), clr);
		CPPUNIT_ASSERT(*p == '\0' && clr == RGB(0x57, 0x6c, 0x91));

		p = ReadColorValue_t(_T("#576C91a"), clr);
		CPPUNIT_ASSERT(*p == 'a' && clr == RGB(0x57, 0x6c, 0x91));

		p = ReadColorValue_t(_T("#76C91"), clr);
		CPPUNIT_ASSERT(*p == '\0' && clr == RGB(0x7, 0x6c, 0x91));

		p = ReadColorValue_t(_T("576C91"), clr);
		CPPUNIT_ASSERT(*p == '\0' && clr == RGB(0x57, 0x6c, 0x91));

		p = ReadColorValue_t(_T("576C910"), clr);
		CPPUNIT_ASSERT(*p == '0' && clr == RGB(0x57, 0x6c, 0x91));
	}

	void TestStrSplit()
	{
		V_Str		vStr;
		SetTStrNCase setStr;
		LPCTSTR		szInput = _T("a,ab,a10,,A,");
		LPCTSTR		strResult[] = { _T("a"), _T("ab"), _T("a10"), _T(""), _T("A"), _T("") };

		StrSplit(szInput, ',', vStr);
		StrSplit(szInput, ',', setStr);

		CPPUNIT_ASSERT(vStr.size() == CountOf(strResult));

		for (int i = 0; i < vStr.size(); i++)
		{
			if (_tcscmp(vStr[i].c_str(), strResult[i]) != 0)
				CPPUNIT_FAIL_T(CStrPrintf(_T("StrSplit Test, case: %d, %s, %s"), i, vStr[i].c_str(), strResult[i]).c_str());
		}

		CPPUNIT_ASSERT(setStr.size() + 2 == CountOf(strResult));

		for (int i = 0; i < vStr.size(); i++)
		{
			if (setStr.find(strResult[i]) == setStr.end())
				CPPUNIT_FAIL_T(CStrPrintf(_T("StrSplit(Set) Test, case(Can't find): %d, %s"), i, strResult[i]).c_str());
		}
	}

	void TestStrSplit2()
	{
		tstring		strLeft, strRight;
		LPCTSTR		szInput = _T("name=valuexxx=x");

		StrSplit(szInput, '=', strLeft, strRight);
		CPPUNIT_ASSERT(strLeft == _T("name"));
		CPPUNIT_ASSERT(strRight == _T("valuexxx=x"));

		szInput = _T("name - valuexxx=x");
		StrSplit(szInput, _T(" - "), strLeft, strRight);
		CPPUNIT_ASSERT(strLeft == _T("name"));
		CPPUNIT_ASSERT(strRight == _T("valuexxx=x"));
	}

	void TestStrJoin()
	{
		tstring		str;

		int vInt[] = { 1, 2, 3, 4 };
		str = StrJoin(vInt, vInt + CountOf(vInt), _T("%d"), _T(","));
		CPPUNIT_ASSERT(str == _T("1,2,3,4"));

		V_Str		vStr;
		vStr.push_back(_T("1"));
		vStr.push_back(_T("2"));
		vStr.push_back(_T("3"));
		str = StrJoin(vStr.begin(), vStr.end(), _T(","));
		CPPUNIT_ASSERT(str == _T("1,2,3"));
	}

	void TestStrReplace()
	{
		LPCTSTR		szSrc[] = { _T("abca"), _T("baAcd"), _T("a") };
		LPCTSTR		szDst[] = { _T("xbcx"), _T("bxAcd"), _T("x") };

		for (int i = 0; i < CountOf(szSrc); i++)
		{
			tstring		str = szSrc[i];
			strrep(str, 'a', 'x');
			CPPUNIT_ASSERT(_tcscmp(szDst[i], str.c_str()) == 0);
		}
	}

	void TestStrBeginWith()
	{
		LPCTSTR		szSrc[] = { _T("abca"), _T("baAcd"), _T("aXDegae"), _T("aXDegae") };
		LPCTSTR		szBeg[] = { _T(""), _T("b"), _T("aXD"), _T("aXDegae") };
		LPCTSTR		szIBeg[] = { _T(""), _T("B"), _T("axd"), _T("aXDegaE") };
		LPCTSTR		szBegFalse[] = { _T("ac"), _T("bb"), _T("x"), _T("aXDegaeD") };

		for (int i = 0; i < CountOf(szSrc); i++)
		{
			CPPUNIT_ASSERT(StrBeginWith(szSrc[i], szBeg[i]));
			CPPUNIT_ASSERT(StrIBeginWith(szSrc[i], szIBeg[i]));
			CPPUNIT_ASSERT(!StrBeginWith(szSrc[i], szBegFalse[i]));
		}
	}

	void TestStrEndWith()
	{
		LPCTSTR		szSrc[] = { _T("abca"), _T("baAcd"), _T("aXDegae"), _T("aXDegae") };
		LPCTSTR		szEnd[] = { _T(""), _T("d"), _T("egae"), _T("aXDegae") };
		LPCTSTR		szIEnd[] = { _T(""), _T("D"), _T("EGaE"), _T("aXDegaE") };
		LPCTSTR		szEndFalse[] = { _T("ac"), _T("bb"), _T("x"), _T("aXDegaeD") };

		for (int i = 0; i < CountOf(szSrc); i++)
		{
			CPPUNIT_ASSERT(StrEndWith(szSrc[i], szEnd[i]));
			CPPUNIT_ASSERT(StrIEndWith(szSrc[i], szIEnd[i]));
			CPPUNIT_ASSERT(!StrEndWith(szSrc[i], szEndFalse[i]));
		}
	}

	void Test_xcsncpy()
	{
		LPCTSTR		szSrc[] = { _T("abca"), _T("baAcd"), _T("aXDegae"), _T("aXDegae") };
		char		szTemp[64];
		WCHAR		szTempW[64];

		for (int i = 0; i < CountOf(szSrc); i++)
		{
			_xcsncpy(szTemp, szSrc[i], 1);
			CPPUNIT_ASSERT(szTemp[0] == szSrc[i][0]);
			CPPUNIT_ASSERT(szTemp[1] == 0);

			_xcsncpy(szTemp, szSrc[i], _tcslen(szSrc[i]));
			CPPUNIT_ASSERT(_xcscmp(szTemp, szSrc[i]) == 0);

			_xcsncpy(szTempW, szSrc[i], _tcslen(szSrc[i]));
			CPPUNIT_ASSERT(_xcscmp(szTempW, szSrc[i]) == 0);
		}
	}

	void TestUriUnquote()
	{
		LPCTSTR cases[] = { _T("%E5%BC%A0%E6%AF%85"), _T("artist.movie.1080p.xxx"), 
			_T(""), _T("%2%X1"), };

		LPCTSTR results[] = { _T("张毅"), _T("artist.movie.1080p.xxx"), _T(""), _T("%2%X1"), };

		CPPUNIT_ASSERT(UriIsQuoted(cases[0]));
		for (int i = 0; i < CountOf(cases); i++)
		{
			tstring r = UriUnquote(cases[i]);
			CPPUNIT_ASSERT(_tcscmp(results[i], r.c_str()) == 0);
		}
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseStringEx);

#endif // _CPPUNIT_TEST
