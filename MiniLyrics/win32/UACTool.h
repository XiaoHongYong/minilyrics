#pragma once


BOOL IsVistaOrLater();

/*
Use GetElevationType() to determine the elevation type of the current process.

Parameters:

ptet
	[out] Pointer to a variable that receives the elevation type of the current process.

	The possible values are:

	TokenElevationTypeDefault - User is not using a "split" token. 
		This value indicates that either UAC is disabled, or the process is started
		by a standard user (not a member of the Administrators group).

	The following two values can be returned only if both the UAC is enabled and
	the user is a member of the Administrator's group (that is, the user has a "split" token):

	TokenElevationTypeFull - the process is running elevated. 

	TokenElevationTypeLimited - the process is not running elevated.

Return Values:
	If the function succeeds, the return value is S_OK. 
	If the function fails, the return value is E_FAIL. To get extended error information, 
	call GetLastError().
*/

HRESULT GetElevationType( __out TOKEN_ELEVATION_TYPE * ptet );

/*
Use IsElevated() to determine whether the current process is elevated or not.

Parameters:

pbElevated
	[out] [optional] Pointer to a BOOL variable that, if non-NULL, receives the result.

	The possible values are:

	TRUE - the current process is elevated.
		This value indicates that either UAC is enabled, and the process was elevated by 
		the administrator, or that UAC is disabled and the process was started by a user 
		who is a member of the Administrators group.

	FALSE - the current process is not elevated (limited).
		This value indicates that either UAC is enabled, and the process was started normally, 
		without the elevation, or that UAC is disabled and the process was started by a standard user. 

Return Values
	If the function succeeds, and the current process is elevated, the return value is S_OK. 
	If the function succeeds, and the current process is not elevated, the return value is S_FALSE. 
	If the function fails, the return value is E_FAIL. To get extended error information, 
	call GetLastError().
*/

HRESULT IsElevated( __out_opt BOOL * pbElevated = NULL );

/*
Use RunElevated() to start an elevated process. This function calls ShellExecEx() with the verb "runas" 
to start the elevated process.

Parameters:

hwnd
	[in] Window handle to any message boxes that the system might produce while executing this function.

pszPath
	[in] Address of a null-terminated string that specifies the name of the executable file that 
		should be used to start the process.

pszParameters
	[in] [optional] Address of a null-terminated string that contains the command-line parameters for the process. 
		If NULL, no parameters are passed to the process.

pszDirectory
	[in] [optional] Address of a null-terminated string that specifies the name of the working directory. 
		If NULL, the current directory is used as the working directory. .

Return Values
	If the function succeeds, the return value is TRUE. 
	If the function fails, the return value is FALSE. To get extended error information, 
	call GetLastError().

NOTE: This function will start a process elevated no matter which attribute (asInvoker, 
highestAvailable, or requireAdministrator) is specified in its manifest, and even if 
there is no such attribute at all.
*/

BOOL RunElevated(
			__in		HWND	hwnd, 
			__in		LPCTSTR pszPath, 
			__in_opt	LPCTSTR pszParameters	= NULL, 
			__in_opt	LPCTSTR pszDirectory	= NULL );

BOOL RunMyselfElevated(HWND hwnd);
