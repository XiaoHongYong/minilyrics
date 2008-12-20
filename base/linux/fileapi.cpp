#include "stdafx.h"
#include "safestr.h"
#include "fileapi.h"


bool filetruncate(FILE *fp, int nLen)
{
	if (ftruncate(fileno(fp), nLen) == -1)
		return false;

	return true;
}

// PURPOSE: to judege whether the specified file or path exist
//
// PARAMETERS:
//      szFileName - the file or path
//
bool IsFileExist(LPCTSTR szFileName)
{
	struct stat		filestat;

	memset(&filestat, 0, sizeof(filestat));
	return stat(szFileName, &filestat) == 0;
}

bool IsDirExist(LPCTSTR szDir)
{
	DIR			*dp;
	dirent		*dirp;

	dp = opendir(szDir);
	if (dp == NULL)
	{
		return false;
	}

	return true;
}

// COMMENT:
//		取得文件的大小
// RETURN:
//		INVALID_FILE_SIZE	-	无效的大小，文件可能无效
bool GetFileLength(LPCTSTR szFileName, uint64 &nLength)
{
	struct stat		filestat;
	int		ret;

	memset(&filestat, 0, sizeof(filestat));
	ret = stat(szFileName, &filestat);
	if (ret == 0)
	{
		nLength = filestat.st_size;
		return true;
	}
	else
	{
		nLength = 0;
		return false;
	}
}

// COMMENT:
//		取得文件的大小
// RETURN:
//		INVALID_FILE_SIZE	-	无效的大小，文件可能无效
DWORD GetFileLength(LPCSTR szFileName)
{
	struct stat		filestat;
	int		ret;

	memset(&filestat, 0, sizeof(filestat));
	ret = stat(szFileName, &filestat);
	if (ret == 0)
	{
		return filestat.st_size;
	}
	else
	{
		return INVALID_FILE_SIZE;
	}
}


bool DeleteFile(LPCTSTR lpFileName)
{
	if (unlink(lpFileName) == 0)
		return true;

	return false;
}

bool MoveFile(const char *oldname, const char *newname)
{
	return rename(oldname, newname) == 0;
}

/*

// 设置文件的修改时间和创建时间
BOOL SetFileTime(LPCTSTR szFile, FILETIME *lpWriteTime)
{
	HANDLE	hFile;
	BOOL	bRet;

	hFile = CreateFile(szFile, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, 0, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	bRet = SetFileTime(hFile, lpWriteTime, NULL, lpWriteTime);

	CloseHandle(hFile);

	return bRet;
}
*/
bool CreateDirectory(LPCSTR lpPathName)
{
	return mkdir(lpPathName, 0) == 0;
}

BOOL CreateDirectoryAll(LPCTSTR szDir)
{
	TCHAR	szTemp[512];
	TCHAR	*szBeg;

	_tcscpy(szTemp, szDir);

	szBeg = _tcschr(szTemp, '/');
	while (szBeg != NULL)
	{
		*szBeg = '\0';

		if (!IsDirExist(szTemp))
		{
			if (!CreateDirectory(szTemp))
				return FALSE;
		}

		*szBeg = '/';
		szBeg = _tcschr(szBeg + 1, '/');
	}

	if (!IsDirExist(szDir))
	{
		if (!CreateDirectory(szDir))
			return FALSE;
	}

	return TRUE;
}

// COMMENT:
//		判断目录内是否可以写
DWORD CanWriteInDir(LPCTSTR szDir)
{
	return true;
}

BOOL EnumFilesInDir(LPCTSTR szBaseDir, LPCTSTR szFilter, vector<tstring> &vFiles, BOOL bEnumFullPath)
{
	CMLFileFind			find;

	if (!find.OpenDir(szBaseDir, szFilter))
		return FALSE;

	while (find.FindNext())
	{
		if (!find.IsCurDir())
		{
			if (bEnumFullPath)
			{
				TCHAR	szFile[MAX_PATH];

				_tcscpy(szFile, szBaseDir);
				DirStringFormat(szFile);
				_tcscat(szFile, find.GetCurName());

				vFiles.push_back(szFile);
			}
			else
				vFiles.push_back(find.GetCurName());
		}
	}

	return TRUE;
}


// typedef void (*FUNProcessFile)(LPCTSTR szFileName, LPCTSTR szDir, LPVOID lpUserData);
//	TCHAR		szFile[MAX_PATH];
//
//	strcpy(szFile, szDir);
//	DirStringFormat(szFile);
//	_tcscat(szFile, szFileName);
BOOL ProcessFilesInfolder(LPCTSTR szBaseDir, FUNProcessFile funProc, LPVOID lpUserData)
{
	CMLFileFind			find;
	TCHAR				szSearch[MAX_PATH];

	if (!find.OpenDir(szBaseDir, NULL))
		return FALSE;

	while (find.FindNext())
	{
		if (!find.IsCurDir())
		{
			funProc(find.GetCurName(), szBaseDir, lpUserData);
		}
		else if (_tcscmp(find.GetCurName(), _T(".")) != 0
			&& _tcscmp(find.GetCurName(), _T("..")) != 0)
		{
			TCHAR		szSubDir[MAX_PATH];
			_tcscpy(szSubDir, szBaseDir);
			DirStringFormat(szSubDir);
			_tcscat(szSubDir, find.GetCurName());
			DirStringFormat(szSubDir);
			ProcessFilesInfolder(szSubDir, funProc, lpUserData);
		}
	}

	return TRUE;
}

// COMMENT:
//		将目录字符串格式化为末尾带'/'的
void DirStringFormat(WCHAR *szDir)
{
	int		nLen;

	nLen = wcslen(szDir);
	if (nLen > 0)
	{
		if (szDir[nLen - 1] != DIR_SLASH)
		{
			szDir[nLen] = DIR_SLASH;
			szDir[nLen + 1] = '\0';
		}
	}
}

// COMMENT:
//		将目录字符串格式化为末尾带'/'的
void DirStringFormat(char *szDir)
{
	int		nLen;

	nLen = strlen(szDir);
	if (nLen > 0)
	{
		if (szDir[nLen - 1] != DIR_SLASH)
		{
			szDir[nLen] = DIR_SLASH;
			szDir[nLen + 1] = '\0';
		}
	}
}

// COMMENT:
//		将目录字符串格式化为末尾带'/'的
void DirStringFormat(string &strDir)
{
	if (!strDir.empty())
	{
		if (strDir[strDir.size() - 1] != DIR_SLASH)
			strDir += DIR_SLASH;
	}
}

void GetRelatedPath(LPCTSTR szDir, LPCTSTR szBaseDir, LPTSTR szRelatedDir)
{
	int		nBaseLen = _tcslen(szBaseDir);
	if (_tcsnicmp(szBaseDir, szDir, nBaseLen) == 0)
	{
		if (szDir[nBaseLen] == DIR_SLASH)
			nBaseLen++;
		_tcscpy(szRelatedDir, szDir + nBaseLen);
	}
	else
	{
		assert(0 && "GetRelatedPath()");
		_tcscpy(szRelatedDir, szDir);
	}
}

// format string pathname ---> filename
// "c:\windows\command.com" ----> "command.com"
void FileGetName(LPTSTR szBuff, int nBuffLen, LPCTSTR szFile)
{
	LPCTSTR		szBeg;

	szBeg = _tcsrchr(szFile, DIR_SLASH);
	if (szBeg)
	{
		_tcscpy_safe(szBuff, nBuffLen, szBeg + 1);
	}
	else
	{
		_tcscpy_safe(szBuff, nBuffLen, szFile);
	}
}

// format string pathname ---> filename
// "http://www.minilyrics.com/windows/command.com" ----> "command.com"
void UrlFileGetName(LPTSTR szBuff, int nBuffLen, LPCTSTR szFile)
{
	LPCTSTR		szBeg;

	szBeg = _tcsrchr(szFile, '/');
	if (szBeg)
	{
		_tcscpy_safe(szBuff, nBuffLen, szBeg + 1);
	}
	else
	{
		_tcscpy_safe(szBuff, nBuffLen, szFile);
	}
}

// format string pathname ---> filename
// "c:\windows\command.com" ----> "command"
void FileGetTitle(LPTSTR szBuff, int nBuffLen, LPCTSTR szFile)
{
	LPCTSTR		szBeg, szEnd;
	int			nLen;

	szEnd = _tcsrchr(szFile, '.');

	szBeg = _tcsrchr(szFile, DIR_SLASH);
	if (szBeg == NULL)
		szBeg = szFile;
	else
		szBeg++;

	if (szEnd == NULL || szBeg > szEnd)
		nLen = szFile - szBeg + _tcslen(szFile);
	else
		nLen = szEnd - szBeg;

	_tcsncpy_safe(szBuff, nBuffLen, szBeg, nLen);
	szBuff[nLen] = '\0';
}
/*   Old function
void FileGetTitle(LPTSTR szBuff, int nBuffLen, LPCTSTR szFile)
{
	int i = 0;
	int ilen = _tcslen(szFile);

	ilen --;
	while (ilen >= 0)
	{
		if (szFile[ilen --] == '.')
			break;
	}

	i = 0;
	while (ilen >= 0)
	{
		if (szFile[ilen] != DIR_SLASH)
		{
			szBuff[i] = szFile[ilen];
			ilen --;
			i ++;
		}
		else
			break;
	}

	szBuff[i] = '\0';

	ilen = i - 1;
	int imid = i / 2;
	TCHAR	chTemp;
	for (i = 0; i < imid; i++)
	{
		chTemp = szBuff[ilen - i];
		szBuff[ilen - i] = szBuff[i];
		szBuff[i] = chTemp;
	}
}*/

// GetPath Only
// "d:\mp3\test.mp3 -----> d:\mp3\"
void FileGetPath(LPTSTR szBuff, LPCTSTR szFile)
{
	int ilen = _tcslen(szFile);
	ilen --;
	while (ilen >= 0)
	{
		if (szFile[ilen] == DIR_SLASH)
			break;
		ilen--;
	}

	if (ilen >= 0)
		_tcsncpy(szBuff, szFile, ilen + 1);
	szBuff[ilen + 1] = '\0';
}

// GetPath Only
// "d:\mp3\test.mp3 -----> d:\mp3\"
void FileGetPath(LPTSTR szBuff)
{
	LPTSTR sz;
	sz = szBuff + _tcslen(szBuff);
	while (sz >= szBuff && *sz != DIR_SLASH) sz --;
	sz ++;
	*sz = 0;
}

// 取得文件名的扩展名
// "d:\mp3\test.mp3 ----> .mp3"
// "d:\mp3\test ----> "
// "d:\mp.3\test ----> "
void FileGetExt(LPCTSTR szFile, LPTSTR szExt, int nLenExtMax)
{
	LPCTSTR		szBeg;

	szBeg = _tcsrchr(szFile, '.');
	if (szBeg == NULL)
		szExt[0] = '\0';
	else
	{
		LPCTSTR		szTemp;

		szTemp = _tcsrchr(szFile, DIR_SLASH);
		if (szTemp == NULL || szTemp < szBeg)
			_tcscpy_safe(szExt, nLenExtMax, szBeg);
		else
			szExt[0] = '\0';
	}
}

// COMMENT:
//		设置文件的扩展名
//		"d:\mp3\test.mp3 +++ .wav -----> d:\mp3\test.wav"
//		"d:\mp3\test     .wav   -----> d:\mp3\test.wav
//		"d:\mp3\test.mp3\test.mp3     .wav   -----> d:\mp3\test.mp3\test.wav
//		test        ---->   test.wav
void FileSetExt(LPTSTR szFile, int nFileBufLen, LPCTSTR szExt)
{
	int len;
	len = _tcslen(szFile);
	len --;
	while (len >= 0)
	{
		if (szFile[len] == DIR_SLASH || szFile[len] == '.')
			break;
		len --;
	}

	if (len > 0)
	{
		if (szFile[len] == '.')
			szFile[len] = 0;
	}
	_tcscat(szFile, szExt);
}

// 判断文件的扩展名是否匹配
BOOL FileIsExtSame(LPCTSTR szFile, LPCTSTR szExt)
{
	int len1 = _tcslen(szFile);
	int len2 = _tcslen(szExt);

	if (len1 < len2)
		return false;

	return _tcsicmp(szFile + len1 - len2, szExt) == 0;
}

// /:*?"<>|
const TCHAR	_SZ_FILE_NAME_INVALID_CHARS[] = _T("/:*?\"<>|");
BOOL FileNameIsIncInvalidChars(LPCTSTR szFile)
{
	LPCTSTR		szBeg;

	szBeg = szFile;

	while (*szBeg)
	{
		LPCTSTR		szInv;

		szInv = _SZ_FILE_NAME_INVALID_CHARS;
		while (*szInv && *szBeg != *szInv)
			szInv++;
		if (*szInv != '\0')
			return TRUE;

		szBeg++;
	}
	return FALSE;
}

void FileNameFilterInvalidChars(LPCTSTR szFile, LPTSTR szNewFile, int nSizeMax)
{
	LPCTSTR		szBeg, szEnd;
	LPTSTR		szNew;

	szBeg = szFile;
	szEnd = szFile + nSizeMax - 1;
	szNew = szNewFile;

	while (*szBeg && szBeg < szEnd)
	{
		LPCTSTR		szInv;

		szInv = _SZ_FILE_NAME_INVALID_CHARS;
		while (*szInv && *szBeg != *szInv)
			szInv++;
		if (*szInv != '\0')
			*szNew = '_';
		else
			*szNew = *szBeg;

		szBeg++;
		szNew++;
	}
	*szNew = '\0';
}

void FileNameFilterInvalidChars(LPTSTR szFile)
{
	LPTSTR		szBeg;

	szBeg = szFile;

	while (*szBeg)
	{
		LPCTSTR		szInv;

		szInv = _SZ_FILE_NAME_INVALID_CHARS;
		while (*szInv && *szBeg != *szInv)
			szInv++;
		if (*szInv != '\0')
			*szBeg = '_';

		szBeg++;
	}
}

const TCHAR	_SZ_FILE_PATH_INVALID_CHARS[] = _T("/:*?\"<>|");
void FilePathFilterInvalidChars(LPTSTR szFile)
{
	LPTSTR		szBeg;

	szBeg = szFile;

	while (*szBeg)
	{
		LPCTSTR		szInv;

		szInv = _SZ_FILE_PATH_INVALID_CHARS;
		while (*szInv && *szBeg != *szInv)
			szInv++;
		if (*szInv != '\0')
			*szBeg = '_';
		else if (*szBeg == '.')
		{
			LPTSTR	szTemp;
			szTemp = szBeg;
			while (*szTemp == '.')
				szTemp++;
			if (*szTemp == DIR_SLASH)
			{
				while (szBeg != szTemp)
				{
					*szBeg = '_';
					szBeg++;
				}
			}
		}
		szBeg++;
	}
}

CMLFileFind::CMLFileFind()
{
	m_dirp = NULL;
	m_dp = NULL;
	m_bStatValid = false;
}

CMLFileFind::~CMLFileFind()
{
	Close();
}

bool CMLFileFind::OpenDir(LPCTSTR szDir, LPCTSTR szFilter)
{
	Close();

	m_dp = opendir(szDir);
	m_dirp = NULL;
	_tcscpy(m_szDir, szDir);

	return m_dp != NULL;
}

void CMLFileFind::Close()
{
	if (m_dp)
	{
		closedir(m_dp);
		m_dirp = NULL;
		m_dp = NULL;
	}
}

bool CMLFileFind::FindNext()
{
	m_dirp = readdir(m_dp);
	m_bStatValid = false;

	return m_dirp != NULL;
}

bool CMLFileFind::IsCurDir()
{
	if (!GetCurStat())
		return false;

	return S_ISDIR(m_stat.st_mode);
}

bool CMLFileFind::GetCurStat()
{
	if (m_bStatValid)
		return true;

	if (!m_dirp)
		return false;

	TCHAR		szFile[MAX_PATH];
	_tcscpy(szFile, m_szDir);
	DirStringFormat(m_szDir);
	_tcscat(szFile, m_dirp->d_name);

	memset(&m_stat, 0, sizeof(m_stat));
	m_bStatValid = (stat(szFile, &m_stat) == 0);

	return m_bStatValid;
}
