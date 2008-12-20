#ifndef _FILE_API_INC_
#define _FILE_API_INC_

#ifndef INVALID_FILE_SIZE
#define INVALID_FILE_SIZE ((DWORD)0xFFFFFFFF) 
#endif

bool filetruncate(FILE *fp, int nLen);

bool IsFileExist(LPCTSTR szFileName);
bool IsDirExist(LPCTSTR szDir);

bool GetFileLength(LPCTSTR szFileName, uint64 &nLength);

DWORD GetFileLength(LPCSTR szFileName);

#ifdef _WIN32
DWORD GetFileLength(LPCWSTR szFileName);

BOOL
SetFileTime(
    LPCTSTR szFile,
    CONST FILETIME *lpCreationTime,
    CONST FILETIME *lpLastAccessTime,
    CONST FILETIME *lpLastWriteTime
    );
#endif

BOOL CreateDirectoryAll(LPCTSTR szDir);

BOOL EnumFilesInDir(LPCTSTR szBaseDir, LPCTSTR szFilter, vector<tstring> &vFiles, BOOL bEnumFullPath);

typedef void (*FUNProcessFile)(LPCTSTR szFileName, LPCTSTR szDir, LPVOID lpUserData);

BOOL ProcessFilesInfolder(LPCTSTR szBaseDir, FUNProcessFile funProc, LPVOID lpUserData);

void DirStringFormat(WCHAR *szDir);

void DirStringFormat(char *szDir);

void DirStringFormat(tstring &strDir);

void GetRelatedPath(LPCTSTR szDir, LPCTSTR szBaseDir, LPTSTR szRelatedDir);

void UrlFileGetName(LPTSTR szBuff, int nBuffLen, LPCTSTR szFile);

void FileGetName(LPTSTR szBuff, int nBuffLen, LPCTSTR szFile);

void FileGetTitle(LPTSTR szBuff, int nBuffLen, LPCTSTR szFile);

// without ext
void FileGetPath(LPTSTR szBuff, int nBufLen, LPCTSTR szFile);

void FileGetPath(LPCTSTR szFile, tstring &strPath);

LPCTSTR FileGetExt(LPCTSTR szFile);
void FileGetExt(LPCTSTR szFile, LPTSTR szExt, int nLenExtMax);
// Set file ext
// "d:\mp3\test.mp3 +++ .wav -----> d:\mp3\test.wav"
void FileSetExt(LPTSTR szFile, LPCTSTR szExt);

BOOL FileIsExtSame(LPCTSTR szFile, LPCTSTR szExt);

void FileNameFilterInvalidChars(LPTSTR szFile);
BOOL FileNameIsIncInvalidChars(LPCTSTR szFile);
void FileNameFilterInvalidChars(LPCTSTR szFile, LPTSTR szNewFile, int nSizeMax);
void FilePathFilterInvalidChars(LPTSTR szFile);

#ifndef WIN32
bool DeleteFile(LPCTSTR lpFileName);
bool CreateDirectory(LPCSTR lpPathName);
bool MoveFile(const char *oldname, const char *newname);
#endif

class CMLFileFind
{
public:
	CMLFileFind();
	virtual ~CMLFileFind();

	bool OpenDir(LPCTSTR szDir, LPCTSTR szFilter = NULL);

	void Close();

	bool FindNext();

	LPCTSTR GetCurName() { return m_dirp->d_name; }

	bool IsCurDir();

protected:
	bool GetCurStat();

	dirent		*m_dirp;
	DIR			*m_dp;
	bool		m_bStatValid;
	struct stat	m_stat;
	TCHAR		m_szDir[MAX_PATH];

};

#endif // _FILE_API_INC_
