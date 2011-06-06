#include "stdafx.h"
#include "UACTool.h"
#include <ShellAPI.h>

BOOL IsVistaOrLater()
{
	OSVERSIONINFO osver;

	osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	
	if (::GetVersionEx(&osver) 
		&& osver.dwPlatformId == VER_PLATFORM_WIN32_NT
		&& (osver.dwMajorVersion >= 6))
		return TRUE;

	return FALSE;
}

HRESULT GetElevationType(__out TOKEN_ELEVATION_TYPE * ptet)
{
	ASSERT(IsVistaOrLater());
	ASSERT(ptet);

	HANDLE hToken	= NULL;

	if (!::OpenProcessToken(::GetCurrentProcess(), 
				TOKEN_QUERY, 
				&hToken))
	{
		ASSERT(FALSE);
		return E_FAIL;
	}

	DWORD dwReturnLength = 0;
	HRESULT hResult = E_FAIL;
	if (!::GetTokenInformation(
				hToken,
				TokenElevationType,
				ptet,
				sizeof(*ptet),
				&dwReturnLength))
	{
		ASSERT(FALSE);
	}
	else
	{
		ASSERT(dwReturnLength == sizeof(*ptet));
		hResult = S_OK;
	}

	::CloseHandle(hToken);

	return hResult;
}

HRESULT IsElevated(__out_opt BOOL * pbElevated)
{
	ASSERT(IsVistaOrLater());

	HANDLE hToken	= NULL;

	if (!::OpenProcessToken(
				::GetCurrentProcess(), 
				TOKEN_QUERY, 
				&hToken))
	{
		ASSERT(FALSE);
		return E_FAIL;
	}

	TOKEN_ELEVATION te = { 0 };
	DWORD dwReturnLength = 0;
	HRESULT hResult = E_FAIL;

	if (!::GetTokenInformation(
				hToken,
				TokenElevation,
				&te,
				sizeof(te),
				&dwReturnLength))
	{
		ASSERT(FALSE);
	}
	else
	{
		ASSERT(dwReturnLength == sizeof(te));

		hResult = te.TokenIsElevated ? S_OK : S_FALSE; 

		if (pbElevated)
			*pbElevated = (te.TokenIsElevated != 0);
	}

	::CloseHandle(hToken);

	return hResult;
}

////////////////////////////////
// RunElevated simply calls ShellExecuteEx with the verb "runas" to start the elevated process.
// I wish there was a just as easy way to start a non-elevated process, as well.

BOOL RunElevated(
	__in		HWND	hwnd, 
	__in		LPCTSTR pszPath, 
	__in_opt	LPCTSTR pszParameters, //	= NULL, 
	__in_opt	LPCTSTR pszDirectory) //	= NULL);
{
	SHELLEXECUTEINFO shex;

	memset(&shex, 0, sizeof(shex));

	shex.cbSize			= sizeof(SHELLEXECUTEINFO); 
	shex.fMask			= 0; 
	shex.hwnd			= hwnd;
	shex.lpVerb			= _T("runas");
	shex.lpFile			= pszPath;
	shex.lpParameters	= pszParameters;
	shex.lpDirectory	= pszDirectory;
	shex.nShow			= SW_NORMAL;

	return ::ShellExecuteEx(&shex);
}


/// the text to be displayed as title of the messages:
LPCTSTR pszMsgTitle = _T("Vista Elevator");

BOOL RunMyselfElevated(HWND hwnd)
{
	TCHAR szPath[MAX_PATH];

	::GetModuleFileName(NULL, szPath, MAX_PATH);

	BOOL bSuccess = RunElevated(hwnd, szPath);

	if (!bSuccess)
	{
		::MessageBox( NULL, 
			_T("Could not perform this action."),
			pszMsgTitle,
			MB_OK | MB_ICONSTOP );
	}

	return bSuccess;
}
