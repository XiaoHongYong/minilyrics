#pragma once

#ifndef EmptyStr
#define EmptyStr(str)	(str[0] = '\0')
#define CountOf(arr)	(sizeof(arr) / sizeof(arr[0]))

#define IsEmptyString(str)	(str[0] == '\0')
#endif

int Utf8ToUCS2(const char* text, int length, WCHAR *ucs2, int ubuflen);

BOOL GetProgramPath(char szPath[], HINSTANCE hInstance/* = NULL*/);

LPCSTR UrlGetName(LPCSTR szFile);

bool IsFileExist(LPCSTR szFileName);

void DirStringFormat(LPSTR	szDir);

BOOL WritePrivateProfileInt(
							LPCSTR lpAppName,  // section name
							LPCSTR lpKeyName,  // key name
							int nValue,			// int to add
							LPCSTR lpFileName  // initialization file
							);

char *strcpy_safe(char *strDestination, int nLenMax, const char *strSource);
char *strcat_safe(char * dst, int Size, const char * src);

int	wcsncpy_safe(WCHAR *strDestination, int nLenMax, const WCHAR *strSource, int nToCopy);
wchar_t *wcscpy_safe(wchar_t *strDestination, int nLenMax, const wchar_t *strSource);
WCHAR * __cdecl wcscat_safe (
							 WCHAR * dst,
							 int	Size,
							 const WCHAR * src
							 );


#ifdef _UNICODE
#define _tcscpy_safe		wcscpy_safe
#define _tcscat_safe		wcscat_safe
#else
#define _tcscpy_safe		strcpy_safe
#define _tcscat_safe		strcat_safe
#endif

int MbcsToUCS2(const char *str, int nLen, WCHAR *strOut, int nOut, int nCodePage = CP_ACP);

inline COLORREF AverageColor(COLORREF clr1, COLORREF clr2)
{
	return RGB((GetRValue(clr1) + GetRValue(clr2)) / 2, 
		(GetGValue(clr1) + GetGValue(clr2)) / 2, 
		(GetBValue(clr1) + GetBValue(clr2)) / 2);
}
