#pragma once

class CProcessEnum
{
public:
	CProcessEnum();
	~CProcessEnum();

	BOOL Enum(DWORD * lpidProcess, DWORD cb, LPDWORD lpcbNeeded);

	DWORD GetModuleFileNameEx(HANDLE hProcess, HMODULE hModule, LPTSTR lpFilename, DWORD nSize);
	tstring GetProcessFileName(DWORD dwProcessID);

protected:
	typedef BOOL (WINAPI *EnumProcessesFunc) (DWORD * lpidProcess, DWORD cb, LPDWORD lpcbNeeded);
	typedef DWORD (WINAPI *GetModuleFileNameExWFunc)(HANDLE hProcess, HMODULE hModule, LPTSTR lpFilename, DWORD nSize);

	HMODULE						m_hPsApi;
	EnumProcessesFunc			m_funcEnum;
	GetModuleFileNameExWFunc	m_funcGetModuleFileNameEx;

};
