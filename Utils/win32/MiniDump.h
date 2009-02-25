#ifndef __MINIDUMPER_H_
#define __MINIDUMPER_H_

#pragma once

#if _MSC_VER < 1300
#define DECLSPEC_DEPRECATED
// VC6: change this path to your Platform SDK headers
#include "M:\\dev7\\vs\\devtools\\common\\win32sdk\\include\\dbghelp.h"			// must be XP version of file
#else
// VC7: ships with updated headers
#include "dbghelp.h"
#endif

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
	CMiniDumper();

	static void Init(IMiniDumperNotify *pDumpNotify);

private:
	static LONG WINAPI TopLevelFilter(struct _EXCEPTION_POINTERS *pExceptionInfo);

	static BOOL GetModuleByAddress(PVOID addr, LPTSTR szModule, int nLen, HMODULE &hModule);

protected:
	static IMiniDumperNotify			*m_pDumpNotify;

};

#endif // __MINIDUMPER_H_
