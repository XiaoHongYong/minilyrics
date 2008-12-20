/********************************************************************
	Created  :	2002/01/04	21:30
	FileName :	Profile.cpp
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#include "../stdafx.h"
#include "Profile.h"
#include "uuencode.h"
#include "app.h"
#include <stdio.h>

/*
Test Encrypt ....
int TestEncryptProfile()
{
	CProfile	file;
	file.Init(_T("c:\\a.ini"), NULL, _T("test"));

	TCHAR		szReturn[256];
	tstringex	str;

	// read k1
	file.EncryptGetProfileString(_T("k1"), _T("k1default"), szReturn, CountOf(szReturn));
	str = file.EncryptGetProfileString(_T("k1"), _T("k1default2"));

	// write k1
	file.EncryptWriteProfileString(_T("k1"), _T("k1value"));

	// read k1
	file.EncryptGetProfileString(_T("k1"), _T("k1default"), szReturn, CountOf(szReturn));
	str = file.EncryptGetProfileString(_T("k1"), _T("k1default2"));

	return 1;
}

int k = TestEncryptProfile();*/

DWORD GetPrivateProfileString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpDefault, LPTSTR lpReturnedString, DWORD nSize, LPCTSTR lpFileName)
{
	FILE		*fp;
	TCHAR		szBuff[1024];
	int			nAppName;
	int			nKeyName;
	BOOL		bFoundAppName = FALSE;
	int			n;

	fp = _tfopen(lpFileName, _T("r"));
	if (!fp)
		goto R_FAILED;

	nAppName = _tcslen(lpAppName);
	nKeyName = _tcslen(lpKeyName);

	//
	// 查找 AppName
	while (_fgetts(szBuff, CountOf(szBuff), fp))
	{
		LPCTSTR		szBeg = szBuff;
		while (*szBeg == ' ' || *szBeg == '\t')
			szBeg++;

		if (*szBeg != '[')
			continue;
		szBeg++;

		while (*szBeg == ' ' || *szBeg == '\t')
			szBeg++;

		if (_tcsnicmp(lpAppName, szBeg, nAppName) != 0)
			continue;
		szBeg += nAppName;

		while (*szBeg == ' ' || *szBeg == '\t')
			szBeg++;

		if (*szBeg != ']')
			continue;
		szBeg++;

		bFoundAppName = TRUE;

		break;
	}

	if (!bFoundAppName)
		goto R_FAILED;

	//
	// 查找 KeyName
	while (_fgetts(szBuff, CountOf(szBuff), fp))
	{
		LPCTSTR		szBeg = szBuff;
		while (*szBeg == ' ' || *szBeg == '\t')
			szBeg++;

		if (*szBeg == '[')
			goto R_FAILED;

		if (_tcsnicmp(lpKeyName, szBeg, nKeyName) != 0)
			continue;
		szBeg += nKeyName;

		while (*szBeg == ' ' || *szBeg == '\t')
			szBeg++;

		if (*szBeg != '=')
			continue;
		szBeg++;

		while (*szBeg == ' ' || *szBeg == '\t')
			szBeg++;

		_tcscpy_safe(lpReturnedString, nSize, szBeg);

		n = _tcslen(lpReturnedString);

		while (n > 0 && (lpReturnedString[n - 1] == '\r' || lpReturnedString[n - 1] == '\n'))
			n--;

		lpReturnedString[n] = '\0';

		if (fp)
			fclose(fp);

		return n;
	}

R_FAILED:
	if (fp)
		fclose(fp);

	_tcscpy_safe(lpReturnedString, nSize, lpDefault);

	return _tcslen(lpReturnedString);
}

BOOL ReplacePrivateProfileStr(CCharBuffer	&buff, const char *lpAppName, const char *lpKeyName, const char *lpString)
{
	const char	*szBeg;
	int			nAppName;
	int			nKeyName;
	BOOL		bFoundAppName = FALSE;
	char	szBuff[512];

	nAppName = strlen(lpAppName);
	nKeyName = strlen(lpKeyName);

	szBeg = buff.c_str();

	for (szBeg = buff.c_str(); szBeg != NULL; szBeg = strchr(szBeg, '\n'))
	{
		while (*szBeg == '\r' || *szBeg == '\n')
			szBeg++;

		while (*szBeg == ' ' || *szBeg == '\t')
			szBeg++;

		if (*szBeg != '[')
			continue;
		szBeg++;

		if (_strnicmp(lpAppName, szBeg, nAppName) != 0)
			continue;
		szBeg += nAppName;

		while (*szBeg == ' ' || *szBeg == '\t')
			szBeg++;

		if (*szBeg != ']')
			continue;
		szBeg++;

		bFoundAppName = TRUE;

		break;
	}

	if (!bFoundAppName)
	{
		if (buff.size())
		{
			if (buff[int(buff.size() -1)] != '\n')
			{
				buff.append("\r\n", 2);
			}
		}
		// Append at end.
		sprintf(szBuff, "[%s]\r\n", lpAppName);
		buff += szBuff;
		sprintf(szBuff, "%s=%s\r\n", lpKeyName, lpString);
		buff += szBuff;

		return TRUE;
	}

	//
	// 查找 KeyName
	for (; szBeg != NULL; szBeg = strchr(szBeg, '\n'))
	{
		szBeg = strchr(szBeg, '\n');
		while (*szBeg == '\r' || *szBeg == '\n')
			szBeg++;

		const char*		szInsert = szBeg;
		while (*szBeg == ' ' || *szBeg == '\t')
			szBeg++;

		if (*szBeg == '[')
		{
			sprintf(szBuff, "%s=%s\r\n", lpKeyName, lpString);
			buff.insert(int(szInsert - buff.c_str()), szBuff, strlen(szBuff));
			return TRUE;
		}

		if (_strnicmp(lpKeyName, szBeg, nKeyName) != 0)
			continue;
		szBeg += nKeyName;

		while (*szBeg == ' ' || *szBeg == '\t')
			szBeg++;

		if (*szBeg != '=')
			continue;
		szBeg++;
		szInsert = szBeg;

		while (*szBeg && *szBeg != '\r' && *szBeg != '\n')
			szBeg++;

		buff.replace(int(szInsert - buff.c_str()), int(szBeg - szInsert), lpString, strlen(lpString));

		return TRUE;
	}

	return TRUE;
}

BOOL WritePrivateProfileString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpString, LPCTSTR lpFileName)
{
	CCharBuffer		buff;

	if (IsFileExist(lpFileName) && !OpenFileToString(lpFileName, buff))
		return FALSE;

	stringex	strAppName, strKeyname, strString;

	ConvertStr(lpAppName, -1, strAppName);
	ConvertStr(lpKeyName, -1, strKeyname);
	ConvertStr(lpString, -1, strString);

	ReplacePrivateProfileStr(buff, strAppName.c_str(), strKeyname.c_str(), strString.c_str());

	return SaveDataAsFile(lpFileName, buff.c_str(), buff.size());
}

BOOL WritePrivateProfileInt(
							LPCTSTR lpAppName,  // section name
							LPCTSTR lpKeyName,  // key name
							int nValue,			// int to add
							LPCTSTR lpFileName  // initialization file
							)
{
	TCHAR	szBuffer[16];

	_stprintf(szBuffer, _T("%d"), nValue);
	return WritePrivateProfileString(lpAppName, lpKeyName, szBuffer, lpFileName);
}

//DWORD RegGetProfileString(LPCTSTR szKeyName, LPCTSTR szDefault, LPTSTR szReturnedString, DWORD nSize)
//{
//	GetPrivateProfileString(_T("Registry"), szKeyName, szDefault, szReturnedString, nSize);
//}
// 
// DWORD RegGetProfileString(HKEY hRoot, LPCTSTR szItem, LPCTSTR szKeyName, LPCTSTR szDefault, LPTSTR szReturnedString, DWORD nSize)
// {
// 	HKEY	hKey;
// 
// 	EmptyStr(szReturnedString);
// 
// 	if (RegOpenKeyEx(hRoot, szItem,
// 		0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
// 	{
// 		DWORD		nSizeRet = nSize;
// 		DWORD		dwType = REG_SZ;
// 		if (RegQueryValueEx(hKey, szKeyName, NULL, &dwType, (BYTE *)szReturnedString, &nSizeRet) != ERROR_SUCCESS)
// 		{
// 			// LOG1(LOG_LVL_ERROR, _T("RegQueryValueEx: %s, FAILED!"), szKeyName);
// 		}
// 		RegCloseKey(hKey);
// 	}
// 
// 	return _tcslen(szReturnedString);
// }
// 
// BOOL RegWriteProfileString(HKEY hRoot, LPCTSTR szItem, LPCTSTR szKeyName, LPCTSTR szValue)
// {
// 	HKEY	hKey;
// 	DWORD	dwDispositon;
// 
// 	if (RegCreateKeyEx(hRoot, szItem,
// 		0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKey, &dwDispositon) == ERROR_SUCCESS)
// 	{
// 		DWORD		dwType = REG_SZ;
// 		if (RegSetValueEx(hKey, szKeyName, 0, dwType, (BYTE *)szValue, lstrlen(szValue) * sizeof(TCHAR)) != ERROR_SUCCESS)
// 		{
// 			// LOG1(LOG_LVL_ERROR, _T("RegQueryValueEx: %s, FAILED!"), szKeyName);
// 		}
// 		RegCloseKey(hKey);
// 	}
// 
// 	return lstrlen(szKeyName);
// }
// 
// BOOL RegWriteProfileString(LPCTSTR szKeyName, LPCTSTR szValue)
// {
// 	HKEY	hKey;
// 	DWORD	dwDispositon;
// 
// 	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, _T("Software\\MiniLyrics"),
// 		0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKey, &dwDispositon) == ERROR_SUCCESS)
// 	{
// 		DWORD		dwType = REG_SZ;
// 		if (RegSetValueEx(hKey, szKeyName, 0, dwType, (BYTE *)szValue, _tcslen(szValue)) != ERROR_SUCCESS)
// 		{
// 			// LOG1(LOG_LVL_ERROR, _T("RegQueryValueEx: %s, FAILED!"), szKeyName);
// 		}
// 		RegCloseKey(hKey);
// 	}
// 
// 	return _tcslen(szKeyName);
// }
// 
// int RegGetProfileInt(HKEY hRoot, LPCTSTR szSubKey, LPCTSTR szKeyName, int nDefault)
// {
// 	HKEY	hKey;
// 	int		nValue = nDefault;
// 
// 	if (RegOpenKeyEx(hRoot, szSubKey,
// 		0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
// 	{
// 		DWORD		nSizeRet = sizeof(nDefault);
// 		DWORD		dwType = REG_DWORD;
// 		if (RegQueryValueEx(hKey, szKeyName, NULL, &dwType, (BYTE *)&nValue, &nSizeRet) != ERROR_SUCCESS)
// 		{
// 			// LOG1(LOG_LVL_ERROR, _T("RegQueryValueEx: %s, FAILED!"), szKeyName);
// 		}
// 		RegCloseKey(hKey);
// 	}
// 
// 	return nValue;
// }
// 
// BOOL RegWriteProfileInt(HKEY hRoot, LPCTSTR szSubKey, LPCTSTR szKeyName, int nValue)
// {
// 	HKEY	hKey;
// 	DWORD	dwDispositon;
// 	BOOL	bRet = FALSE;
// 
// 	if (RegCreateKeyEx(hRoot, szSubKey,
// 		0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKey, &dwDispositon) == ERROR_SUCCESS)
// 	{
// 		DWORD		dwType = REG_DWORD;
// 		if (RegSetValueEx(hKey, szKeyName, 0, dwType, (BYTE *)&nValue, sizeof(nValue)) == ERROR_SUCCESS)
// 		{
// 			bRet = TRUE;
// 		}
// 		RegCloseKey(hKey);
// 	}
// 
// 	return bRet;
// }

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProfile::CProfile()
{
	m_szProfile[0] = '\0';
}

CProfile::~CProfile()
{
}

bool CProfile::GetKey(LPCTSTR szAppName, LPCTSTR szKeyName, STRING_MAP::iterator &it)
{
	APPNAME_MAP::iterator	itApp;

	itApp = m_mapAppName2Keys.find(szAppName);
	if (itApp == m_mapAppName2Keys.end())
	{
		//
		// 不区分大小写，查找AppName
		for (itApp = m_mapAppName2Keys.begin(); itApp != m_mapAppName2Keys.end(); itApp++)
		{
			if (_tcsicmp((*itApp).first.c_str(), szAppName) == 0)
				break;
		}

		if (itApp == m_mapAppName2Keys.end())
			return false;
	}

	it = (*itApp).second.find(szKeyName);
	if (it == (*itApp).second.end())
	{
		//
		// 不区分大小写，查找KeyName
		for (it = (*itApp).second.begin(); it != (*itApp).second.end(); it++)
		{
			if (_tcsicmp((*it).first.c_str(), szKeyName) == 0)
				break;
		}
		if (it == (*itApp).second.end())
			return false;
	}
	return true;
}

DWORD CProfile::GetProfileString(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szDefault, LPTSTR szReturnedString, DWORD nSize)
{
	assert(m_szProfile[0] != '\0');

	DWORD		dwRet;

	if (szKeyName[0] == '\0')
	{
		EmptyStr(szReturnedString);
		return 0;
	}

	STRING_MAP::iterator	itStr;

	if (!GetKey(szAppName, szKeyName, itStr))
	{
		dwRet = GetPrivateProfileString(szAppName, szKeyName, szDefault, szReturnedString, nSize, m_szProfile);
		AddKeys(szAppName, szKeyName, szReturnedString);
		return dwRet;
	}
	else
	{
		_tcscpy(szReturnedString, (*itStr).second.c_str());

		return (*itStr).second.size();
	}
}

LPCTSTR CProfile::GetProfileString(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szDefault)
{
	assert(m_szProfile[0] != '\0');

	DWORD		dwRet;

	if (szKeyName[0] == '\0')
		return _T("");

	STRING_MAP::iterator	itStr;

	if (!GetKey(szAppName, szKeyName, itStr))
	{
		TCHAR	szReturnedString[1024];
		dwRet = GetPrivateProfileString(szAppName, szKeyName, szDefault, szReturnedString, 1024, m_szProfile);
		AddKeys(szAppName, szKeyName, szReturnedString);
		if (!GetKey(szAppName, szKeyName, itStr))
		{
			assert(0 && "GetKey Can't be FAILED!");
			return _T("");
		}
	}

	return (*itStr).second.c_str();
}

BOOL CProfile::WriteProfileString(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szValue)
{
	assert(m_szProfile[0] != '\0');

	DWORD		dwRet;
	STRING_MAP::iterator	itStr;

	dwRet = WritePrivateProfileString(szAppName, szKeyName, szValue, m_szProfile);

	if (!GetKey(szAppName, szKeyName, itStr))
	{
		AddKeys(szAppName, szKeyName, szValue);
	}
	else
		(*itStr).second = szValue;

	return dwRet;
}


UINT CProfile::GetProfileInt(LPCTSTR szAppName, LPCTSTR szKeyName, int nDefault)
{
	assert(m_szProfile[0] != '\0');

	if (szKeyName[0] == '\0')
	{
		return nDefault;
	}

	TCHAR	szValue[64];
	TCHAR	szDefault[64];
	_stprintf(szDefault, _T("%d"), nDefault);

	GetProfileString(szAppName, szKeyName, szDefault, szValue, 64);

	return _ttoi(szValue);
}

BOOL CProfile::WriteProfileInt(LPCTSTR szAppName, LPCTSTR szKeyName, int nValue)
{
	assert(m_szProfile[0] != '\0');

	TCHAR		szValue[MAX_PATH];
	
	_stprintf(szValue, _T("%d"), nValue);

	return WriteProfileString(szAppName, szKeyName, szValue);
}


LPCTSTR CProfile::GetProfileName()
{
	return m_szProfile;
}

LPCTSTR CProfile::GetDefAppName()
{
	return m_strDefAppName.c_str();
}

void CProfile::Init(LPCTSTR szProfile, LPCTSTR szDefKey)
{
	m_mapAppName2Keys.clear();

	if (szDefKey == NULL)
		m_strDefAppName = _T("");
	else
		m_strDefAppName = szDefKey;

	if (_tcslen(szProfile) > 3)
	{
		if ((szProfile[1] == ':') && (szProfile[2] == DIR_SLASH) ||
			(szProfile[0] == DIR_SLASH))
		{
			// 如果已经是全路径，则不管
			_tcscpy(m_szProfile, szProfile);
			return;
		}
	}

	GetWorkingFolder(m_szProfile);
	_tcscat(m_szProfile, szProfile);
}

void CProfile::Close()
{
	m_mapAppName2Keys.clear();
	m_strDefAppName = _T("");
	m_szProfile[0] = '\0';
}

//
// 区分大小写，将szAppName, szKey添加到缓存中
void CProfile::AddKeys(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szValue)
{
	APPNAME_MAP::iterator	it;
	STRING_MAP				mapStr;

	it = m_mapAppName2Keys.find(szAppName);
	if (it == m_mapAppName2Keys.end())
	{
		m_mapAppName2Keys[szAppName] = mapStr;
	}

	m_mapAppName2Keys[szAppName][szKeyName] = szValue;
}

template<class XCHAR>
void charep(XCHAR *str, XCHAR Src, XCHAR Targ)
{
	while (*str)
	{
		if (*str == Src)
			*str = Targ;
		str++;
	}
}

//
// support encrypt keyname and value
//
void EncryptStr(LPCTSTR szStr, tstringex &strOut)
{
	stringex	strEncrypt;
	stringex	strAnsi;
	ConvertStr(szStr, -1, strAnsi);
	encodeuu(strAnsi.c_str(), strAnsi.size(), strEncrypt, true, false);
	charep(strEncrypt.data(), '=', '?');
	ConvertStr(strEncrypt.c_str(), strEncrypt.size(), strOut);
}

void DecryptStr(LPCTSTR szStr, tstringex &strOut)
{
	stringex	strDecrypt;
	stringex	strAnsi;
	ConvertStr(szStr, -1, strAnsi);
	charep(strAnsi.data(), '?', '=');
	decodebase64(strAnsi.c_str(), strAnsi.size(), strDecrypt);
	ConvertStr(strDecrypt.c_str(), strDecrypt.size(), strOut);
}
/*
void TestEnDecrypt()
{
	TCHAR		szString[] = _T("012345adjkl;ge;lk561./zx/,.ckl");
	tstringex	str, strOut, strOut2;

	for (int i = 0; i < 20; i++)
	{
		str.clear();
		str.append(szString, i);
		EncryptStr(str.c_str(), strOut);
		DecryptStr(strOut.c_str(), strOut2);
		assert(_tcscmp(str.c_str(), strOut2.c_str()) == 0);
	}
}
*/

DWORD CProfile::EncryptGetProfileString(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szDefault, LPTSTR szReturnedString, DWORD nSize)
{
	assert(m_szProfile[0] != '\0');

	DWORD		dwRet;

	if (szKeyName[0] == '\0')
	{
		EmptyStr(szReturnedString);
		return 0;
	}

	tstringex	strKeyNameEncrypt, strValue;
	EncryptStr(szKeyName, strKeyNameEncrypt);

	STRING_MAP::iterator	itStr;

	if (!GetKey(szAppName, strKeyNameEncrypt.c_str(), itStr))
	{
		EncryptStr(szDefault, strValue);
		dwRet = GetPrivateProfileString(szAppName, strKeyNameEncrypt.c_str(), strValue.c_str(), szReturnedString, nSize, m_szProfile);
		DecryptStr(szReturnedString, strValue);
		AddKeys(szAppName, strKeyNameEncrypt.c_str(), strValue.c_str());
		_tcscpy_safe(szReturnedString, nSize, strValue.c_str());
	}
	else
	{
		_tcscpy_safe(szReturnedString, nSize, (*itStr).second.c_str());
	}

	return _tcslen(szReturnedString);
}

LPCTSTR CProfile::EncryptGetProfileString(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szDefault)
{
	assert(m_szProfile[0] != '\0');

	DWORD		dwRet;

	if (szKeyName[0] == '\0')
		return _T("");

	STRING_MAP::iterator	itStr;

	tstringex	strKeyNameEncrypt, strValue;
	EncryptStr(szKeyName, strKeyNameEncrypt);

	if (!GetKey(szAppName, strKeyNameEncrypt.c_str(), itStr))
	{
		TCHAR	szReturnedString[1024];
		EncryptStr(szDefault, strValue);
		dwRet = GetPrivateProfileString(szAppName, strKeyNameEncrypt.c_str(), strValue.c_str(), szReturnedString, CountOf(szReturnedString), m_szProfile);
		DecryptStr(szReturnedString, strValue);
		AddKeys(szAppName, strKeyNameEncrypt.c_str(), strValue.c_str());
		if (!GetKey(szAppName, strKeyNameEncrypt.c_str(), itStr))
			return _T("");
	}

	return (*itStr).second.c_str();
}

BOOL CProfile::EncryptWriteProfileString(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szValue)
{
	assert(m_szProfile[0] != '\0');

	DWORD		dwRet;
	STRING_MAP::iterator	itStr;
	tstringex	strKeyNameEncrypt, strValueEncrypt;

	EncryptStr(szKeyName, strKeyNameEncrypt);
	EncryptStr(szValue, strValueEncrypt);

	dwRet = WritePrivateProfileString(szAppName, strKeyNameEncrypt.c_str(), strValueEncrypt.c_str(), m_szProfile);

	if (!GetKey(szAppName, strKeyNameEncrypt.c_str(), itStr))
	{
		AddKeys(szAppName, strKeyNameEncrypt.c_str(), szValue);
	}
	else
		(*itStr).second = szValue;

	return dwRet;
}
