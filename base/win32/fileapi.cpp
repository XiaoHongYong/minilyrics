#include "stdafx.h"
#include "base.h"
#include "safestr.h"
#include "base.h"
#include "fileapi.h"

// Truncate the tail of file by nLen.
#ifdef _WINCE_
bool filetruncate(FILE *fp, int nLen)
{
	return false;
}
#else
#include "io.h"
bool filetruncate(FILE *fp, int nLen)
{
	if (fseek(fp, nLen, SEEK_SET) != 0)
		return false;

	HANDLE	hFile;
	hFile = (HANDLE)_get_osfhandle(fp->_file);
	if (hFile)
	{
		if (!SetEndOfFile(hFile))
			return false;
	}

	return true;
}
#endif

bool IsFileExist(LPCTSTR szFileName)
{
    return (GetFileAttributes(szFileName) != -1);
}

bool IsDirExist(LPCTSTR szDir)
{
	DWORD	dwRet;

	dwRet = GetFileAttributes(szDir);

	return (dwRet != -1 && (dwRet & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);
}

bool GetFileLength(LPCTSTR szFileName, uint64 &nLength)
{
	HANDLE				hFileFind;
	WIN32_FIND_DATA		FindData;
	hFileFind = FindFirstFile(szFileName, &FindData);
	if (hFileFind != INVALID_HANDLE_VALUE)
	{
		if (FindData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
		{
			LARGE_INTEGER		Length;
			Length.LowPart = FindData.nFileSizeLow;
			Length.HighPart = FindData.nFileSizeHigh;
			nLength = Length.QuadPart;
			FindClose(hFileFind);
			return true;
		}
	}

	FindClose(hFileFind);
	return false;
}

#ifndef _WINCE_
// RETURN:
//		INVALID_FILE_SIZE	- Invalid file size
DWORD GetFileLength(LPCSTR szFileName)
{
	HANDLE				hFileFind;
	WIN32_FIND_DATAA	FindData;
	DWORD				dwSize = INVALID_FILE_SIZE;

	hFileFind = FindFirstFileA(szFileName, &FindData);
	if (hFileFind != INVALID_HANDLE_VALUE)
	{
		if (FindData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
		{
			dwSize = FindData.nFileSizeLow;
		}
	}

	FindClose(hFileFind);

	return dwSize;
}
#endif

DWORD GetFileLength(LPCWSTR szFileName)
{
	HANDLE				hFileFind;
	WIN32_FIND_DATAW	FindData;
	DWORD				dwSize = INVALID_FILE_SIZE;

	hFileFind = FindFirstFileW(szFileName, &FindData);
	if (hFileFind != INVALID_HANDLE_VALUE)
	{
		if (FindData.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
		{
			dwSize = FindData.nFileSizeLow;
		}
	}

	FindClose(hFileFind);

	return dwSize;
}

bool CreateDirectory(LPCTSTR lpPathName)
{
	return Tobool(CreateDirectory(lpPathName, NULL));
}

BOOL SetFileTime(
    LPCTSTR szFile,
    CONST FILETIME *lpCreationTime,
    CONST FILETIME *lpLastAccessTime,
    CONST FILETIME *lpLastWriteTime
    )
{
	HANDLE	hFile;
	BOOL	bRet;

	hFile = CreateFile(szFile, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, 0, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	bRet = SetFileTime(hFile, lpCreationTime, lpLastAccessTime, lpLastWriteTime);

	CloseHandle(hFile);

	return bRet;
}

DWORD CanWriteInDir(LPCTSTR szDir)
{
	DWORD	dwAttr;

	dwAttr = GetFileAttributes(szDir);
	if (dwAttr == 0xFFFFFFFF)
		return FALSE;

	if (!SetFileAttributes(szDir, dwAttr))
		return FALSE;

	// Try to create a file in the folder, then delete it.
	tstring strFile = szDir;
	DirStringAddSlash(strFile);
	strFile += _T("temp.txt");
	if (!SaveDataAsFile(strFile.c_str(), "abc", 3))
		return FALSE;
	DeleteFile(strFile.c_str());

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

CMLFileFind::CMLFileFind()
{
	hFileFind = INVALID_HANDLE_VALUE;
	bFirst = false;
	memset(&FindData, 0, sizeof(FindData));
}

CMLFileFind::~CMLFileFind()
{
	if (hFileFind)
		FindClose(hFileFind);
}

bool CMLFileFind::OpenDir(LPCTSTR szDir, LPCTSTR szFilter)
{
	if (!szFilter)
		szFilter = _T("*.*");

	tstring		str;
	str = szDir;
	DirStringAddSlash(str);
	str += szFilter;

	hFileFind = FindFirstFile(str.c_str(), &FindData);
	if (hFileFind == INVALID_HANDLE_VALUE)
		return false;

	bFirst = true;

	// Remove _T(".") and _T("..") from result.
	while (IsFlagSet(FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY)
		&& IsCurOrParentDir(FindData.cFileName))
	{
		if (!FindNextFile(hFileFind, &FindData))
		{
			Close();
			return false;
		}
	}

	return true;
}

void CMLFileFind::Close()
{
	if (hFileFind)
	{
		FindClose(hFileFind);
		hFileFind = NULL;
	}
}

bool CMLFileFind::FindNext()
{
	if (bFirst)
	{
		bFirst = false;
		return true;
	}

	if (!FindNextFile(hFileFind, &FindData))
		return false;

	// Remove _T(".") and _T("..") from result.
	while (IsFlagSet(FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY)
		&& IsCurOrParentDir(FindData.cFileName))
	{
		if (!FindNextFile(hFileFind, &FindData))
			return false;
	}

	return true;
}

#include "Winioctl.h"
#include "base.h"

void GetLogicalDrives(V_Str &vDrives)
{
#define MAX_DRIVER_SUPPORT  ('Z' - 'A' + 1)
#define DRIVE_STR_BUF_SIZE	((3 + 1) * MAX_DRIVER_SUPPORT + 1)
#define DRIVE_STR_STR_LEN	(3 * MAX_DRIVER_SUPPORT)

	TCHAR aLogicDiskString[DRIVE_STR_BUF_SIZE] = { 0 };
	DWORD len = GetLogicalDriveStrings(DRIVE_STR_STR_LEN, aLogicDiskString);

	if (len > 0)
		MultiStrToVStr(aLogicDiskString, vDrives);

}

// Check disk queue length 
BOOL IsDiskIdle(const TCHAR* disk)
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

	if (INVALID_HANDLE_VALUE == hDevice)
		return FALSE;

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
