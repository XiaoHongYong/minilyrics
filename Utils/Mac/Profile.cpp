/********************************************************************
	Created  :	2002/01/04	21:30
	FileName :	Profile.cpp
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#include "../stdafx.h"
#include "Profile.h"
#include "UUEncode.h"
#include "App.h"

#include "../privateprofilestring.cpp"

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

BOOL WritePrivateProfileInt(
							LPCTSTR lpAppName,  // section name
							LPCTSTR lpKeyName,  // key name
							int nValue,			// int to add
							LPCTSTR lpFileName  // initialization file
							)
{
	TCHAR	szBuffer[16];

	_itot_s(nValue, szBuffer, CountOf(szBuffer), 10);
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
}

CProfile::~CProfile()
{
	Close();
}

//
// 区分大小写，将szAppName, szKey添加到缓存中
void CProfile::AddKeys(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szValue)
{
	LIST_APPSECT::iterator	itApp;
	APP_SECT				*appSec = NULL;
	tstring					strKeyNameLower;

	// find AppName
	for (itApp = m_listAppSect.begin(); itApp != m_listAppSect.end(); itApp++)
	{
		appSec = *itApp;
		if (_tcsicmp(appSec->strAppName.c_str(), szAppName) == 0)
			break;
	}
	if (appSec == NULL || itApp == m_listAppSect.end())
	{
		// append new app secton
		appSec = new APP_SECT;
		appSec->strAppName = szAppName;
		m_listAppSect.push_back(appSec);
	}

	strKeyNameLower = szKeyName;
	LowerString((LPTSTR)strKeyNameLower.c_str());
	appSec->mapKeys[strKeyNameLower] = szValue;
}

bool CProfile::GetKey(LPCTSTR szAppName, LPCTSTR szKeyName, MAP_STRING::iterator &it)
{
	LIST_APPSECT::iterator	itApp;
	APP_SECT				*appSec = NULL;
	tstring					strKeyNameLower;

	strKeyNameLower = szKeyName;
	LowerString((LPTSTR)strKeyNameLower.c_str());

	// 查找AppName
	for (itApp = m_listAppSect.begin(); itApp != m_listAppSect.end(); itApp++)
	{
		appSec = *itApp;
		if (_tcsicmp(appSec->strAppName.c_str(), szAppName) == 0)
			break;
	}
	if (appSec == NULL || itApp == m_listAppSect.end())
		return false;

	//
	// 查找KeyName
	it = appSec->mapKeys.find(strKeyNameLower);
	if (it == appSec->mapKeys.end())
		return false;

	return true;
}

void CProfile::SetKeyDefaultIfNotExist(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szValue)
{
	LIST_APPSECT::iterator	itApp;
	APP_SECT				*appSec = NULL;
	tstring					strKeyNameLower;
	MAP_STRING::iterator	it;

	strKeyNameLower = szKeyName;
	LowerString((LPTSTR)strKeyNameLower.c_str());

	// 查找AppName
	for (itApp = m_listAppSect.begin(); itApp != m_listAppSect.end(); itApp++)
	{
		appSec = *itApp;
		if (_tcsicmp(appSec->strAppName.c_str(), szAppName) == 0)
			break;
	}
	if (appSec == NULL || itApp == m_listAppSect.end())
	{
		// append new app secton
		appSec = new APP_SECT;
		appSec->strAppName = szAppName;
		m_listAppSect.push_back(appSec);
		appSec->mapKeys[strKeyNameLower] = szValue;
		return;
	}

	it = appSec->mapKeys.find(strKeyNameLower);
	if (it == appSec->mapKeys.end())
		appSec->mapKeys[strKeyNameLower] = szValue;
}

void CProfile::Init(LPCTSTR szProfile, LPCTSTR szDefKey)
{
	Close();

	if (szDefKey == NULL)
		m_strDefAppName = _T("");
	else
		m_strDefAppName = szDefKey;

	if (_tcslen(szProfile) > 3)
	{
		if (((szProfile[1] == ':') && (szProfile[2] == DIR_SLASH)) ||
			(szProfile[0] == DIR_SLASH))
		{
			// 如果已经是全路径，则不管
			m_strProfile = szProfile;
			return;
		}
	}

	m_strProfile = GetAppDataDir();
	m_strProfile += szProfile;
}

void CProfile::Close()
{
	m_strDefAppName = _T("");
	m_strProfile = _T("");

	LIST_APPSECT::iterator	itApp;
	APP_SECT				*appSec;

	// 查找AppName
	for (itApp = m_listAppSect.begin(); itApp != m_listAppSect.end(); itApp++)
	{
		appSec = *itApp;
		delete appSec;
	}
	m_listAppSect.clear();
}


void CProfile::DoCache()
{
	tstringex	str;
	LPCTSTR		szBeg, szEnd;
	tstring		strAppName, strKey, strValue;

	if (!OpenFileToTString(m_strProfile.c_str(), str))
		return;

	//
	// 查找 AppName
	szBeg = str.c_str();
	while (*szBeg)
	{
		while (*szBeg == ' ' || *szBeg == '\t' || *szBeg == '\n')
			szBeg++;

		if (*szBeg != '[')
		{
			while (*szBeg && *szBeg != '\n')
				szBeg++;
			continue;
		}
		szBeg++;

		while (*szBeg == ' ' || *szBeg == '\t')
			szBeg++;

		szEnd = szBeg;
		while (*szEnd && *szEnd != ']' && *szEnd != '\n')
			szEnd++;

		if (*szEnd != ']')
		{
			while (*szBeg && *szBeg != '\n')
				szBeg++;
			continue;
		}

		// found appname now
		strAppName = _T("");
		strAppName.append(szBeg, szEnd);
		TrimStr(strAppName);

		szBeg = szEnd + 1;

		while (*szBeg)
		{
			// get key and values
			while (*szBeg == ' ' || *szBeg == '\t')
				szBeg++;

			while (*szBeg == '\r' || *szBeg == '\n')
				szBeg++;

			if (*szBeg == '[')
				break;

			szEnd = szBeg;
			while (*szEnd && *szEnd != '=' && *szEnd != '\n')
				szEnd++;

			if (*szEnd != '=')
			{
				szBeg = szEnd;
				while (*szBeg && *szBeg != '\n')
					szBeg++;
				continue;
			}

			// found key
			strKey = _T("");
			strKey.append(szBeg, szEnd);

			szBeg = szEnd + 1;
			while (*szEnd && *szEnd != '\n')
				szEnd++;

			while (*szEnd == '\r' || *szEnd == '\n')
				szEnd--;
			if (*szEnd)
				szEnd++;
			strValue = _T("");
			strValue.append(szBeg, szEnd);

			AddKeys(strAppName.c_str(), strKey.c_str(), strValue.c_str());

			szBeg = szEnd;
		}
	}
}

void CProfile::SetDefaultIfNotExist(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szDefault)
{
	SetKeyDefaultIfNotExist(szAppName, szKeyName, szDefault);
}

void CProfile::SetDefaultIfNotExist(LPCTSTR szAppName, LPCTSTR szKeyName, int nDefault)
{
	TCHAR	szDefault[64];
	_itot_s(nDefault, szDefault, CountOf(szDefault), 10);

	SetKeyDefaultIfNotExist(szAppName, szKeyName, szDefault);
}

DWORD CProfile::GetProfileString(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szDefault, LPTSTR szReturnedString, DWORD nSize)
{
	assert(!m_strProfile.empty());

	DWORD		dwRet;

	if (szKeyName[0] == '\0')
	{
		EmptyStr(szReturnedString);
		return 0;
	}

	MAP_STRING::iterator	itKey;

	if (!GetKey(szAppName, szKeyName, itKey))
	{
		dwRet = GetPrivateProfileString(szAppName, szKeyName, szDefault, szReturnedString, nSize, m_strProfile.c_str());
		AddKeys(szAppName, szKeyName, szReturnedString);
		return dwRet;
	}
	else
	{
		_tcscpy_safe(szReturnedString, nSize, (*itKey).second.c_str());

		return (*itKey).second.size();
	}
}

LPCTSTR CProfile::GetProfileString(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szDefault)
{
	assert(!m_strProfile.empty());

	if (szKeyName[0] == '\0')
		return _T("");

	MAP_STRING::iterator	itKey;

	if (!GetKey(szAppName, szKeyName, itKey))
	{
		TCHAR	szReturnedString[1024];
		GetPrivateProfileString(szAppName, szKeyName, szDefault, szReturnedString, 1024, m_strProfile.c_str());
		AddKeys(szAppName, szKeyName, szReturnedString);
		if (!GetKey(szAppName, szKeyName, itKey))
		{
			assert(0 && "GetKey Can't be FAILED!");
			return _T("");
		}
	}

	return (*itKey).second.c_str();
}

BOOL CProfile::WriteProfileString(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szValue)
{
	assert(!m_strProfile.empty());

	DWORD		dwRet;
	MAP_STRING::iterator	itKey;

	dwRet = WritePrivateProfileString(szAppName, szKeyName, szValue, m_strProfile.c_str());

	if (!GetKey(szAppName, szKeyName, itKey))
	{
		AddKeys(szAppName, szKeyName, szValue);
	}
	else
		(*itKey).second = szValue;

	return dwRet;
}


UINT CProfile::GetProfileInt(LPCTSTR szAppName, LPCTSTR szKeyName, int nDefault)
{
	assert(!m_strProfile.empty());

	if (szKeyName[0] == '\0')
	{
		return nDefault;
	}

	TCHAR	szValue[64];
	TCHAR	szDefault[64];
	_itot_s(nDefault, szDefault, CountOf(szDefault), 10);

	GetProfileString(szAppName, szKeyName, szDefault, szValue, 64);

	return _ttoi(szValue);
}

BOOL CProfile::WriteProfileInt(LPCTSTR szAppName, LPCTSTR szKeyName, int nValue)
{
	assert(!m_strProfile.empty());

	TCHAR		szValue[64];

	_itot_s(nValue, szValue, CountOf(szValue), 10);

	return WriteProfileString(szAppName, szKeyName, szValue);
}


LPCTSTR CProfile::GetProfileName()
{
	return m_strProfile.c_str();
}

LPCTSTR CProfile::GetDefAppName()
{
	return m_strDefAppName.c_str();
}

//
// support encrypt keyname and value
//
void EncryptStr(LPCTSTR szStr, tstring &strOut)
{
	stringex	strUtf8;
	TStringToUTF8(szStr, -1, strUtf8);
	encodeuu(strUtf8.c_str(), strUtf8.size(), strOut, true, false);
	strrep(strOut, '=', '?');
}

void DecryptStr(LPCTSTR szStr, tstring &strOut)
{
	tstring		str = szStr;
	string		strUtf8;

	strrep(str, '?', '=');
	decodebase64(str.c_str(), str.size(), strUtf8);
	UTF8ToTString(strUtf8.c_str(), strUtf8.size(), strOut);
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

tstring CProfile::EncryptGetProfileString(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szDefault)
{
	assert(!m_strProfile.empty());

    TCHAR       szReturnedString[512];

	if (szKeyName[0] == '\0')
	{
		EmptyStr(szReturnedString);
		return 0;
	}

	tstring	strKeyNameEncrypt, strValue;
	EncryptStr(szKeyName, strKeyNameEncrypt);

	MAP_STRING::iterator	itKey;

	if (!GetKey(szAppName, strKeyNameEncrypt.c_str(), itKey))
	{
		EncryptStr(szDefault, strValue);
		GetPrivateProfileString(szAppName, strKeyNameEncrypt.c_str(), strValue.c_str(), szReturnedString, CountOf(szReturnedString), m_strProfile.c_str());
		AddKeys(szAppName, strKeyNameEncrypt.c_str(), szReturnedString);
	}
	else
	{
		_tcscpy_safe(szReturnedString, CountOf(szReturnedString), (*itKey).second.c_str());
	}
	DecryptStr(szReturnedString, strValue);
	_tcscpy_safe(szReturnedString, CountOf(szReturnedString), strValue.c_str());

	return szReturnedString;
}

BOOL CProfile::EncryptWriteProfileString(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szValue)
{
	assert(!m_strProfile.empty());

	DWORD		dwRet;
	MAP_STRING::iterator	itKey;
	tstring	strKeyNameEncrypt, strValueEncrypt;

	EncryptStr(szKeyName, strKeyNameEncrypt);
	EncryptStr(szValue, strValueEncrypt);

	dwRet = WritePrivateProfileString(szAppName, strKeyNameEncrypt.c_str(), strValueEncrypt.c_str(), m_strProfile.c_str());

	if (!GetKey(szAppName, strKeyNameEncrypt.c_str(), itKey))
	{
		AddKeys(szAppName, strKeyNameEncrypt.c_str(), szValue);
	}
	else
		(*itKey).second = strValueEncrypt;

	return dwRet;
}

int CProfile::EncryptGetProfileInt(LPCTSTR szKeyName, int nValue)
{
	tstring	str = EncryptGetProfileString(szKeyName, _T(""));
	if (str.empty())
		return nValue;
	else
		return _ttoi(str.c_str());
}

void CProfile::EncryptWriteProfileInt(LPCTSTR szKeyName, int nValue)
{
	EncryptWriteProfileString(szKeyName, CStrPrintf(_T("%d"), nValue).c_str());
}
