#ifndef _FILE_API_INC_
#define _FILE_API_INC_

DECLAR_CPPUNIT_TEST_REG(FileAPI)

#ifndef INVALID_FILE_SIZE
#define INVALID_FILE_SIZE ((DWORD)0xFFFFFFFF) 
#endif

bool filetruncate(FILE *fp, int nLen);

bool IsFileExist(LPCTSTR szFileName);
bool IsDirExist(LPCTSTR szDir);

bool GetFileLength(LPCTSTR szFileName, uint64 &nLength);

#ifndef _WINCE_
DWORD GetFileLength(LPCSTR szFileName);
#endif
DWORD GetFileLength(LPCWSTR szFileName);

#ifdef _WIN32
BOOL
SetFileTime(
    LPCTSTR szFile,
    CONST FILETIME *lpCreationTime,
    CONST FILETIME *lpLastAccessTime,
    CONST FILETIME *lpLastWriteTime
    );
#endif

BOOL CreateDirectoryAll(LPCTSTR szDir);

DWORD CanWriteInDir(LPCTSTR szDir);

BOOL EnumFilesInDir(LPCTSTR szBaseDir, LPCTSTR szFilter, vector<tstring> &vFiles, BOOL bEnumFullPath);

typedef void (*FUNProcessFile)(LPCTSTR szFileName, LPCTSTR szDir, LPVOID lpUserData);

BOOL ProcessFilesInfolder(LPCTSTR szBaseDir, FUNProcessFile funProc, LPVOID lpUserData);

void DirStringAddSlash(WCHAR *szDir);

void DirStringAddSlash(char *szDir);

void DirStringAddSlash(tstring &strDir);

tstring DirStringJoint(LPCTSTR szDir, LPCTSTR szSubFileDir);

#define DirStringFormat		DirStringAddSlash

void GetRelatedPath(LPCTSTR szDir, LPCTSTR szBaseDir, LPTSTR szRelatedPath, int nRelatedPathLen);

LPCTSTR UrlGetName(LPCTSTR szFile);

tstring UrlGetTitle(LPCTSTR szFile);

LPCTSTR FileGetName(LPCTSTR szFile);

void FileGetTitle(LPTSTR szBuff, int nBuffLen, LPCTSTR szFile);
void FileGetTitle(LPCTSTR szFile, tstring &strFileTitle);

void FileGetPath(LPCTSTR szFile, tstring &strPath);
void FileGetPath(LPTSTR szBuff, int nBufLen, LPCTSTR szFile);

void FileSetExt(tstring &strFile, LPCTSTR szExt);
void FileSetExt(LPTSTR szFile, int nFileBufLen, LPCTSTR szExt);
LPCTSTR FileGetExt(LPCTSTR szFile);

BOOL FileIsExtSame(LPCTSTR szFile, LPCTSTR szExt);

BOOL FileNameIsIncInvalidChars(LPCTSTR szFile);
tstring FileNameFilterInvalidChars(LPCTSTR szFile);
void FilePathFilterInvalidChars(LPTSTR szFile);

#ifndef WIN32
bool DeleteFile(LPCTSTR lpFileName);
bool CreateDirectory(LPCSTR lpPathName);
#endif


#ifdef WIN32

BOOL IsDiskIdle(const TCHAR* disk);
void GetLogicalDrives(V_Str &vDrives);
#endif


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

protected:
	HANDLE				hFileFind;
	WIN32_FIND_DATA		FindData;
	bool				bFirst;

};

#endif // _FILE_API_INC_
