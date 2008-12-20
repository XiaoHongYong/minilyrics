// CrashHandler.h: interface for the CCrashHandler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CRASHHANDLER_H__3D49A3C7_76FE_4AE9_96B6_56E9219DC44C__INCLUDED_)
#define AFX_CRASHHANDLER_H__3D49A3C7_76FE_4AE9_96B6_56E9219DC44C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



class CCrashHandler;

typedef BOOL (*FunCrashCallback)(CCrashHandler *pCrashHandle, LPCTSTR szMsg, HMODULE hCrashMod, LPVOID lpUserData);

class CCrashHandler  
{
public:
	BOOL Report(PEXCEPTION_POINTERS pExceptionRecord);

	void Init(FunCrashCallback funCallback, LPVOID lpUserData = NULL);

	// void AddWatchModule(HANDLE hModule);

	CCrashHandler();
	virtual ~CCrashHandler();

protected:
	LPTOP_LEVEL_EXCEPTION_FILTER  m_oldFilter;      // previous exception filter
	// LPGETLOGFILE                  m_lpfnCallback;   // client crash callback
	int                           m_pid;            // process id
	FunCrashCallback				m_funCallback;
	LPVOID							m_lpUserData;
	// vector<HANDLE>					m_vWatchModule;

friend LONG WINAPI CustomUnhandledExceptionFilter(PEXCEPTION_POINTERS pExInfo);

};

extern CCrashHandler		g_CrashHandler;

#endif // !defined(AFX_CRASHHANDLER_H__3D49A3C7_76FE_4AE9_96B6_56E9219DC44C__INCLUDED_)
