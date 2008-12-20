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

#include "PrivateProfileString.cpp"

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
}

CProfile::~CProfile()
{
	Close();
}

//
// 区分大小写，将szAppName, szKey添加到缓存中
void CProfile::AddKeys(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szValue)
{
	LIST_KEYS				mapStr;
	LIST_APPSECT::iterator	itApp;
	APP_SECT				*appSec;

	// find AppName
	for (itApp = m_listAppSect.begin(); itApp != m_listAppSect.end(); itApp++)
	{
		appSec = *itApp;
		if (_tcsicmp(appSec->strAppName.c_str(), szAppName) == 0)
			break;
	}
	if (itApp == m_listAppSect.end())
	{
		// append new app secton
		appSec = new APP_SECT;
		appSec->strAppName = szAppName;
		m_listAppSect.push_back(appSec);
	}
	else
		appSec = *itApp;

	appSec->listKeysUsed.push_back(KEY(szKeyName, szValue));
}

bool CProfile::GetKey(LPCTSTR szAppName, LPCTSTR szKeyName, LIST_KEYS::iterator &it)
{
	LIST_APPSECT::iterator	itApp;
	APP_SECT				*appSec;

	// 查找AppName
	for (itApp = m_listAppSect.begin(); itApp != m_listAppSect.end(); itApp++)
	{
		appSec = *itApp;
		if (_tcsicmp(appSec->strAppName.c_str(), szAppName) == 0)
			break;
	}
	if (itApp == m_listAppSect.end())
		return false;

	//
	// 不区分大小写，查找KeyName
	for (it = appSec->listKeysUsed.begin(); it != appSec->listKeysUsed.end(); it++)
	{
		KEY	&key = *it;
		if (_tcsicmp(key.strKeyName.c_str(), szKeyName) == 0)
			break;
	}
	if (it == appSec->listKeysUsed.end())
		return false;

	return true;
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
		if ((szProfile[1] == ':') && (szProfile[2] == DIR_SLASH) ||
			(szProfile[0] == DIR_SLASH))
		{
			// 如果已经是全路径，则不管
			m_strProfile = szProfile;
			return;
		}
	}

	m_strProfile = GetWorkingFolder();
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

DWORD CProfile::GetProfileString(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szDefault, LPTSTR szReturnedString, DWORD nSize)
{
	assert(!m_strProfile.empty());

	DWORD		dwRet;

	if (szKeyName[0] == '\0')
	{
		EmptyStr(szReturnedString);
		return 0;
	}

	LIST_KEYS::iterator	itKey;

	if (!GetKey(szAppName, szKeyName, itKey))
	{
		dwRet = GetPrivateProfileString(szAppName, szKeyName, szDefault, szReturnedString, nSize, m_strProfile.c_str());
		AddKeys(szAppName, szKeyName, szReturnedString);
		return dwRet;
	}
	else
	{
		_tcscpy(szReturnedString, (*itKey).strValue.c_str());

		return (*itKey).strValue.size();
	}
}

LPCTSTR CProfile::GetProfileString(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szDefault)
{
	assert(!m_strProfile.empty());

	DWORD		dwRet;

	if (szKeyName[0] == '\0')
		return _T("");

	LIST_KEYS::iterator	itKey;

	if (!GetKey(szAppName, szKeyName, itKey))
	{
		TCHAR	szReturnedString[1024];
		dwRet = GetPrivateProfileString(szAppName, szKeyName, szDefault, szReturnedString, 1024, m_strProfile.c_str());
		AddKeys(szAppName, szKeyName, szReturnedString);
		if (!GetKey(szAppName, szKeyName, itKey))
		{
			assert(0 && "GetKey Can't be FAILED!");
			return _T("");
		}
	}

	return (*itKey).strValue.c_str();
}

BOOL CProfile::WriteProfileString(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szValue)
{
	assert(!m_strProfile.empty());

	DWORD		dwRet;
	LIST_KEYS::iterator	itKey;

	dwRet = WritePrivateProfileString(szAppName, szKeyName, szValue, m_strProfile.c_str());

	if (!GetKey(szAppName, szKeyName, itKey))
	{
		AddKeys(szAppName, szKeyName, szValue);
	}
	else
		(*itKey).strValue = szValue;

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
	_stprintf(szDefault, _T("%d"), nDefault);

	GetProfileString(szAppName, szKeyName, szDefault, szValue, 64);

	return _ttoi(szValue);
}

BOOL CProfile::WriteProfileInt(LPCTSTR szAppName, LPCTSTR szKeyName, int nValue)
{
	assert(!m_strProfile.empty());

	TCHAR		szValue[MAX_PATH];
	
	_stprintf(szValue, _T("%d"), nValue);

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
	assert(!m_strProfile.empty());

	DWORD		dwRet;

	if (szKeyName[0] == '\0')
	{
		EmptyStr(szReturnedString);
		return 0;
	}

	tstringex	strKeyNameEncrypt, strValue;
	EncryptStr(szKeyName, strKeyNameEncrypt);

	LIST_KEYS::iterator	itKey;

	if (!GetKey(szAppName, strKeyNameEncrypt.c_str(), itKey))
	{
		EncryptStr(szDefault, strValue);
		dwRet = GetPrivateProfileString(szAppName, strKeyNameEncrypt.c_str(), strValue.c_str(), szReturnedString, nSize, m_strProfile.c_str());
		DecryptStr(szReturnedString, strValue);
		AddKeys(szAppName, strKeyNameEncrypt.c_str(), strValue.c_str());
		_tcscpy_safe(szReturnedString, nSize, strValue.c_str());
	}
	else
	{
		_tcscpy_safe(szReturnedString, nSize, (*itKey).strValue.c_str());
	}

	return _tcslen(szReturnedString);
}

LPCTSTR CProfile::EncryptGetProfileString(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szDefault)
{
	assert(!m_strProfile.empty());

	DWORD		dwRet;

	if (szKeyName[0] == '\0')
		return _T("");

	LIST_KEYS::iterator	itKey;

	tstringex	strKeyNameEncrypt, strValue;
	EncryptStr(szKeyName, strKeyNameEncrypt);

	if (!GetKey(szAppName, strKeyNameEncrypt.c_str(), itKey))
	{
		TCHAR	szReturnedString[1024];
		EncryptStr(szDefault, strValue);
		dwRet = GetPrivateProfileString(szAppName, strKeyNameEncrypt.c_str(), strValue.c_str(), szReturnedString, CountOf(szReturnedString), m_strProfile.c_str());
		DecryptStr(szReturnedString, strValue);
		AddKeys(szAppName, strKeyNameEncrypt.c_str(), strValue.c_str());
		if (!GetKey(szAppName, strKeyNameEncrypt.c_str(), itKey))
			return _T("");
	}

	return (*itKey).strValue.c_str();
}

BOOL CProfile::EncryptWriteProfileString(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szValue)
{
	assert(!m_strProfile.empty());

	DWORD		dwRet;
	LIST_KEYS::iterator	itKey;
	tstringex	strKeyNameEncrypt, strValueEncrypt;

	EncryptStr(szKeyName, strKeyNameEncrypt);
	EncryptStr(szValue, strValueEncrypt);

	dwRet = WritePrivateProfileString(szAppName, strKeyNameEncrypt.c_str(), strValueEncrypt.c_str(), m_strProfile.c_str());

	if (!GetKey(szAppName, strKeyNameEncrypt.c_str(), itKey))
	{
		AddKeys(szAppName, strKeyNameEncrypt.c_str(), szValue);
	}
	else
		(*itKey).strValue = szValue;

	return dwRet;
}
