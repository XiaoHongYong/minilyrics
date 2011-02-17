
#include "stdafx.h"
#include "FullDiskSearch.h"
#include "base.h"
#include <process.h>
#include <winioctl.h>

//#include "mutex.h"


///////////////////////////////////////////////////////////////////////////////
// StartFullDiskSearch()
// create a low priority task to start search 
///////////////////////////////////////////////////////////////////////////////
BOOL CFullDiskSearch::Start(void)
{
	if (!m_threadWorking.Create(SeachWholeDisk, this))
		return FALSE;

	m_threadWorking.SetPriority(THREAD_PRIORITY_IDLE);

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// cbIsFileIntereasted()
///////////////////////////////////////////////////////////////////////////////
BOOL CFullDiskSearch::cbIsFileIntereasted(const TCHAR* szFile)
{
	const TCHAR * const aExtensions[] = {_T("mp3"), _T("wav"), _T("ape")};
	const TCHAR* szExt = NULL;

	szExt = FileGetExt(szFile);

	if(_T('\0') == szExt[0])
		return FALSE;


	for (int i = 0; i < CountOf(aExtensions); i++)
	{
		if (0 == _tcsicmp(aExtensions[i], szExt))
			return TRUE;
	}

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// cbOnNewFile()
// media library will set call back function to add or update media info
///////////////////////////////////////////////////////////////////////////////
void CFullDiskSearch::cbOnNewFile(const TCHAR* path, const TCHAR* file)
{
	// User Debug tracer tool to view the log instantly.
	DBGLOG2(_T("NewFile: %s%s"), path, file);
}

///////////////////////////////////////////////////////////////////////////////
// cbOnNewFile()
// media library will set call back function to add or update media info
///////////////////////////////////////////////////////////////////////////////
void CFullDiskSearch::cbOnRemoveFile(const TCHAR* path, const TCHAR* file)
{
}

///////////////////////////////////////////////////////////////////////////////
// cbOnEndOfFolder()
// media library will set call back function to remove all existing items under this folder
///////////////////////////////////////////////////////////////////////////////
void CFullDiskSearch::cbOnEndOfFolder(const TCHAR* path)
{
	// update media library and notify UI
	DBGLOG1(_T("End of scan folder: %s"), path);
}

///////////////////////////////////////////////////////////////////////////////
// SearchFolder()
// actually search a folder
///////////////////////////////////////////////////////////////////////////////
void CFullDiskSearch::SearchFolder(const TCHAR* szStr, /* "C:\" */ BOOL bRoot)
{
	CMLFileFind		find;

	while (!IsDiskIdle(NULL))
		Sleep(2000);

	if (!find.OpenDir(szStr))
		return;

	tstring searchStr = szStr;

	while (find.FindNext())
	{
		if (find.IsCurDir())
		{
			tstring subFolder = szStr;
			subFolder += find.GetCurName();
			subFolder += _T(SZ_DIR_SLASH);
			SearchFolder(subFolder.c_str(), FALSE);
		}
		else if (cbIsFileIntereasted(find.GetCurName()))
		{
			// ???? Should sleep on adding every file?
			cbOnNewFile(szStr, find.GetCurName());
		}
	}

	cbOnEndOfFolder(szStr);
}

///////////////////////////////////////////////////////////////////////////////
// SeachWholeDisk()
// entry of working thread
///////////////////////////////////////////////////////////////////////////////
void CFullDiskSearch::SeachWholeDisk(void* param)
{
	CFullDiskSearch* p = (CFullDiskSearch*)param;
	// enum all disk with the type

	V_Str		vDrives;

	GetLogicalDrives(vDrives);

	for (unsigned int i = 0; i < vDrives.size(); i++)
	{
		// search this disk
		p->SearchFolder(vDrives[i].c_str(), TRUE);
	}
}

