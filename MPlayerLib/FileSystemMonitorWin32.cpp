
#include "stdafx.h"
#include "filesystemmonitor.h"
#include "filesystemmonitorwin32.h"
#include <process.h>


///////////////////////////////////////////////////////////////////////////////
// StartFullDiskSearch()
// create a low priority task to start search 
///////////////////////////////////////////////////////////////////////////////
BOOL CFileSystemMonitorWin32::StartFullDiskSearch(void)
{
	HANDLE m_hWorkingThread = (HANDLE)_beginthreadex(NULL, 0, cbFullDiskSearchThread, NULL, CREATE_SUSPENDED, NULL);
	if(m_hWorkingThread)
	{
		m_bDiskQueueIdle = FALSE;
		ASSERT(SetThreadPriority(m_hWorkingThread, THREAD_PRIORITY_IDLE));
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}




///////////////////////////////////////////////////////////////////////////////
// TurnOnFileSystemMonitor()
///////////////////////////////////////////////////////////////////////////////
BOOL CFileSystemMonitorWin32::TurnOnFileSystemMonitor(BOOL bTurnOn)
{
	if(bTurnOn && !m_bMonitorOn)
	{
		// turn on the monitor in win32
	}
	else if(!bTurnOn && m_bMonitorOn)
	{
		// turn off the monitor in win32
	}
	// else do nothing

	return (m_bMonitorOn = bTurnOn);
}




///////////////////////////////////////////////////////////////////////////////
// cbIsFileIntereasted()
///////////////////////////////////////////////////////////////////////////////
BOOL CFileSystemMonitorWin32::cbIsFileIntereasted(TCHAR* extension)
{
	const TCHAR * const aExtensions[]= {_T("mp3"), _T("wav"), _T("ape")};
	int i = SIZEOFARRAY(aExtensions);
	
	while(--i > 0)
	{
		if(0 == _tcsicmp(aExtensions[i], extension)) break;
	}

	return (-1 != i);
}

///////////////////////////////////////////////////////////////////////////////
// cbOnNewFile()
// media library will set call back function to add or update media info
///////////////////////////////////////////////////////////////////////////////
void CFileSystemMonitorWin32::cbOnNewFile(TCHAR* path, TCHAR* file)
{
}
///////////////////////////////////////////////////////////////////////////////
// cbOnEndOfFolder()
// media library will set call back function to remove all existing items under this folder
///////////////////////////////////////////////////////////////////////////////
void CFileSystemMonitorWin32::cbOnEndOfFolder(TCHAR* path)
{
}

///////////////////////////////////////////////////////////////////////////////
// cbFullDiskSearchThread()
// break down the directory and check the hard disk queue from time to time
///////////////////////////////////////////////////////////////////////////////
unsigned int __stdcall cbFullDiskSearchThread(void* param)
{
	return 0; // error
}



