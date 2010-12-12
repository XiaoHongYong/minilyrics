#include "StdAfx.h"
#include "FileVersionInfo.h"

CFileVersionInfo::CFileVersionInfo()
{
	m_pFileInfo = NULL;
	m_pLanguages = NULL;
	m_nLanguageCount = 0;
}


CFileVersionInfo::~CFileVersionInfo()
{
}

bool CFileVersionInfo::Open(LPCTSTR lpszFileName)
{
	m_pFileInfo = NULL;
	m_pLanguages = NULL;
	m_nLanguageCount = 0;
	m_strCurLangBlockName.clear();

	DWORD	dwHandle;
	DWORD	dwFileVersionInfoSize = GetFileVersionInfoSize((LPTSTR)lpszFileName, &dwHandle);
	if (!dwFileVersionInfoSize)
		return false;

	m_buf.reserve(dwFileVersionInfoSize);
	if (m_buf.capacity() < dwFileVersionInfoSize)
		return false;

	if (!GetFileVersionInfo((LPTSTR)lpszFileName, dwHandle, dwFileVersionInfoSize, m_buf.data()))
		return false;

	UINT		nInfoLen;
	if (VerQueryValue(m_buf.data(), _T("\\"), (LPVOID *)&m_pFileInfo, &nInfoLen))
	{
		if (nInfoLen != sizeof(VS_FIXEDFILEINFO))
		{
			ASSERT(nInfoLen == sizeof(VS_FIXEDFILEINFO));
			m_pFileInfo = NULL;
			return false;
		}
	}

	if (VerQueryValue(m_buf.data(), _T("\\VarFileInfo\\Translation"), (LPVOID *)&m_pLanguages, &nInfoLen))
		m_nLanguageCount = nInfoLen / sizeof(LANGANDCODEPAGE);

	AutoSetLanguage();

	return TRUE;
}

void CFileVersionInfo::GetFileVersion(int &nMajor, int &nMinor, int &nBuild, int &nRevision)
{
	nMajor = HIWORD(m_pFileInfo->dwFileVersionMS);
	nMinor = LOWORD(m_pFileInfo->dwFileVersionMS);

	nBuild = HIWORD(m_pFileInfo->dwFileVersionLS);
	nRevision = LOWORD(m_pFileInfo->dwFileVersionLS);
}

void CFileVersionInfo::GetProductVersion(int &nMajor, int &nMinor, int &nBuild, int &nRevision)
{
	nMajor = HIWORD(m_pFileInfo->dwProductVersionMS);
	nMinor = LOWORD(m_pFileInfo->dwProductVersionMS);

	nBuild = HIWORD(m_pFileInfo->dwProductVersionLS);
	nRevision = LOWORD(m_pFileInfo->dwProductVersionLS);
}

//
// The method below is related to current language Id.
//
void CFileVersionInfo::SetCurrentLanguage(int nIndex)
{
	if (m_nLanguageCount <= 0)
		return;

	ASSERT(nIndex >= 0 && nIndex < m_nLanguageCount);
	if (nIndex < 0 || nIndex >= m_nLanguageCount)
		nIndex = 0;

	m_strCurLangBlockName = CStrPrintf(_T("\\StringFileInfo\\%04X%04X\\"), 
		m_pLanguages[nIndex].wCodePage, m_pLanguages[nIndex].wLanguage).c_str();
}

tstring CFileVersionInfo::GetLanguageName(int nLanguageId)
{
	TCHAR	szBuf[256];
	EmptyStr(szBuf);

	VerLanguageName(nLanguageId, szBuf, CountOf(szBuf));

	return szBuf;
}

void CFileVersionInfo::AutoSetLanguage()
{
	if (m_nLanguageCount <= 0)
		return;

	if (m_nLanguageCount == 1)
	{
		SetCurrentLanguage(0);
		return;
	}

	LANGID		vQueryLangId[] = {
		GetUserDefaultLangID(), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL)
	};

	for (int k = 0; k < CountOf(vQueryLangId); k++)
	{
		for (int i = 0; i < m_nLanguageCount; i++)
		{
			if (m_pLanguages[i].wLanguage == vQueryLangId[k]
				|| (PRIMARYLANGID(vQueryLangId[k]) == PRIMARYLANGID(m_pLanguages[i].wLanguage)))
			{
				SetCurrentLanguage(i);
				return;
			}
		}
	}

	SetCurrentLanguage(0);
}

bool CFileVersionInfo::GetValue(LPCTSTR szValueName, tstring &strValue)
{
	LPVOID		lpInfo;
	UINT		nInfoLen;
	if (VerQueryValue(m_buf.data(), (LPTSTR)(m_strCurLangBlockName + szValueName).c_str(),
		&lpInfo, &nInfoLen))
	{
		strValue = (LPCTSTR)lpInfo;
		return true;
	}

	return false;
}
