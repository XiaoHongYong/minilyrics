/********************************************************************
	Created  :	2003/11/2	23:28
	FileName :	App.h
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/


#ifndef _FRAME_WORK_2003_11_2_23_28INCLUDED_
#define _FRAME_WORK_2003_11_2_23_28INCLUDED_

#include "Log.h"
#include "LogAlias.h"

//
// golobal variable
//

// ≈‰÷√Œƒº˛
extern class CProfile			g_Profile;

void GetWorkingFolder(LPTSTR szWorkingFolder);
LPCTSTR GetWorkingFolder();

void SetWorkingFolder(LPCTSTR szWorkingFolder);

#ifdef _WIN32

HINSTANCE GetAppInstance();
void SetAppInstance(HINSTANCE hInst);

HINSTANCE GetResourceHandle();
HINSTANCE SetResourceHandle(HINSTANCE hResHandle);

BOOL InitBaseFrameWork(HINSTANCE hInst, LPCTSTR szLogFile, LPCTSTR szProfileName, LPCTSTR szDefAppName = NULL);
#else
BOOL InitBaseFrameWork(int argc, char *argv[], LPCTSTR szLogFile, LPCTSTR szProfileName, LPCTSTR szDefAppName = NULL);
#endif

#endif // _FRAME_WORK_2003_11_2_23_28INCLUDED_
