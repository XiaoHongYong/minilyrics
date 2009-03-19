
#include "stdafx.h"
#include "filesystemmonitor.h"
#include "filesystemmonitorwin32.h"
#include <process.h>
#include <winioctl.h>
#include "mlstdafx.h"

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

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// cbIsFileIntereasted()
///////////////////////////////////////////////////////////////////////////////
BOOL CFileSystemMonitorWin32::cbIsFileIntereasted(const TCHAR* szFile)
{
	const TCHAR * const aExtensions[]= {_T("mp3"), _T("wav"), _T("ape")};
    const TCHAR* p1 = szFile, *p2 = szFile;

    while(NULL != *p2)
    {
        if(*p2 == _T('.')) p1 = p2 + 1;
        p2++;
    }
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

#ifdef _DEBUG
    FILE * fp = NULL;
    static BOOL bFirstCall = TRUE;
    
    SYSTEMTIME time;
    ::GetSystemTime(&time);

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
        _ftprintf(fp, _T("[%02d:%02d'%03d] %s%s\r\n"), time.wMinute, time.wSecond, time.wMilliseconds, path, file);
        fflush(fp);
    }
    fclose(fp);
#endif // _DEBUG

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
// SearchFolder()
// actually search a folder
///////////////////////////////////////////////////////////////////////////////
void CFileSystemMonitorWin32::SearchFolder(const TCHAR* szStr, /* "C:\" */ BOOL bRoot)
{
    WIN32_FIND_DATA data;
    tstring searchStr = szStr;
    searchStr += _T("*"); // "C:\*"

    while(!IsDiskIdle(NULL)) Sleep(2000);

    HANDLE hFind = FindFirstFile(searchStr.c_str(), &data);
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
                tstring subFolder = szStr;
                subFolder += data.cFileName;
                subFolder += _T("\\");
                SearchFolder(subFolder.c_str(), FALSE);
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
                    tstring subFolder = szStr;
                    subFolder += data.cFileName;
                    subFolder += _T("\\");
                    SearchFolder(subFolder.c_str(), FALSE);
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
///////////////////////////////////////////////////////////////////////////////
// SeachWholeDisk()
// entry of working thread
///////////////////////////////////////////////////////////////////////////////
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
	return 1; // 0 for error
}


///////////////////////////////////////////////////////////////////////////////
// IsDiskIdle()
// to check disk queue length 
///////////////////////////////////////////////////////////////////////////////
BOOL CFileSystemMonitorWin32::IsDiskIdle(const TCHAR* disk)
{
    HANDLE  hDevice;    // handle to the drive to be examined 
    BOOL    bResult;    // results flag
    DWORD   junk;       // discard results
    
    hDevice = CreateFile(_T("\\\\.\\PhysicalDrive0"),  // drive to open
                      0,                // no access to the drive
                      FILE_SHARE_READ | // share mode
                      FILE_SHARE_WRITE, 
                      NULL,             // default security attributes
                      OPEN_EXISTING,    // disposition
                      0,                // file attributes
                      NULL);            // do not copy file attributes
    
    if (INVALID_HANDLE_VALUE != hDevice) // cannot open the drive
    {
        DISK_PERFORMANCE data;

        bResult = DeviceIoControl(hDevice,  // device to be queried
                  IOCTL_DISK_PERFORMANCE,   // operation to perform
                                  NULL, 0,  // no input buffer
                                  &data, sizeof(data),   // output buffer
                                  &junk,                 // # bytes returned
                                  (LPOVERLAPPED) NULL);  // synchronous I/O

        CloseHandle(hDevice);
        return (0 == data.QueueDepth);
    }
    
    return (FALSE);

}
