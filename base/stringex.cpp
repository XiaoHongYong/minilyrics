#include "stdafx.h"
#include "error.h"
#include "stringex_t.h"
#include "CharEncoding.h"
#include "stringex.h"
#include "safestr.h"
#include "xstr.h"

#ifdef _UNICODE
typedef	wchar_t	UTCHAR;
#else
typedef	unsigned char	UTCHAR;
#endif

FILE_ENCODING GetStrEncodingType(LPCVOID lpData)
{
	BYTE						*szBuffer;

	szBuffer = (BYTE *)lpData;

	// FF FE
	if (szBuffer[0] == 0xFF && szBuffer[1] == 0xFE)
		return FE_UCS2;
	// EF BB BF
	else if (szBuffer[0] == 0xEF && szBuffer[1] == 0xBB && szBuffer[2] == 0xBF)
		return FE_UTF8;
	else
		return FE_ANSI;
}

void strrep(tstring &str, const TCHAR *szSrc, const TCHAR *szDest)
{
	int		nPos;
	int		nLenSrc = _tcslen(szSrc);
	int		nLenDest = _tcslen(szDest);
	LPCTSTR	szPos;

	nPos = 0;
	while (nPos < (int)str.size())
	{
		szPos = _tcsstr(str.c_str() + nPos, szSrc);
		if (szPos)
		{
			nPos = int(szPos - str.c_str());
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
	int		nPos;
	int		nLenSrc = strlen(szSrc);
	int		nLenDest = strlen(szDest);
	LPCSTR	szPos;

	nPos = 0;
	while (nPos < (int)str.size())
	{
		szPos = strstr(str.c_str() + nPos, szSrc);
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

void strrep(wstringex &str, const WCHAR *szSrc, const WCHAR *szDest)
{
	int		nPos;
	int		nLenSrc = wcslen(szSrc);
	int		nLenDest = wcslen(szDest);
	LPCWSTR	szPos;

	nPos = 0;
	while (nPos < (int)str.size())
	{
		szPos = wcsstr(str.c_str() + nPos, szSrc);
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

void TrimStr(tstring &str, TCHAR ch)
{
	tstring::iterator	it;
	int		i;

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
		TrimStr(vStr[i]);
}

//
// 去掉字符串两端的空格
void TrimStr(LPTSTR szString, TCHAR ch)
{
	int		nLen;
	LPTSTR	szBeg;
	szBeg = szString;

	while (*szBeg == ch)
		szBeg++;

	nLen = _tcslen(szBeg);
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
	tstring		str;

	for (int i = 0; szString[i] != '\0'; i ++)
		if ('a' <= szString[i]  && szString[i] <= 'z')
			str[i] += 'A' - 'a';

	return str;
}

tstring ToLower(LPCTSTR szString)
{
	tstring		str;

	str = szString;
	for (int i = 0; szString[i] != '\0'; i ++)
		if ('A' <= szString[i]  && szString[i] <= 'Z')
			str[i] += 'a' - 'A';

	return str;
}

static LPCTSTR		szEscape[] = 
{
	_T("&nbsp;"), _T(" "),
	_T("&lt;"), _T("<"),
	_T("&gt;"), _T(">"),
	_T("&amp;"), _T("&"),
	_T("&quot;"), _T("\""),
	_T("&apos;"), _T("\\"),
	NULL, _T("")
};

// COMMENT:
//		过滤所有的HTML标记
int FilterAllHTMLTags(LPTSTR szText)
{
	LPTSTR	szBeg, szCopyPos, szEnd;
	int		i;

	szCopyPos = szText;
	szBeg = szText;
	szEnd = szBeg + _tcslen(szText);

	BOOL bSpace = FALSE;
	while (szBeg < szEnd)
	{
		// 复制TAG外字符串
		while (*szBeg != '<' && szBeg < szEnd)
		{
			if (*szBeg == '&')
			{
				for (i = 0; szEscape[i] != NULL; i += 2)
				{
					int len = _tcslen(szEscape[i]);
					if (_tcsnicmp(szBeg, szEscape[i], len) == 0)
					{
						szBeg += len;
						*szCopyPos = szEscape[i + 1][0];
						szCopyPos++;
						break;
					}
				}
				if (szEscape[i] == NULL)
				{
					// &#39;
					LPTSTR		szTemp = szBeg;
					if (szBeg[1] == '#' && isdigit(szBeg[2]))
					{
						int		n = _ttoi(szBeg + 2);
						if (n > 0 && n <= 255)
						{
							szBeg += 2;
							while (isdigit(*szBeg))
								szBeg++;
							if (*szBeg == ';')
								szBeg++;
							*szCopyPos = (TCHAR)n;
							szCopyPos++;
						}
					}

					if (szTemp == szBeg)
					{
						*szCopyPos = *szBeg;
						szBeg++;
						szCopyPos++;
					}
				}
			}
			else if (_tcsnicmp(szBeg, _T("\r\n"), 2) == 0)
			{
				szBeg += 2;
			}
			else if (_tcsnicmp(szBeg, _T("\n"), 1) == 0)
			{
				szBeg += 1;
			}
			else
			{
				// 如果上次为空格则过滤该空格
				if (*szBeg == ' ' || *szBeg == '\t')
				{
					if (bSpace)
					{
						szBeg++;
					}
					else
					{
						*szCopyPos = ' ';//*szBeg;
						szBeg++;
						szCopyPos++;
						bSpace = TRUE;
					}
				}
				else
				{
					*szCopyPos = *szBeg;
					szBeg++;
					szCopyPos++;
					bSpace = FALSE;
				}
			}
		}

		static LPCTSTR		szToNewLine[] = 
		{
			_T("<BR>"), _T("<br/>"), _T("<br />"), _T("</P>"), _T("</div>"), _T("</h1>"), _T("</h2>"), _T("</h3>"), _T("</li>"),
			_T("</TR>"), _T("<br >"),
		};
		bool	bReplaced = false;

		for (i = 0; i < CountOf(szToNewLine); i++)
		{
			int		n = _tcslen(szToNewLine[i]);
			if (_tcsnicmp(szBeg, szToNewLine[i], n) == 0)
			{
				_tcscpy_safe(szCopyPos, 4, _T("\r\n"));
				szCopyPos += 2;	// _tcslen("\r\n");
				szBeg += n;
				bReplaced = true;
				break;
			}
		}

		if (!bReplaced)
		{
			LPCTSTR		szIgnoreTags[] = 
			{
				_T("<style>"), _T("</style>"), 
				_T("<head>"), _T("</head>"), 
				_T("<title>"), _T("</title>"), 
				_T("<noscript>"), _T("</noscript>"), 
				_T("<script"), _T("</script>"), 
				_T("<!--"), _T("-->"), 
			};

			for (i = 0; i < CountOf(szIgnoreTags); i += 2)
			{
				int		n = _tcslen(szIgnoreTags[i]);
				if (_tcsnicmp(szBeg, szIgnoreTags[i], n) == 0)
				{
					szBeg = stristr(szBeg, szIgnoreTags[i + 1]);
					if (szBeg)
						szBeg += _tcslen(szIgnoreTags[i + 1]);
					else
						szBeg = szEnd;
					bReplaced = true;
					break;
				}
			}
		}

		if (bReplaced)
		{
		}
		else if (_tcsnicmp(szBeg, _T("<pre "), 5) == 0)
		{
			szBeg += 5;
			szBeg = _tcschr(szBeg, '>');
			if (szBeg)
			{
				szBeg += 1;
				while (*szBeg && *szBeg != '<')
				{
					*szCopyPos = *szBeg;
					szBeg++;
					szCopyPos++;
				}
			}
			else
				szBeg = szEnd;
		}
		else
		{
			while (*szBeg != '>' && szBeg < szEnd)
				szBeg++;
			if (*szBeg == '>')
				szBeg++;
		}
	}

	*szCopyPos = '\0';

	return int(szEnd - szCopyPos);
}

// COMMENT:
//		判断字符串是否为数字串
//		规则：必须包含至少一个以上，且全为数字的数字才是数字串
BOOL IsStrNumeric(LPCTSTR szString)
{
	LPCTSTR		szTemp;
	szTemp = szString;

	while (IsDigit(*szTemp))
		szTemp++;
	
	if (*szTemp != '\0')
		return FALSE;

	// 为空
	if (szString == szTemp)
		return FALSE;

	return TRUE;
}


BOOL IsHexChar(char ch)
{
	return (IsDigit(ch) || 
		(ch >= 'a' && ch <= 'f') ||
		(ch >= 'A' && ch <= 'F'));
}

int HexToInt(char chHex)
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

// COMMENT:
//		将%20等类似的名字转换为对应的字符
void UriUnescape(const char *szInet, string &str)
{
	int		i = 0;
	int		nLenInet;

	nLenInet = strlen(szInet);
	str = "";

	while (i + 2 < nLenInet)
	{
		if (szInet[i] == '%')
		{
			if (IsHexChar(szInet[i + 1])
				&& IsHexChar(szInet[i + 2]))
			{
				BYTE		ch;
				ch = (BYTE)HexToInt(szInet[i + 1]) * 0x10;
				ch += (BYTE)HexToInt(szInet[i + 2]);
				str += (char)ch;
				i += 3;
				continue;
			}
		}

		str += szInet[i];
		i++;
	}
	while(szInet[i] != '\0')
	{
		str += szInet[i];
		i++;
	}
}

// COMMENT:
//		将%20等类似的名字转换为对应的字符
void UriUnescape(const char *szInet, char *szLocal, int nLenMax)
{
	int		i = 0, k = 0;
	int		nLenInet;

	nLenInet = strlen(szInet);

	while (i + 2 < nLenInet)
	{
		if (szInet[i] == '%')
		{
			if (IsHexChar(szInet[i + 1])
				&& IsHexChar(szInet[i + 2]))
			{
				szLocal[k] = (BYTE)HexToInt(szInet[i + 1]) * 0x10;
				szLocal[k] += (BYTE)HexToInt(szInet[i + 2]);
				k++;
				i += 3;
				continue;
			}
		}

		szLocal[k] = szInet[i];
		k++;
		i++;
	}
	while(szInet[i] != '\0')
	{
		szLocal[k] = szInet[i];
		k++;
		i++;
	}
	szLocal[k] = '\0';
}

void UriUnescape(const WCHAR *szInet, WCHAR *szLocal, int nLenMax, CHAR_ENCODING encoding)
{
	stringex		strIn;
	string			strOut;
	wstringex		strOutW;

	ConvertStr(szInet, -1, strIn);

	UriUnescape(strIn.c_str(), strOut);
	ConvertStr(strOut.c_str(), strOut.size(), strOutW, encoding);

	wcscpy_safe(szLocal, nLenMax, strOutW.c_str());
}

void UriUnescape(const WCHAR *szInet, wstring &str, CHAR_ENCODING encoding)
{
	stringex		strIn;
	string			strOut;
	wstringex		strOutW;

	ConvertStr(szInet, -1, strIn);

	UriUnescape(strIn.c_str(), strOut);
	ConvertStr(strOut.c_str(), strOut.size(), strOutW, encoding);

	str = strOutW.c_str();
}

// COMMENT:
//		将汉字、' '、'	'等转换为 %20....等
void UriEscape(const char *szLocal, string &strInet)
{
	int		i = 0;
	char	szTemp[256];
	int		nLen;

	nLen = strlen(szLocal);
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

void UriEscape(const char *szLocal, char *szInet, int nLenMax)
{
	string		strTemp;

	UriEscape(szLocal, strTemp);

	strcpy_safe(szInet, nLenMax, strTemp.c_str());
}

void UriEscape(const WCHAR *szLocal, WCHAR *szInet, int nLenMax, CHAR_ENCODING encoding)
{
	stringex		strIn;
	string				str;
	wstringex		strOutW;

	ConvertStr(szLocal, -1, strIn, encoding);

	UriEscape(strIn.c_str(), str);
	ConvertStr(str.c_str(), str.size(), strOutW);

	wcscpy_safe(szInet, nLenMax, strOutW.c_str());
}

void UriEscape(const WCHAR *szLocal, wstring &strInet, CHAR_ENCODING encoding)
{
	stringex		strIn;
	string				str;
	wstringex		strOutW;

	ConvertStr(szLocal, -1, strIn, encoding);

	UriEscape(strIn.c_str(), str);
	ConvertStr(str.c_str(), str.size(), strOutW);

	strInet = strOutW.c_str();
}

BOOL IsUnicodeStr(LPCVOID lpData)
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

BOOL SaveDataAsFile(LPCTSTR szFile, const LPCVOID lpData, int nLen)
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

BOOL SaveStringAsFile(LPCTSTR szFile, LPCTSTR szText, int nLen, FILE_ENCODING fileEncoding, int nCharEncodingID)
{
	assert(nLen >= 0);

	FILE	*fp;
	BOOL	bRet = true;

	fp = _tfopen(szFile, _T("wb"));
	if (fp == NULL)
		return FALSE;

	if (fileEncoding == FE_UCS2)
	{
		BYTE		byTag[2];
		byTag[0] = 0xFF;
		byTag[1] = 0xFE;

		if (fwrite(byTag, 1, 2, fp) != 2)
		{
			fclose(fp);
			return FALSE;
		}

#ifdef _UNICODE
		if (fwrite(szText, sizeof(TCHAR), nLen, fp) != nLen)
			bRet = false;
#else
		wstringex	strW;
		ConvertStr(szText, nLen, strW, nCharEncodingID);
		if (fwrite(strW.c_str(), sizeof(WCHAR), strW.size(), fp) != strW.size())
			bRet = false;
#endif
	}
	else if (fileEncoding == FE_UTF8)
	{
		// EF BB BF
		BYTE		byTag[3];
		byTag[0] = 0xEF;
		byTag[1] = 0xBB;
		byTag[2] = 0xBF;

		if (fwrite(byTag, 1, 3, fp) != 3)
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
		MbcsToUtf8(szText, nLen, str, nCharEncodingID);
		if (fwrite(str.c_str(), sizeof(char), str.size(), fp) != str.size())
			bRet = false;
#endif
	}
	else
	{
#ifdef _UNICODE
		stringex	str;
		ConvertStr(szText, nLen, str, nCharEncodingID);
		if (fwrite(str.c_str(), sizeof(char), str.size(), fp) != str.size())
		{
			fclose(fp);
			return FALSE;
		}
#else
		if (fwrite(szText, sizeof(TCHAR), nLen, fp) != nLen)
			bRet = false;
#endif
	}

	fclose(fp);

	return bRet;
}

#ifdef WIN32
BOOL OpenFileToBuffer(LPCWSTR szFile, CCharBuffer &str)
{
	FILE	*fp;
	int		nLen;
	char	*szBuffer;

	str.clear();

	fp = _wfopen(szFile, L"rb");
	if (fp == NULL)
		return FALSE;

	if (fseek(fp, 0, SEEK_END) != 0)
	{
		fclose(fp);
		return FALSE;
	}

	nLen = ftell(fp);
	if (nLen == 0)
	{
		fclose(fp);
		return TRUE;
	}

	fseek(fp, 0, SEEK_SET);

	str.reserve(nLen + 3);
	szBuffer = str.data();
	if (fread(szBuffer, 1, nLen, fp) != nLen)
	{
		fclose(fp);
		return FALSE;
	}

	str.setSize(nLen);
	// 确保如果读入的是unicode编码的文本也有以'\0'结尾的字符
	szBuffer[nLen] = '\0';
	szBuffer[nLen + 1] = '\0';
	szBuffer[nLen + 2] = '\0';

	fclose(fp);

	return TRUE;
}
#endif

BOOL OpenFileToBuffer(LPCSTR szFile, CCharBuffer &str)
{
	FILE	*fp;
	int		nLen;
	char	*szBuffer;

	str.clear();

	fp = fopen(szFile, "rb");
	if (fp == NULL)
		return FALSE;

	if (fseek(fp, 0, SEEK_END) != 0)
	{
		fclose(fp);
		return FALSE;
	}

	nLen = ftell(fp);
	if (nLen == 0)
	{
		fclose(fp);
		return TRUE;
	}

	fseek(fp, 0, SEEK_SET);

	str.reserve(nLen + 3);
	szBuffer = str.data();
	if (fread(szBuffer, 1, nLen, fp) != nLen)
	{
		fclose(fp);
		return FALSE;
	}

	str.setSize(nLen);
	// 确保如果读入的是unicode编码的文本也有以'\0'结尾的字符
	szBuffer[nLen] = '\0';
	szBuffer[nLen + 1] = '\0';
	szBuffer[nLen + 2] = '\0';

	fclose(fp);

	return TRUE;
}

BOOL OpenFileToTString(LPCTSTR szFile, tstringex &str, CHAR_ENCODING encoding)
{
	CCharBuffer		buff;
	FILE_ENCODING	fileEncoding;

	if (!OpenFileToBuffer(szFile, buff))
		return false;

	fileEncoding = GetStrEncodingType(buff.c_str());
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
BOOL OpenFileToBuffer(LPCTSTR szFile, CString &str)
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

BOOL SaveBufferAsTempFile(LPTSTR szTempFile, LPVOID lpBuffer, int nLen)
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

	return containerStr.size();
}

int StrSplit(LPCTSTR szStr, TCHAR chSeperator, vector<tstring> &containerStr)
{
	_VectorInsert vec;
	vec.m_vecStr = &containerStr;
	_StrSplit(szStr, chSeperator, vec);

	return containerStr.size();
}

void _itostr(int value, tstring &str, int base)
{
	TCHAR		szBuff[32];

	_itot_s(value, szBuff, CountOf(szBuff), base);
	str = szBuff;
}

void ColorToStr(COLORREF clr, char szStr[])
{
	sprintf_s(szStr, 9, "#%02X%02X%02X", clr & 0xFF, (clr >> 8) & 0xff, (clr >> 16) & 0xff);
}


#ifdef _WIN32
void ColorToStr(COLORREF clr, WCHAR szStr[])
{
	swprintf_s(szStr, 9, L"#%02X%02X%02X", clr & 0xFF, (clr >> 8) & 0xff, (clr >> 16) & 0xff);
}
#endif

#ifdef _CPPUNIT_TEST

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

IMPLEMENT_CPPUNIT_TEST_REG(StringEx)

class CTestCaseStringEx : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(CTestCaseStringEx);
	CPPUNIT_TEST(TestReadInt_t);
	CPPUNIT_TEST(TestReadColorValue_t);
	CPPUNIT_TEST(TestTrimStrRight);
	CPPUNIT_TEST(TestStrSplit);
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

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseStringEx);


#endif // _CPPUNIT_TEST
