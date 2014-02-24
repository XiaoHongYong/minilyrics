#include "../stdafx.h"
#include "../safestr.h"
#include "fileapi.h"
#include "../stringex_t.h"
#include "../CharEncoding.h"
#include "../stringex.h"
#include "../FileApiBase.h"

bool filetruncate(FILE *fp, int nLen)
{
	if (ftruncate(fileno(fp), nLen) == -1)
		return false;

	return true;
}

bool IsFileExist(LPCTSTR szFileName)
{
	struct stat		filestat;

	memset(&filestat, 0, sizeof(filestat));
	return stat(szFileName, &filestat) == 0;
}

bool IsDirExist(LPCTSTR szDir)
{
	DIR			*dp;

	dp = opendir(szDir);
	if (dp == NULL)
	{
		return false;
	}

	closedir(dp);
	
	return true;
}

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

// RETURN:
//		INVALID_FILE_SIZE	- Invalid file size
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

bool RemoveDirectory(LPCTSTR lpPathName)
{
	return rmdir(lpPathName) == 0;
}

bool CreateDirectory(LPCSTR lpPathName)
{
	int n = mkdir(lpPathName, S_IRWXU | S_IRWXG | S_IROTH);
	
	return n == 0;
}

#ifdef _LINUX
#include <copyfile.h>

bool CopyFile(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, BOOL bFailIfExists)
{
	if (!bFailIfExists)
	{
		if (IsFileExist(lpNewFileName))
			DeleteFile(lpNewFileName);
	}
	int n = copyfile(lpExistingFileName, lpNewFileName, 0, COPYFILE_STAT | COPYFILE_DATA);
	return n == 0;
}

#else

bool CopyFile(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, BOOL bFailIfExists)
{
	if (!bFailIfExists)
	{
		if (IsFileExist(lpNewFileName))
			DeleteFile(lpNewFileName);
	}
	// TODO: TBD, int n = copyfile(lpExistingFileName, lpNewFileName, 0, COPYFILE_STAT | COPYFILE_DATA);
	return false;
}

#endif

DWORD CanWriteInDir(LPCTSTR szDir)
{
	return true;
}

//////////////////////////////////////////////////////////////////////////

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

	while (m_dirp && m_dirp->d_name[0] == '.')
	{
		if (!(m_dirp->d_name[1] == '\0'
			|| (m_dirp->d_name[1] == '.' && m_dirp->d_name[2] == '\0')))
			break;
		m_dirp = readdir(m_dp);
	}

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

	TCHAR		szFile[400];
	_tcscpy(szFile, m_szDir);
	DirStringAddSlash(szFile);
	_tcscat(szFile, m_dirp->d_name);

	memset(&m_stat, 0, sizeof(m_stat));
	m_bStatValid = (stat(szFile, &m_stat) == 0);

	return m_bStatValid;
}
