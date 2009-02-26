
#include "stdafx.h"
#include "App.h"
#include "MiniDump.h"

IMiniDumperNotify	*CMiniDumper::m_pDumpNotify = NULL;

CMiniDumper::CMiniDumper()
{
}

void CMiniDumper::Init(IMiniDumperNotify *pDumpNotify)
{
	if (m_pDumpNotify)
	{
		// already inited.
		return;
	}
	m_pDumpNotify = pDumpNotify;

	SetUnhandledExceptionFilter(TopLevelFilter);
}

LONG CMiniDumper::TopLevelFilter(struct _EXCEPTION_POINTERS *pExceptionInfo)
{
	LONG retval = EXCEPTION_CONTINUE_SEARCH;
	HWND hParent = NULL;						// find a better value for your app

	if (!m_pDumpNotify)
		return EXCEPTION_CONTINUE_SEARCH;

	// firstly see if dbghelp.dll is around and has the function we need
	// look next to the EXE first, as the one in System32 might be old 
	// (e.g. Windows 2000)
	HMODULE hDll = NULL;
	TCHAR	szDbgHelpPath[_MAX_PATH];

	if (GetModulePath(szDbgHelpPath, GetAppInstance()))
	{
		_tcscat_safe(szDbgHelpPath, CountOf(szDbgHelpPath), _T("DBGHELP.DLL"));
		hDll = ::LoadLibrary(szDbgHelpPath);
	}

	// load any version we can
	if (hDll == NULL)
		hDll = ::LoadLibrary(_T("DBGHELP.DLL"));
	if (!hDll)
		return EXCEPTION_CONTINUE_SEARCH;

	TCHAR szFaultingModule[MAX_PATH];
	HMODULE	hFaultModule;

	// Get Except fault module
	if (!GetModuleByAddress(pExceptionInfo->ExceptionRecord->ExceptionAddress, szFaultingModule, CountOf(szFaultingModule), hFaultModule))
		return EXCEPTION_CONTINUE_SEARCH;

	ERRLOG1(_T("Got crash: %s"), szFaultingModule);

	// Get MiniDumpWrite function ptr
	MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress(hDll, "MiniDumpWriteDump");
	if (!pDump)
	{
		FreeLibrary(hDll);
		ERRLOG0(_T("DbgHelp.dll is too old."));
		return EXCEPTION_CONTINUE_SEARCH;
	}

	TCHAR szDumpPath[_MAX_PATH] = { 0 };

	// 
	if (!m_pDumpNotify->OnBeginDump(hFaultModule, szDumpPath, CountOf(szDumpPath)))
		return EXCEPTION_CONTINUE_SEARCH;

	FileSetExt(szDumpPath, CountOf(szDumpPath), _T(".dmp"));

	// create the file
	HANDLE hFile = ::CreateFile(szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
								FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		FreeLibrary(hDll);
		ERRLOG1(_T("FAILED to create dump file: %s."), szDumpPath);
		return EXCEPTION_CONTINUE_SEARCH;
	}

	_MINIDUMP_EXCEPTION_INFORMATION ExInfo;

	ExInfo.ThreadId = ::GetCurrentThreadId();
	ExInfo.ExceptionPointers = pExceptionInfo;
	ExInfo.ClientPointers = NULL;

	// write the dump
	if (pDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL))
	{
		ERRLOG1(_T("Save dump file to: %s."), szDumpPath);
		retval = EXCEPTION_EXECUTE_HANDLER;

		::CloseHandle(hFile);

		// Notify mini dump file is saved.
		if (!m_pDumpNotify->OnDumpFinished(hFaultModule, szDumpPath))
			return EXCEPTION_CONTINUE_SEARCH;
	}
	else
	{
		::CloseHandle(hFile);
		ERRLOG0(_T("Failed to write mini dump file."));
	}

	return retval;
}

BOOL CMiniDumper::GetModuleByAddress(PVOID addr, LPTSTR szModule, int nLen, HMODULE &hModule)
{
	MEMORY_BASIC_INFORMATION mbi;

	if (!VirtualQuery(addr, &mbi, sizeof(mbi)))
		return FALSE;

	DWORD hMod = (DWORD)mbi.AllocationBase;

	if (!GetModuleFileName((HMODULE)hMod, szModule, nLen))
		return FALSE;

	hModule = GetModuleHandle(szModule);
	if (!hModule)
		return FALSE;

	return TRUE;
}
