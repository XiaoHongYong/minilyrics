/********************************************************************
	Created  :	2002/01/04	21:30
	FileName :	Profile.cpp
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#include "stdafx.h"
#include "Profile.h"

CProfile	g_Profile;


// PURPOSE   : Get full task save file name by task name.
//
// PARAMETER: 
//      szPath is the path that returned: format is like : "c:\\antivirus\\"
BOOL GetProgramPath(TCHAR szPath[], HINSTANCE hInstance/* = NULL*/)
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

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProfile::CProfile()
{
	m_szProfile[0] = '\0';
	m_bModified = FALSE;
}

CProfile::~CProfile()
{
}

CProfile::STRING_MAP::iterator CProfile::GetKey(LPCTSTR szAppName, LPCTSTR szKeyName)
{
	APPNAME_MAP::iterator	it;

	it = m_mapAppName2Keys.find(szAppName);
	if (it == m_mapAppName2Keys.end())
	{
		//
		// 不区分大小写，查找AppName
		for (it = m_mapAppName2Keys.begin(); it != m_mapAppName2Keys.end(); it++)
		{
			if (stricmp((*it).first.c_str(), szAppName) == 0)
				break;
		}

		if (it == m_mapAppName2Keys.end())
			return NULL;
	}

	STRING_MAP::iterator	itStr;
	itStr = (*it).second.find(szKeyName);
	if (itStr == (*it).second.end())
	{
		//
		// 不区分大小写，查找KeyName
		for (itStr = (*it).second.begin(); itStr != (*it).second.end(); itStr++)
		{
			if (stricmp((*itStr).first.c_str(), szKeyName) == 0)
				break;
		}
		if (itStr == (*it).second.end())
			return NULL;
	}
	return itStr;
}

DWORD CProfile::GetString(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szDefault, LPSTR szReturnedString, DWORD nSize)
{
	ASSERT(m_szProfile[0] != '\0');

	DWORD		dwRet;

	STRING_MAP::iterator	itStr;

	itStr = GetKey(szAppName, szKeyName);
	if (itStr == NULL)
	{
		dwRet = GetPrivateProfileString(szAppName, szKeyName, szDefault, szReturnedString, nSize, m_szProfile);
		AddKeys(szAppName, szKeyName, szReturnedString);
		return dwRet;
	}
	else
	{
		strcpy(szReturnedString, (*itStr).second.c_str());

		return (*itStr).second.size();
	}
}

BOOL CProfile::WriteString(LPCTSTR szAppName, LPCTSTR szKeyName, LPCTSTR szValue)
{
	ASSERT(m_szProfile[0] != '\0');

	DWORD		dwRet;
	STRING_MAP::iterator	itStr;

	dwRet = WritePrivateProfileString(szAppName, szKeyName, szValue, m_szProfile);

	itStr = GetKey(szAppName, szKeyName);
	if (itStr == NULL)
	{
		AddKeys(szAppName, szKeyName, szValue);
	}
	else
		(*itStr).second = szValue;

	return dwRet;
}


UINT CProfile::GetInt(LPCTSTR szAppName, LPCTSTR szKeyName, int nDefault)
{
	ASSERT(m_szProfile[0] != '\0');

	TCHAR	szValue[64];
	TCHAR	szDefault[64];
	sprintf(szDefault, "%d", nDefault);

	GetString(szAppName, szKeyName, szDefault, szValue, 64);

	return atoi(szValue);
}

BOOL CProfile::WriteInt(LPCTSTR szAppName, LPCTSTR szKeyName, int nValue)
{
	ASSERT(m_szProfile[0] != '\0');

	TCHAR		szValue[MAX_PATH];
	
	wsprintf(szValue, _T("%d"), nValue);

	return WriteString(szAppName, szKeyName, szValue);
}

void CProfile::GetFile(char szProfile[])
{
}

void CProfile::Init(LPCTSTR szProfile, HINSTANCE hInst)
{
	m_mapAppName2Keys.clear();

	if (strlen(szProfile) > 3)
	{
		if ((szProfile[1] == ':') && (szProfile[2] == '\\'))
		{
			// 如果已经是全路径，则不管
			lstrcpy(m_szProfile, szProfile);
			return;
		}
	}

	GetProgramPath(m_szProfile, hInst);
	lstrcat(m_szProfile, szProfile);
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
