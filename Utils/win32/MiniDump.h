#pragma once

#include "dbghelp.h"

class IMiniDumperNotify
{
public:
	virtual bool OnBeginDump(HMODULE hCrashMod, TCHAR szDumpFileToSave[], int nLen) = 0;
	virtual bool OnDumpFinished(HMODULE hCrashMod, LPCTSTR szDumpFileToSave) = 0;

};

// based on dbghelp.h
typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
									CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
									CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
									CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
									);

class CMiniDumper
{
public:
	static void Init(IMiniDumperNotify *pDumpNotify);

private:
	static LONG WINAPI TopLevelFilter(struct _EXCEPTION_POINTERS *pExceptionInfo);

	static BOOL GetModuleByAddress(PVOID addr, LPTSTR szModule, int nLen, HMODULE &hModule);

};
