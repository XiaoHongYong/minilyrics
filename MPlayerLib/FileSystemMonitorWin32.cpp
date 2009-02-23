
#include "stdafx.h"
#include "filesystemmonitor.h"
#include "filesystemmonitorwin32.h"
#include <process.h>
//#include "mutex.h"


///////////////////////////////////////////////////////////////////////////////
// StartFullDiskSearch()
// create a low priority task to start search 
///////////////////////////////////////////////////////////////////////////////
BOOL CFileSystemMonitorWin32::StartFullDiskSearch(void)
{
	m_hWorkingThread = (HANDLE)_beginthreadex(NULL, 0, SeachWholeDisk, this, CREATE_SUSPENDED, NULL);

	if(m_hWorkingThread)
	{
		m_bDiskQueueIdle = TRUE;
		ASSERT(SetThreadPriority(m_hWorkingThread, THREAD_PRIORITY_IDLE));
        ResumeThread(m_hWorkingThread);
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
		// turn on

	    TCHAR aLogicDiskString[DRIVE_STR_BUF_SIZE] = {0};
	    DWORD len = GetLogicalDriveStrings(DRIVE_STR_STR_LEN, aLogicDiskString);
	    TCHAR* szStr = aLogicDiskString;

	    if(len < DRIVE_STR_STR_LEN)
	    {
		    // c:\<null>d:\<null><null> //
		    while((NULL != *szStr) && (m_count < SIZEOFARRAY(m_aHandle)))
		    {
			    // create a thread to wait
	            m_aHandle[m_count] = (HANDLE)_beginthreadex(NULL, 0, MonitorAFolder, szStr, 0, NULL);
                if(m_aHandle[m_count]) m_count++;
			    szStr += _tcslen(szStr) + 1; // jump to next 
		    }
	    }

    }
	else if(!bTurnOn && m_bMonitorOn)
	{
		// turn off
        while(m_count-- > 0)
        {
            CloseHandle(m_aHandle[m_count]);
        }
	}

	return (m_bMonitorOn = bTurnOn);
}




///////////////////////////////////////////////////////////////////////////////
// cbIsFileIntereasted()
///////////////////////////////////////////////////////////////////////////////
BOOL CFileSystemMonitorWin32::cbIsFileIntereasted(const TCHAR* szFile)
{
    const TCHAR* p1 = szFile, *p2 = szFile;

    while(NULL != *p2)
    {
        if(*p2 == _T('.')) p1 = p2 + 1;
        p2++;
    }
	const TCHAR * const aExtensions[]= {_T("mp3"), _T("wav"), _T("ape")};
	int i = SIZEOFARRAY(aExtensions);
	
	while(i-- > 0)
	{
		if(0 == _tcsicmp(aExtensions[i], p1)) break;
	}

	return (-1 != i);
}

///////////////////////////////////////////////////////////////////////////////
// cbOnNewFile()
// media library will set call back function to add or update media info
///////////////////////////////////////////////////////////////////////////////
void CFileSystemMonitorWin32::cbOnNewFile(const TCHAR* path, const TCHAR* file)
{
    FILE * fp = NULL;
    static BOOL bFirstCall = TRUE;
    

    if(0 == _tcscmp(path, _T("C:\\My Music\\")))
    {
        int j = 0;
    }


    if(bFirstCall)
    {
        fp = _tfopen(_T("C:\\music_log.txt"), _T("w+t"));
        bFirstCall = FALSE; 
    }
    else
    {
        fp = _tfopen(_T("C:\\music_log.txt"), _T("at"));
    }

    if(fp)
    {
        _ftprintf(fp, _T("%s%s\r\n"), path, file);
        fflush(fp);
    }

	// update media library and notify UI
}

///////////////////////////////////////////////////////////////////////////////
// cbOnNewFile()
// media library will set call back function to add or update media info
///////////////////////////////////////////////////////////////////////////////
void CFileSystemMonitorWin32::cbOnRemoveFile(const TCHAR* path, const TCHAR* file)
{
}

///////////////////////////////////////////////////////////////////////////////
// cbOnEndOfFolder()
// media library will set call back function to remove all existing items under this folder
///////////////////////////////////////////////////////////////////////////////
void CFileSystemMonitorWin32::cbOnEndOfFolder(const TCHAR* path)
{
	// update media library and notify UI
}

///////////////////////////////////////////////////////////////////////////////
// cbFullDiskSearchThread()
// break down the directory and check the hard disk queue from time to time
///////////////////////////////////////////////////////////////////////////////
void CFileSystemMonitorWin32::SearchFolder(const TCHAR* szStr, BOOL bRoot)
{
    WIN32_FIND_DATA data;

    TCHAR searchStr[MAX_PATH + 1] = {0};

    // "C:\*"
    _tcsncpy(searchStr, szStr, MAX_PATH - 1);
    _tcscat(searchStr, _T("*"));

    TCHAR subFolder[MAX_PATH + 1] = {0};


    HANDLE hFind = FindFirstFile(searchStr, &data);
    if (INVALID_HANDLE_VALUE != hFind) 
    {
        if(FILE_ATTRIBUTE_DIRECTORY == data.dwFileAttributes )
        {
			if( (_T('.') == data.cFileName[0]) && 
				((_T('\0') == data.cFileName[1]) || ((_T('.') == data.cFileName[1]) && (_T('\0') == data.cFileName[2]))))
			{
				// do nothing for "." and ".."
			}
			else
            {
                _tcscpy(subFolder, szStr);
                _tcscat(subFolder, data.cFileName);
                _tcscat(subFolder, _T("\\"));
                SearchFolder(subFolder, FALSE);
            }
        }
        else
        {
			if(cbIsFileIntereasted(data.cFileName ))
			{
				cbOnNewFile(szStr, data.cFileName);
			}
        }

        while(FindNextFile(hFind, &data))
        {
            if(FILE_ATTRIBUTE_DIRECTORY == data.dwFileAttributes )
            {
                if(bRoot)
                {
                    int i = 0;
                }


			    if( (_T('.') == data.cFileName[0]) && 
				    ((_T('\0') == data.cFileName[1]) || ((_T('.') == data.cFileName[1]) && (_T('\0') == data.cFileName[2]))))
			    {
				    // do nothing for "." and ".."
			    }
			    else
                {
                    _tcscpy(subFolder, szStr);
                    _tcscat(subFolder, data.cFileName);
                    _tcscat(subFolder, _T("\\"));
                    SearchFolder(subFolder, FALSE);
                }
            }
            else
            {
			    if(cbIsFileIntereasted(data.cFileName ))
			    {
				    cbOnNewFile(szStr, data.cFileName);
			    }
            }
        }// while

        cbOnEndOfFolder(szStr);
    }// if 

}
unsigned int __stdcall CFileSystemMonitorWin32::SeachWholeDisk(void* param)
{
    CFileSystemMonitorWin32* p = (CFileSystemMonitorWin32*)param;
	// enum all disk with the type

	TCHAR aLogicDiskString[DRIVE_STR_BUF_SIZE] = {0};
	DWORD len = GetLogicalDriveStrings(DRIVE_STR_STR_LEN, aLogicDiskString);
	TCHAR* szStr = aLogicDiskString;

	if(len < DRIVE_STR_STR_LEN)
	{
		// c:\<null>d:\<null><null> //
		while(NULL != *szStr)
		{
			// search this disk
			p->SearchFolder(szStr, TRUE);

			szStr += _tcslen(szStr) + 1; // jump to next 
		}
	}
	return 0; // 0 for error
}

unsigned int __stdcall CFileSystemMonitorWin32::MonitorAFolder(void* param)
{
    return 0; // 0 for error
}
