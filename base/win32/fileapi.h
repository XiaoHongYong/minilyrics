#pragma once

BOOL
SetFileTime(
    LPCTSTR szFile,
    CONST FILETIME *lpCreationTime,
    CONST FILETIME *lpLastAccessTime,
    CONST FILETIME *lpLastWriteTime
    );

BOOL IsDiskIdle(const TCHAR* disk);
void GetLogicalDrives(V_Str &vDrives);

class CMLFileFind
{
public:
	CMLFileFind();
	virtual ~CMLFileFind();

	bool OpenDir(LPCTSTR szDir, LPCTSTR szFilter = NULL);

	void Close();

	bool FindNext();

	LPCTSTR GetCurName()
		{ return FindData.cFileName; }

	bool IsCurDir()
		{ return IsFlagSet(FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY); }

	uint32 GetFileSize()
		{ return FindData.nFileSizeLow; }

protected:
	HANDLE				hFileFind;
	WIN32_FIND_DATA		FindData;
	bool				bFirst;

};
