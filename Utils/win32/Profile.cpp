/********************************************************************
	Created  :	2002/01/04	21:30
	FileName :	Profile.cpp
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#include "stdafx.h"
#include "Profile.h"
#include "stdio.h"
#include "uuencode.h"
#include "app.h"

#ifdef _WINCE_

#include "PrivateProfileString.cpp"

#endif
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

DWORD RegGetProfileString(HKEY hRoot, LPCTSTR szItem, LPCTSTR szKeyName, LPCTSTR szDefault, LPTSTR szReturnedString, DWORD nSize)
{
	HKEY	hKey;

	EmptyStr(szReturnedString);

	if (RegOpenKeyEx(hRoot, szItem,
		0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		DWORD		nSizeRet = nSize * sizeof(TCHAR);
		DWORD		dwType = REG_SZ;
		if (RegQueryValueEx(hKey, szKeyName, NULL, &dwType, (BYTE *)szReturnedString, &nSizeRet) != ERROR_SUCCESS)
		{
			_tcscpy_safe(szReturnedString,nSize, szDefault);
		}
		else
		{
			nSizeRet /= sizeof(TCHAR);
			if (nSizeRet >= nSize)
				nSizeRet = nSize - 1;
			szReturnedString[nSizeRet] = '\0';
			if (dwType != REG_SZ)
			{
				if (dwType == REG_DWORD)
				{
					_itot_s(RegGetProfileInt(hRoot, szItem, szKeyName, _ttoi(szDefault)),
						szReturnedString, nSize, 10);
				}
				else
					ASSERT(0);
			}
		}
		RegCloseKey(hKey);
	}
	else
		_tcscpy_safe(szReturnedString, nSize, szDefault);

	return _tcslen(szReturnedString);
}

BOOL RegWriteProfileString(HKEY hRoot, LPCTSTR szItem, LPCTSTR szKeyName, LPCTSTR szValue)
{
	HKEY	hKey;
	DWORD	dwDispositon;
	BOOL	bRet = false;

	if (RegCreateKeyEx(hRoot, szItem,
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKey, &dwDispositon) == ERROR_SUCCESS)
	{
		DWORD		dwType = REG_SZ;
		if (RegSetValueEx(hKey, szKeyName, 0, dwType, (BYTE *)szValue, lstrlen(szValue) * sizeof(TCHAR)) == ERROR_SUCCESS)
		{
			bRet = true;
		}
		RegCloseKey(hKey);
	}

	return bRet;
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
		else if (dwType == REG_SZ)
		{
			TCHAR		szValue[256];
			DWORD		nSizeRet = CountOf(szValue) * sizeof(TCHAR);
			DWORD		dwType = REG_SZ;
			if (RegQueryValueEx(hKey, szKeyName, NULL, &dwType, (BYTE *)szValue, &nSizeRet) != ERROR_SUCCESS)
			{
				nValue = nDefault;
			}
			else
			{
				nSizeRet /= sizeof(TCHAR);
				if (nSizeRet >= CountOf(szValue))
					nSizeRet = CountOf(szValue) - 1;
				szValue[nSizeRet] = '\0';
				nValue = _ttoi(szValue);
			}
		}
		else
			ASSERT(dwType == REG_DWORD);
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


void RegSetKeyDefValueIfNotExist(HKEY hRoot, LPCTSTR szItem, LPCTSTR szKeyName, LPCTSTR szDefault)
{
	HKEY	hKey;

	if (RegOpenKeyEx(hRoot, szItem,
		0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		DWORD		nSizeRet = 0;
		DWORD		dwType = REG_SZ;

		if (RegQueryValueEx(hKey, szKeyName, NULL, &dwType, NULL, &nSizeRet) != ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			goto R_FAILED;
		}
		RegCloseKey(hKey);
	}
	else
		goto R_FAILED;

R_FAILED:
	RegWriteProfileString(hRoot, szItem, szKeyName, szDefault);
}

void RegSetKeyDefValueIfNotExist(HKEY hRoot, LPCTSTR szItem, LPCTSTR szKeyName, int nDefault)
{
	HKEY	hKey;

	if (RegOpenKeyEx(hRoot, szItem,
		0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		DWORD		nSizeRet = 0;
		DWORD		dwType = REG_DWORD;

		if (RegQueryValueEx(hKey, szKeyName, NULL, &dwType, NULL, &nSizeRet) != ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			goto R_FAILED;
		}
		RegCloseKey(hKey);
	}
	else
		goto R_FAILED;

R_FAILED:
	RegWriteProfileInt(hRoot, szItem, szKeyName, nDefault);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProfile::CProfile()
{
	for (int i = 0; i < RET_STR_MAX; i++)
		m_szRetString[i] = NULL;

	m_hKeyRoot = NULL;
	m_nCurRetStr = 0;
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

#define HKEY_LEN		5
	if (_tcsncmp(szProfile, _T("HKEY_"), HKEY_LEN) == 0)
	{
		/*HKEY_CLASSES_ROOT
		HKEY_CURRENT_CONFIG
		HKEY_CURRENT_USER
		HKEY_LOCAL_MACHINE
		HKEY_USERS*/
		int		nLenRootKey;

		// uses registry
		if (_tcsncmp(szProfile + HKEY_LEN, _T("LOCAL_MACHINE"), 
								   nLenRootKey = _tcslen(_T("LOCAL_MACHINE"))) == 0)
		{
			m_hKeyRoot = HKEY_LOCAL_MACHINE;
		}
		else if (_tcsncmp(szProfile + HKEY_LEN, _T("CURRENT_USER"), 
								   nLenRootKey = _tcslen(_T("CURRENT_USER"))) == 0)
		{
			m_hKeyRoot = HKEY_CURRENT_USER;
		}
		else if (_tcsncmp(szProfile + HKEY_LEN, _T("USERS"), 
								   nLenRootKey = _tcslen(_T("USERS"))) == 0)
		{
			m_hKeyRoot = HKEY_USERS;
		}
		else if (_tcsncmp(szProfile + HKEY_LEN, _T("CLASSES_ROOT"), 
								   nLenRootKey = _tcslen(_T("CLASSES_ROOT"))) == 0)
		{
			m_hKeyRoot = HKEY_CLASSES_ROOT;
		}
#ifndef _WIN32_WCE
		else if (_tcsncmp(szProfile + HKEY_LEN, _T("CURRENT_CONFIG"), 
								   nLenRootKey = _tcslen(_T("CURRENT_CONFIG"))) == 0)
		{
			m_hKeyRoot = HKEY_CURRENT_CONFIG;
		}
#endif
		else
			m_hKeyRoot = NULL;

		if (m_hKeyRoot)
		{
			m_nCurRetStr = 0;

			if (szProfile[HKEY_LEN + nLenRootKey] == '\\')
				nLenRootKey++;

			m_strProfile = szProfile + HKEY_LEN + nLenRootKey;
			DirStringFormat(m_strProfile);

			for (int i = 0; i < RET_STR_MAX; i++)
				m_szRetString[i] = new TCHAR[RET_STR_LEN];
			return;
		}
	}

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
	m_hKeyRoot = NULL;

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

	for (int i = 0; i < RET_STR_MAX; i++)
	{
		if (m_szRetString[i])
		{
			delete []m_szRetString[i];
			m_szRetString[i] = NULL;
		}
	}
}

static void ProfileKeyValueMultiStrToVStr(LPCTSTR szText, vector<tstring> &vStr)
{
	while (*szText)
	{
		LPCTSTR		szBeg;
		szBeg = szText;
		while (*szText && *szText != '=')
			szText++;
		if (*szText == '=')
		{
			tstring		str;
			str.append(szBeg, szText);

			szText++;
			vStr.push_back(str);
			vStr.push_back(szText);
			while (*szText != '\0')
				szText++;
			szText++;
		}
		else
			szText++;
	}
}

void CProfile::DoCache()
{
	if (m_hKeyRoot)
		return;

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
	if (m_hKeyRoot)
	{
		tstring	strSubKey;

		strSubKey = m_strProfile + szAppName;

		RegSetKeyDefValueIfNotExist(m_hKeyRoot, strSubKey.c_str(), szKeyName, szDefault);
	}
	else
		SetKeyDefaultIfNotExist(szAppName, szKeyName, szDefault);
}

void CProfile::SetDefaultIfNotExist(LPCTSTR szAppName, LPCTSTR szKeyName, int nDefault)
{
	if (m_hKeyRoot)
	{
		tstring	strSubKey;

		strSubKey = m_strProfile + szAppName;

		RegSetKeyDefValueIfNotExist(m_hKeyRoot, strSubKey.c_str(), szKeyName, nDefault);
	}
	else
	{
		TCHAR	szDefault[64];
		_itot_s(nDefault, szDefault, CountOf(szDefault), 10);

		SetKeyDefaultIfNotExist(szAppName, szKeyName, szDefault);
	}
}

DWORD CProfile::GetProfileString(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szDefault, LPTSTR szReturnedString, DWORD nSize)
{
	assert(!m_strProfile.empty());

	if (m_hKeyRoot)
	{
		tstring	strSubKey;

		strSubKey = m_strProfile + szAppName;

		return RegGetProfileString(m_hKeyRoot, strSubKey.c_str(), szKeyName, szDefault, szReturnedString, nSize);
	}

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

	if (m_hKeyRoot)
	{
		tstring	strSubKey;

		strSubKey = m_strProfile + szAppName;

		m_nCurRetStr++;
		if (m_nCurRetStr >= RET_STR_MAX)
			m_nCurRetStr = 0;

		RegGetProfileString(m_hKeyRoot, strSubKey.c_str(), szKeyName, szDefault, m_szRetString[m_nCurRetStr], RET_STR_LEN);
		return m_szRetString[m_nCurRetStr];
	}

	DWORD		dwRet;

	if (szKeyName[0] == '\0')
		return _T("");

	MAP_STRING::iterator	itKey;

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

	return (*itKey).second.c_str();
}

BOOL CProfile::WriteProfileString(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szValue)
{
	assert(!m_strProfile.empty());

	if (m_hKeyRoot)
	{
		tstring	strSubKey;

		strSubKey = m_strProfile + szAppName;

		return RegWriteProfileString(m_hKeyRoot, strSubKey.c_str(), szKeyName, szValue);
	}

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

	if (m_hKeyRoot)
	{
		tstring	strSubKey;

		strSubKey = m_strProfile + szAppName;

		return RegGetProfileInt(m_hKeyRoot, strSubKey.c_str(), szKeyName, nDefault);
	}

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

	if (m_hKeyRoot)
	{
		tstring	strSubKey;

		strSubKey = m_strProfile + szAppName;

		return RegWriteProfileInt(m_hKeyRoot, strSubKey.c_str(), szKeyName, nValue);
	}

	TCHAR		szValue[MAX_PATH];
	
	wsprintf(szValue, _T("%d"), nValue);

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
void EncryptStr(LPCTSTR szStr, tstringex &strOut)
{
	stringex	strEncrypt;
	stringex	strAnsi;
	ConvertStr(szStr, -1, strAnsi);
	encodeuu(strAnsi.c_str(), strAnsi.size(), strEncrypt, true, false);
	_xcsrep(strEncrypt.data(), '=', '?');
	ConvertStr(strEncrypt.c_str(), strEncrypt.size(), strOut);
}

void DecryptStr(LPCTSTR szStr, tstringex &strOut)
{
	stringex	strDecrypt;
	stringex	strAnsi;
	ConvertStr(szStr, -1, strAnsi);
	_xcsrep(strAnsi.data(), '?', '=');
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

	if (m_hKeyRoot)
	{
		tstring	strSubKey;

		strSubKey = m_strProfile + szAppName;

		EncryptStr(szDefault, strValue);
		dwRet = RegGetProfileString(m_hKeyRoot, strSubKey.c_str(), strKeyNameEncrypt.c_str(), strValue.c_str(), szReturnedString, nSize);
		DecryptStr(szReturnedString, strValue);
		_tcscpy_safe(szReturnedString, nSize, strValue.c_str());

		return strValue.size();
	}

	MAP_STRING::iterator	itKey;

	if (!GetKey(szAppName, strKeyNameEncrypt.c_str(), itKey))
	{
		EncryptStr(szDefault, strValue);
		dwRet = GetPrivateProfileString(szAppName, strKeyNameEncrypt.c_str(), strValue.c_str(), szReturnedString, nSize, m_strProfile.c_str());
		AddKeys(szAppName, strKeyNameEncrypt.c_str(), szReturnedString);
	}
	else
	{
		_tcscpy_safe(szReturnedString, nSize, (*itKey).second.c_str());
	}
	DecryptStr(szReturnedString, strValue);
	_tcscpy_safe(szReturnedString, nSize, strValue.c_str());

	return _tcslen(szReturnedString);
}

BOOL CProfile::EncryptWriteProfileString(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szValue)
{
	assert(!m_strProfile.empty());

	DWORD		dwRet;
	MAP_STRING::iterator	itKey;
	tstringex	strKeyNameEncrypt, strValueEncrypt;

	EncryptStr(szKeyName, strKeyNameEncrypt);
	EncryptStr(szValue, strValueEncrypt);

	if (m_hKeyRoot)
	{
		tstring	strSubKey;

		strSubKey = m_strProfile + szAppName;

		return RegWriteProfileString(m_hKeyRoot, strSubKey.c_str(), strKeyNameEncrypt.c_str(), strValueEncrypt.c_str());
	}

	dwRet = WritePrivateProfileString(szAppName, strKeyNameEncrypt.c_str(), strValueEncrypt.c_str(), m_strProfile.c_str());

	if (!GetKey(szAppName, strKeyNameEncrypt.c_str(), itKey))
	{
		AddKeys(szAppName, strKeyNameEncrypt.c_str(), szValue);
	}
	else
		(*itKey).second = strValueEncrypt;

	return dwRet;
}
