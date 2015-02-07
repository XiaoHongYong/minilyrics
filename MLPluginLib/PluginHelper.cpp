#include "StdAfx.h"
#include "PluginHelper.h"


int MbcsToWideChar(const char *str, int nLen, WCHAR *strOut, int nOut, int nCodePage)
{
	int		n;

	n = MultiByteToWideChar(nCodePage, 0,
		str, nLen, strOut, nOut);
	if (n == 0 && GetLastError() == ERROR_INVALID_PARAMETER)
	{
		n = MultiByteToWideChar(CP_ACP, 0,
			str, nLen, strOut, nOut);
	}

	if (n < nOut)
		strOut[n] = '\0';

	return n;
}

int Utf8ToUCS2(const char* text, int length, WCHAR *ucs2, int ubuflen)
{
	return MbcsToWideChar(text, length, ucs2, ubuflen, CP_UTF8);
}

BOOL GetProgramPath(TCHAR szPath[], HINSTANCE hInstance)
{
	__try
	{
		int nLen;

		nLen = GetModuleFileName(hInstance, szPath, MAX_PATH);
		nLen --;
		while (nLen >= 0 && szPath[nLen] != '\\')
			nLen --;
		if (nLen < 0)
			return FALSE;

		szPath[nLen + 1] = '\0';
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL WritePrivateProfileInt(
							LPCTSTR lpAppName,  // section name
							LPCTSTR lpKeyName,  // key name
							int nValue,			// int to add
							LPCTSTR lpFileName  // initialization file
							)
{
	TCHAR	szBuffer[16];

	wsprintf(szBuffer, _T("%d"), nValue);
	return WritePrivateProfileString(lpAppName, lpKeyName, szBuffer, lpFileName);
}

int RegGetProfileInt(HKEY hRoot, LPCTSTR szSubKey, LPCTSTR szKeyName, int nDefault)
{
	HKEY	hKey;
	int		nValue = nDefault;

	if (RegOpenKeyEx(hRoot, szSubKey,
		0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		DWORD		nSizeRet = sizeof(nDefault);
		DWORD		dwType = REG_DWORD;
		if (RegQueryValueEx(hKey, szKeyName, NULL, &dwType, (BYTE *)&nValue, &nSizeRet) != ERROR_SUCCESS)
		{
			// LOG1(LOG_LVL_ERROR, _T("RegQueryValueEx: %s, FAILED!"), szKeyName);
		}
		RegCloseKey(hKey);
	}

	return nValue;
}

BOOL RegWriteProfileInt(HKEY hRoot, LPCTSTR szSubKey, LPCTSTR szKeyName, int nValue)
{
	HKEY	hKey;
	DWORD	dwDispositon;
	BOOL	bRet = FALSE;

	if (RegCreateKeyEx(hRoot, szSubKey,
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKey, &dwDispositon) == ERROR_SUCCESS)
	{
		DWORD		dwType = REG_DWORD;
		if (RegSetValueEx(hKey, szKeyName, 0, dwType, (BYTE *)&nValue, sizeof(nValue)) == ERROR_SUCCESS)
		{
			bRet = TRUE;
		}
		RegCloseKey(hKey);
	}

	return bRet;
}

#define SZ_REG_MINILYRICS		"SOFTWARE\\MiniLyrics"

char *strcpy_safe(char *strDestination, size_t nLenMax, const char *strSource)
{
	char	*cp = strDestination;
	char	*cpEnd = strDestination + nLenMax - 1;

	for (; cp < cpEnd && *strSource; cp++, strSource++)
	{
		*cp = *strSource;
	}

	*cp = '\0';

	return(strDestination);
}

WCHAR *strcpy_safe(WCHAR *strDestination, size_t nLenMax, const WCHAR *strSource)
{
	WCHAR	*cp = strDestination;
	WCHAR	*cpEnd = strDestination + nLenMax - 1;

	for (; cp < cpEnd && *strSource; cp++, strSource++)
	{
		*cp = *strSource;
	}

	*cp = '\0';

	return(strDestination);
}

TCHAR *strcat_safe(TCHAR * dst, size_t Size, const TCHAR * src)
{
	TCHAR * cp = dst;
	TCHAR * endcp = dst + Size - 1;

	while( *cp )
		cp++;                   /* find end of dst */

	while((cp < endcp) && (*cp++ = *src++)) ;       /* Copy src to end of dst */

	*cp = '\0';

	return( dst );                  /* return dst */
}


size_t	wcsncpy_safe(WCHAR *strDestination, size_t nLenMax, const WCHAR *strSource, size_t nToCopy)
{
	if (nToCopy >= nLenMax)
		nToCopy = nLenMax - 1;

	size_t		org = nToCopy;

	while (nToCopy && (*strDestination++ = *strSource++))    /* copy string */
		nToCopy--;

	*strDestination = '\0';

	return org - nToCopy;
}

wchar_t *wcscpy_safe(wchar_t *strDestination, size_t nLenMax, const wchar_t *strSource)
{
	wchar_t	*cp = strDestination;
	wchar_t	*cpEnd = strDestination + nLenMax - 1;

	for (; cp < cpEnd && *strSource; cp++, strSource++)
	{
		*cp = *strSource;
	}

	*cp = '\0';

	return(strDestination);
}

WCHAR * __cdecl wcscat_safe (
							 WCHAR * dst,
							 size_t	Size,
							 const WCHAR * src
							 )
{
	WCHAR * cp = dst;
	WCHAR * endcp = dst + Size - 1;

	while( *cp )
		cp++;                   /* find end of dst */

	while((cp < endcp) && (*cp++ = *src++)) ;       /* Copy src to end of dst */

	*cp = '\0';

	return( dst );                  /* return dst */

}

string UrlGetTitle(LPCSTR szFile)
{
	LPCSTR		szBeg, szBeg2, szEnd;
	int			nLen;
	string		strFileTitle;

	szEnd = strrchr(szFile, '.');

	szBeg = strrchr(szFile, '/');
	szBeg2 = strrchr(szFile, '\\');
	if (szBeg2 > szBeg)
		szBeg = szBeg2;

	if (szBeg == NULL)
		szBeg = szFile;
	else
		szBeg++;

	if (szEnd == NULL || szBeg > szEnd)
		nLen = (int)(szFile - szBeg + strlen(szFile));
	else
		nLen = (int)(szEnd - szBeg);

	strFileTitle.append(szBeg, nLen);

	return strFileTitle;
}

wstring UrlGetTitle(LPCWSTR szFile)
{
	LPCWSTR		szBeg, szBeg2, szEnd;
	int			nLen;
	wstring		strFileTitle;

	szEnd = wcsrchr(szFile, '.');

	szBeg = wcsrchr(szFile, '/');
	szBeg2 = wcsrchr(szFile, '\\');
	if (szBeg2 > szBeg)
		szBeg = szBeg2;

	if (szBeg == NULL)
		szBeg = szFile;
	else
		szBeg++;

	if (szEnd == NULL || szBeg > szEnd)
		nLen = (int)(szFile - szBeg + wcslen(szFile));
	else
		nLen = (int)(szEnd - szBeg);

	strFileTitle.append(szBeg, nLen);

	return strFileTitle;
}

LPCSTR UrlGetName(LPCSTR szFile)
{
	LPCSTR		szBeg, szBeg2;

	szBeg = strrchr(szFile, '/');
	szBeg2 = strrchr(szFile, '\\');
	if (szBeg2 > szBeg)
		szBeg = szBeg2;
	if (szBeg)
		return szBeg + 1;
	else
		return szFile;
}

bool IsFileExist(LPCTSTR szFileName)
{
	return (GetFileAttributes(szFileName) != -1);
}

void DirStringFormat(LPTSTR	szDir)
{
	size_t		nLen;

	nLen = _tcslen(szDir);
	if (nLen > 0)
	{
		if (szDir[nLen - 1] != '\\')
		{
			szDir[nLen] = '\\';
			szDir[nLen + 1] = '\0';
		}
	}
}

