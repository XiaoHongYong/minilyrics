#include "stdafx.h"
#include "base.h"
#include "ProcessEnum.h"

CProcessEnum::CProcessEnum()
{
	m_funcEnum = NULL; m_funcGetModuleFileNameEx = NULL;

	m_hPsApi = LoadLibrary(_T("psapi.dll"));
	ASSERT(m_hPsApi);
	if (!m_hPsApi)
		return;

	m_funcEnum = (EnumProcessesFunc)GetProcAddress(m_hPsApi, "EnumProcesses");
#ifdef UNICODE
	m_funcGetModuleFileNameEx = (GetModuleFileNameExWFunc)GetProcAddress(m_hPsApi, "GetModuleFileNameExW");
#else
	m_funcGetModuleFileNameEx = (GetModuleFileNameExWFunc)GetProcAddress(m_hPsApi, "GetModuleFileNameExA");
#endif
}
CProcessEnum::~CProcessEnum()
{
	FreeLibrary(m_hPsApi);
}

BOOL CProcessEnum::Enum(DWORD * lpidProcess, DWORD cb, LPDWORD lpcbNeeded)
{
	if (m_funcEnum)
		return m_funcEnum(lpidProcess, cb, lpcbNeeded);
	else
		return false;
}

DWORD CProcessEnum::GetModuleFileNameEx(HANDLE hProcess, HMODULE hModule, LPTSTR lpFilename, DWORD nSize)
{
	if (m_funcGetModuleFileNameEx)
		return m_funcGetModuleFileNameEx(hProcess, hModule, lpFilename, nSize);
	else
		return 0;
}

tstring CProcessEnum::GetProcessFileName(DWORD dwProcessID)
{
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessID);
	// ASSERT(hProcess);
	if (hProcess)
	{
		TCHAR	szFile[MAX_PATH];
		int n = GetModuleFileNameEx(hProcess, NULL, szFile, CountOf(szFile));
		if (n > 0)
			return szFile;
	}

	return _T("");
}
