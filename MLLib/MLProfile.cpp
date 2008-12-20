/********************************************************************
	Created  :	2002/01/04	21:30
	FileName :	MLProfile.cpp
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#include "stdafx.h"
#include "MLLib.h"
#include "MLProfile.h"

#define CHAR_WORKING_FOLDER			'-'

void AnalysisProxySetting(LPCTSTR szProxySetting, TCHAR szServer[], int nMaxSize, int &nPort);

//
//取得IE 的代理设置
//	else if (nHttpProxyType == HTTP_PROXY_IE)
//	{
//		// 取得IE 的代理设置
//		// [HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Internet Settings]
//		// REG_SZ: "ProxyServer"="127.1.1.1:80"
//		// ftp=localhost:80;gopher=localhost:80;http=localhost:80;https=localhost:80
//		HKEY	hKeyInet;
//
//		if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",
//			0, KEY_QUERY_VALUE, &hKeyInet) == ERROR_SUCCESS)
//		{
//			DWORD		nSizeProxy = sizeof(szProxy);
//			DWORD		dwType = REG_SZ;
//			if (RegQueryValueEx(hKeyInet, "ProxyServer", NULL, &dwType, (BYTE *)szProxy, &nSizeProxy) != ERROR_SUCCESS)
//			{
//				ERRLOG0("RegQueryValueEx: ProxyServer FAILED!");
//			}
//			else
//			{
//				// strstr(szProxy, "http=")
//			}
//			RegCloseKey(hKeyInet);
//		}
//		else
//			ERRLOG0("RegOpenKeyEx: [HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings] FAILED!");
//	}


CMLProfile::CMLProfile()
{
}

CMLProfile::~CMLProfile()
{

}

BOOL CMLProfile::InetIsUseProxy()
{
	return (g_Profile.GetProfileInt(_T("ProxyType"), HTTP_PROXY_NONE) != HTTP_PROXY_NONE);
}

BOOL CMLProfile::InetGetProxy(TCHAR szServer[], int &nPort)
{
	int		nHttpProxyType;

	EmptyStr(szServer);
	nPort = __HTTP_PORT;

	// 代理类型？
	nHttpProxyType = g_Profile.GetProfileInt(_T("ProxyType"), HTTP_PROXY_NONE);
	if (nHttpProxyType == HTTP_PROXY_OURS)
	{
		// 我们自己的代理设置
		// 代理服务器地址
		nPort = g_Profile.GetProfileInt(_T("ProxyPort"), __HTTP_PORT);
		if (nPort <= 0)
			nPort = __HTTP_PORT;
		_tcscpy(szServer, g_Profile.GetProfileString(_T("ProxyServer"), _T("")));
		if (IsEmptyString(szServer))
		{
			//
			// 可能是minilyrics 旧的proxy 设置，分析并读取
			//
			AnalysisProxySetting(g_Profile.GetProfileString(_T("Proxy"), _T("")),
				szServer, 128, nPort);
		}

		if (!IsEmptyString(szServer))
			return TRUE;
	}

	return FALSE;
}

LPCTSTR CMLProfile::InetGetBase64ProxyUserPass()
{
	// return _T("");
	return g_Profile.GetProfileString(_T("Base64ProxyUserPass"), _T(""));
}


BOOL CMLProfile::WriteDir(LPCTSTR szSectName, LPCTSTR szKeyName, LPCTSTR szDir)
{
	if (szDir[0] == GetWorkingFolder()[0] && szDir[1] == ':')
	{
		// Lyrics folder is the same drive of Current working folder
		tstring		str = szDir;
		str[0] = CHAR_WORKING_FOLDER;
		return g_Profile.WriteProfileString(szSectName, szKeyName, str.c_str());
	}
	else
		return g_Profile.WriteProfileString(szSectName, szKeyName, szDir);
}


tstring CMLProfile::GetDir(LPCTSTR szSectName, LPCTSTR szKeyName, LPCTSTR szDefDir)
{
	tstring		strDir;

	strDir = g_Profile.GetProfileString(szSectName, szKeyName, szDefDir);

	if (strDir[0] == CHAR_WORKING_FOLDER && strDir[1] == ':')
		strDir[0] = GetWorkingFolder()[0];

	return strDir;
}

