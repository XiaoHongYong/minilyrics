#include "stdafx.h"
#include "safestr.h"
#include "fileapi.h"

IMPLEMENT_CPPUNIT_TEST_REG(FileAPI)

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
//		INVALID_FILE_SIZE	-	无效的大小，文件可能无效
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

// RETURN:
//		INVALID_FILE_SIZE	-	无效的大小，文件可能无效
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

// 设置文件的修改时间和创建时间
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

BOOL CreateDirectoryAll(LPCTSTR szDir)
{
	TCHAR	szTemp[512];
	TCHAR	*szBeg;

	_tcscpy_safe(szTemp, CountOf(szTemp), szDir);

	szBeg = _tcschr(szTemp, DIR_SLASH);
	while (szBeg != NULL)
	{
		*szBeg = '\0';

		if (GetFileAttributes(szTemp) == -1)
		{
			if (!CreateDirectory(szTemp, NULL))
				return FALSE;
		}

		*szBeg = DIR_SLASH;
		szBeg = _tcschr(szBeg + 1, DIR_SLASH);
	}

	if (GetFileAttributes(szDir) == -1)
	{
		if (!CreateDirectory(szDir, NULL))
			return FALSE;
	}

	return TRUE;
}

// COMMENT:
//		判断目录内是否可以写
DWORD CanWriteInDir(LPCTSTR szDir)
{
	DWORD	dwAttr;

	dwAttr = GetFileAttributes(szDir);
	if (dwAttr == 0xFFFFFFFF)
		return FALSE;

	return SetFileAttributes(szDir, dwAttr);
}

BOOL EnumFilesInDir(LPCTSTR szBaseDir, LPCTSTR szFilter, vector<tstring> &vFiles, BOOL bEnumFullPath)
{
	HANDLE				hFileFind;
	WIN32_FIND_DATA		FindData;
	tstring				strSearch;

	strSearch = szBaseDir;
	DirStringFormat(strSearch);

	strSearch += szFilter;

	hFileFind = FindFirstFile(strSearch.c_str(), &FindData);
	if (hFileFind == INVALID_HANDLE_VALUE)
		return FALSE;

	do
	{
		if (!IsFlagSet(FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY))
		{
			if (bEnumFullPath)
			{
				tstring		strFile;

				strFile = szBaseDir;
				DirStringFormat(strFile);
				strFile += FindData.cFileName;

				vFiles.push_back(strFile);
			}
			else
				vFiles.push_back(FindData.cFileName);
		}
	}
	while (FindNextFile(hFileFind, &FindData));

	FindClose(hFileFind);

	return TRUE;
}

// typedef void (*FUNProcessFile)(LPCTSTR szFileName, LPCTSTR szDir, LPVOID lpUserData);
BOOL ProcessFilesInfolder(LPCTSTR szBaseDir, FUNProcessFile funProc, LPVOID lpUserData)
{
	HANDLE				hFileFind;
	WIN32_FIND_DATA		FindData;
	TCHAR				szSearch[MAX_PATH];

	_tcscpy(szSearch, szBaseDir);
	DirStringFormat(szSearch);

	_tcscat(szSearch, _T("*.*"));

	hFileFind = FindFirstFile(szSearch, &FindData);
	if (hFileFind == INVALID_HANDLE_VALUE)
		return FALSE;

	do
	{
		if (!IsFlagSet(FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY))
		{
			funProc(FindData.cFileName, szBaseDir, lpUserData);
		}
		else if (IsFlagSet(FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY)
			&& _tcscmp(FindData.cFileName, _T(".")) != 0
			&& _tcscmp(FindData.cFileName, _T("..")) != 0)
		{
			tstring		strSubDir;

			strSubDir = szBaseDir;
			DirStringFormat(strSubDir);
			strSubDir += FindData.cFileName;
			DirStringFormat(strSubDir);
			ProcessFilesInfolder(strSubDir.c_str(), funProc, lpUserData);
		}
	}
	while (FindNextFile(hFileFind, &FindData));

	FindClose(hFileFind);

	return TRUE;
}

// COMMENT:
//		将目录字符串格式化为末尾带DIR_SLASH的
void DirStringFormat(WCHAR *szDir)
{
	int		nLen;

	nLen = wcslen(szDir);
	if (nLen == 0 || szDir[nLen - 1] != DIR_SLASH)
	{
		szDir[nLen] = DIR_SLASH;
		szDir[nLen + 1] = '\0';
	}
}

// COMMENT:
//		将目录字符串格式化为末尾带DIR_SLASH的
void DirStringFormat(char *szDir)
{
	int		nLen;

	nLen = strlen(szDir);
	if (nLen == 0 || szDir[nLen - 1] != DIR_SLASH)
	{
		szDir[nLen] = DIR_SLASH;
		szDir[nLen + 1] = '\0';
	}
}

// COMMENT:
//		将目录字符串格式化为末尾带DIR_SLASH的
void DirStringFormat(tstring &strDir)
{
	if (strDir.empty() || strDir[strDir.size() - 1] != DIR_SLASH)
	{
		strDir += DIR_SLASH;
	}
}

// Dir: /temp/a/b/c		
// Basedir: /temp/a
// Return related dir: b/c
void GetRelatedPath(LPCTSTR szDir, LPCTSTR szBaseDir, LPTSTR szRelatedPath, int nRelatedPathLen)
{
	LPCTSTR		pDir = szDir, pBaseDir = szBaseDir;
	LPCTSTR		pLatestSame = pDir;

#ifdef _WIN32
	// win32, dir name is case insensitive
	int		f,l;
	while (*pDir)
	{
		if ( ((f = (*(pDir))) >= 'A') && (f <= 'Z') )
			f -= ('A' - 'a');
		if ( ((l = (*(pBaseDir))) >= 'A') && (l <= 'Z') )
			l -= ('A' - 'a');
		if (f != l)
			break;
		if (f == DIR_SLASH)
			pLatestSame = pDir;
		pDir++, pBaseDir++;
	}
#else
	while (*pDir == *pBaseDir && *pDir != '\0')
	{
		pDir++;
		pBaseDir++;
		if (*pDir == DIR_SLASH)
			pLatestSame = pDir;
	}
#endif

	if (*pBaseDir == '\0')
	{
		if (*pDir == DIR_SLASH)
			pDir++;
		_tcscpy_safe(szRelatedPath, nRelatedPathLen, pDir);
	}
	else
	{
		ASSERT(0 && _T("Not implemented yet."));
		_tcscpy_safe(szRelatedPath, nRelatedPathLen, szDir);
// 		if (*pDir == '\0')
// 		{
// 			_tcsinc
// 		}
	}
}

LPCTSTR UrlGetName(LPCTSTR szFile)
{
	LPCTSTR		szBeg, szBeg2;

	szBeg = _tcsrchr(szFile, '/');
	szBeg2 = _tcsrchr(szFile, '\\');
	if (szBeg2 > szBeg)
		szBeg = szBeg2;
	if (szBeg)
		return szBeg + 1;
	else
		return szFile;
}

tstring UrlGetTitle(LPCTSTR szFile)
{
	LPCTSTR		szBeg, szBeg2, szEnd;
	int			nLen;
	tstring		strFileTitle;

	szEnd = _tcsrchr(szFile, '.');

	szBeg = _tcsrchr(szFile, '/');
	szBeg2 = _tcsrchr(szFile, '\\');
	if (szBeg2 > szBeg)
		szBeg = szBeg2;

	if (szBeg == NULL)
		szBeg = szFile;
	else
		szBeg++;

	if (szEnd == NULL || szBeg > szEnd)
		nLen = szFile - szBeg + _tcslen(szFile);
	else
		nLen = szEnd - szBeg;

	strFileTitle.append(szBeg, nLen);

	return strFileTitle;
}

LPCTSTR FileGetName(LPCTSTR szFile)
{
	LPCTSTR		szBeg;

	szBeg = _tcsrchr(szFile, DIR_SLASH);
	if (szBeg)
		return szBeg + 1;
	else
		return szFile;
}

// Retrieve the file title.
// File name format: path/title/.ext
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


void FileGetTitle(LPCTSTR szFile, tstring &strFileTitle)
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

	strFileTitle.resize(0);
	strFileTitle.append(szBeg, nLen);
}


void FileGetPath(LPTSTR szBuff, int nBufLen, LPCTSTR szFile)
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
		_tcsncpy_safe(szBuff, nBufLen, szFile, ilen + 1);
	szBuff[ilen + 1] = '\0';
}

void FileGetPath(LPCTSTR szFile, tstring &strPath)
{
	int ilen = _tcslen(szFile);
	ilen --;
	while (ilen >= 0)
	{
		if (szFile[ilen] == DIR_SLASH)
			break;
		ilen--;
	}

	strPath.resize(0);
	if (ilen >= 0)
		strPath.append(szFile, ilen + 1);
}

LPCTSTR FileGetExt(LPCTSTR szFile)
{
	LPCTSTR		szBeg;

	szBeg = _tcsrchr(szFile, '.');
	if (szBeg == NULL)
		return _T("");
	else
	{
		LPCTSTR		szTemp;

		szTemp = _tcsrchr(szFile, DIR_SLASH);
		if (szTemp == NULL || szTemp < szBeg)
			return szBeg;
		else
			return _T("");
	}
}

// Set the file extention.
// File name format: path/title/.ext
void FileSetExt(LPTSTR szFile, int nFileBufLen, LPCTSTR szExt)
{
	int		len;
	len = _tcslen(szFile);

	len --;
	while (len >= 0)
	{
		if (szFile[len] == DIR_SLASH || szFile[len] == '.')
			break;
		len --;
	}

	if (len >= 0)
	{
		if (szFile[len] == '.')
			szFile[len] = 0;
	}

	_tcscat_safe(szFile, nFileBufLen, szExt);
}

void FileSetExt(tstring &strFile, LPCTSTR szExt)
{
	LPCTSTR		szFile = strFile.c_str();
	int			len = strFile.size();

	len --;
	while (len >= 0)
	{
		if (szFile[len] == DIR_SLASH || szFile[len] == '.')
			break;
		len --;
	}

	if (len >= 0)
	{
		if (szFile[len] == '.')
			strFile.resize(len);
	}

	strFile += szExt;
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

	// 忽略c:\\之类的字符
	if (IsCharAlpha(*szBeg))
	{
		szBeg++;
		if (*szBeg == ':')
		{
			if (*(szBeg + 1) == DIR_SLASH)
				szBeg += 2;
		}
	}

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

	// 忽略c:\\之类的字符
	if (IsCharAlpha(*szBeg))
	{
		*szNew = *szBeg;
		szBeg++;
		szNew++;
		if (*szBeg == ':')
		{
			if (*(szBeg + 1) == DIR_SLASH)
			{
				*szNew = *szBeg;
				szBeg++;
				szNew++;

				*szNew = *szBeg;
				szBeg++;
				szNew++;
			}
		}
	}

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

	// 忽略c:\\之类的字符
	if (IsCharAlpha(*szBeg))
	{
		szBeg++;
		if (*szBeg == ':')
		{
			if (*(szBeg + 1) == DIR_SLASH)
				szBeg += 2;
		}
	}

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

const TCHAR	_SZ_FILE_PATH_INVALID_CHARS[] = _T(":*?\"<>|")_T(SZ_DIR_SLASH);
void FilePathFilterInvalidChars(LPTSTR szFile)
{
	LPTSTR		szBeg;

	szBeg = szFile;

	// 忽略c:\\之类的字符
	if (IsCharAlpha(*szBeg))
	{
		szBeg++;
		if (*szBeg == ':')
		{
			if (*(szBeg + 1) == DIR_SLASH)
				szBeg += 2;
		}
	}

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

//////////////////////////////////////////////////////////////////////////
//

CMLFileFind::CMLFileFind()
{
	hFileFind = INVALID_HANDLE_VALUE;
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
	DirStringFormat(str);
	str += szFilter;

	hFileFind = FindFirstFile(str.c_str(), &FindData);

	bFirst = true;

	return (hFileFind != INVALID_HANDLE_VALUE);
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

	return FindNextFile(hFileFind, &FindData) != 0;
}

LPCTSTR CMLFileFind::GetCurName()
{
	return FindData.cFileName;
}

bool CMLFileFind::IsCurDir()
{
	return IsFlagSet(FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY);
}

#ifdef _CPPUNIT_TEST

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

class CTestCaseFileAPI : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(CTestCaseFileAPI);
	CPPUNIT_TEST(TestFileNameStr);
	CPPUNIT_TEST(TestGetRelatedDir);
	CPPUNIT_TEST(TestFileSetExt);
	CPPUNIT_TEST_SUITE_END();

protected:

public:
	void setUp()
	{
	}
	void tearDown() 
	{
	}

protected:
	void TestFileNameStr()
	{
#ifdef _WIN32
		LPCTSTR		SZ_DIR_CMP[] =  { _T("c:\\folder"), _T("c:\\folder\\"), _T("") };
		LPCTSTR		SZ_DIR_CMP_OK[] = { _T("c:\\folder\\"), _T("c:\\folder\\"), _T("\\") };
#else
		LPCTSTR		SZ_DIR_CMP[] = { _T("/tmp/folder"), _T("/tmp/folder/"), _T("") };
		LPCTSTR		SZ_DIR_CMP_OK[] = { _T("/tmp/folder/"), _T("/tmp/folder/"), _T("") };
#endif
		TCHAR		szDir[MAX_PATH];
		tstring		strDir;

		ASSERT(CountOf(SZ_DIR_CMP) == CountOf(SZ_DIR_CMP_OK));
		for (int i = 0; i < CountOf(SZ_DIR_CMP); i++)
		{
			_tcscpy(szDir, SZ_DIR_CMP[i]);
			DirStringFormat(szDir);
			if (!(_tcscmp(szDir, SZ_DIR_CMP_OK[i]) == 0))
				CPPUNIT_FAIL_T(CStrPrintf(_T("DirStringFormat, case: %d, %s"), i, SZ_DIR_CMP_OK[i]).c_str());

			strDir = SZ_DIR_CMP[i];
			DirStringFormat(strDir);
			if (!(_tcscmp(strDir.c_str(), SZ_DIR_CMP_OK[i]) == 0))
				CPPUNIT_FAIL_T(CStrPrintf(_T("DirStringFormat(stl), case: %d, %s"), i, SZ_DIR_CMP_OK[i]).c_str());
		}
	}

	void TestGetRelatedDir()
	{
// #ifdef _WIN32
// 		LPCTSTR		SZ_DIR[] =  { _T("c:\\a\\b\\c"), _T("c:\\a\\b\\c"), _T("c:\\a\\b\\c"), _T("c:\\a\\b\\c"), _T("c:\\a\\b\\c") };
// 		LPCTSTR		SZ_DIR_BASE[] =  { _T("c:\\a"), _T("c:\\a\\"), _T(""), _T("c:\\a\\b\\c\\d"), _T("c:\\a\\b\\e") };
// 		LPCTSTR		SZ_DIR_RELATED[] = { _T("b\\c"), _T("b\\c"), _T("c:\\a\\b\\c"), _T("..\\"), _T("..\\c") };
// #else
// 		LPCTSTR		SZ_DIR[] =  { _T("/a/b/c"), _T("/a/b/c"), _T("/a/b/c"), _T("/a/b/c"), _T("/a/b/c") };
// 		LPCTSTR		SZ_DIR_BASE[] =  { _T("/a"), _T("/a/"), _T(""), _T("/a/b/c/d"), _T("/a/b/e") };
// 		LPCTSTR		SZ_DIR_RELATED[] = { _T("b/c"), _T("b/c"), _T("/a/b/c"), _T("../"), _T("../c") };
// #endif
// 		TCHAR		szDirRelated[MAX_PATH];
// 
// 		ASSERT(CountOf(SZ_DIR) == CountOf(SZ_DIR_BASE));
// 		ASSERT(CountOf(SZ_DIR) == CountOf(SZ_DIR_RELATED));
// 		for (int i = 0; i < CountOf(SZ_DIR_RELATED); i++)
// 		{
// 			GetRelatedPath(SZ_DIR[i], SZ_DIR_BASE[i], szDirRelated, CountOf(szDirRelated));
// 			if (!(_tcscmp(szDirRelated, SZ_DIR_RELATED[i]) == 0))
// 				CPPUNIT_FAIL_T(CStrPrintf(_T("GetRelatedDir, case: %d, %s"), i, szDirRelated).c_str());
// 		}
	}

	void TestFileSetExt()
	{
		LPCTSTR		SZ_EXT = _T(".ext");
#ifdef _WIN32
		LPCTSTR		SZ_FILE[] =  { _T("c:\\file.txt"), _T("c:\\file.txt3"), _T(""), _T("c:\\file.t\\xt3") };
		LPCTSTR		SZ_FILE_OK[] = { _T("c:\\file.ext"), _T("c:\\file.ext"), _T(".ext"), _T("c:\\file.t\\xt3.ext") };
#else
		LPCTSTR		SZ_FILE[] =  { _T("/file.txt"), _T("/file.txt3"), _T(""), _T("/file.t/xt3") };
		LPCTSTR		SZ_FILE_OK[] = { _T("/file.ext"), _T("/file.ext"), _T(".ext"), _T("/file.t/xt3.ext") };
#endif
		LPCTSTR		SZ_EXT_OK[] = { _T(".txt"), _T(".txt3"), _T(""), _T("") };

		TCHAR		szFile[MAX_PATH];
		tstring		strFile;

		ASSERT(CountOf(SZ_FILE) == CountOf(SZ_FILE_OK));
		for (int i = 0; i < CountOf(SZ_FILE); i++)
		{
			_tcscpy(szFile, SZ_FILE[i]);
			FileSetExt(szFile, CountOf(szFile), SZ_EXT);
			if (!(_tcscmp(szFile, SZ_FILE_OK[i]) == 0))
				CPPUNIT_FAIL_T(CStrPrintf(_T("FileSetExt, case: %d, %s"), i, SZ_FILE_OK[i]).c_str());

			strFile = SZ_FILE[i];
			FileSetExt(strFile, SZ_EXT);
			if (!(_tcscmp(strFile.c_str(), SZ_FILE_OK[i]) == 0))
				CPPUNIT_FAIL_T(CStrPrintf(_T("FileSetExt(stl), case: %d, %s"), i, SZ_FILE_OK[i]).c_str());

			if (!(_tcscmp(FileGetExt(SZ_FILE[i]), SZ_EXT_OK[i]) == 0))
				CPPUNIT_FAIL_T(CStrPrintf(_T("FileGetExt, case: %d, %s"), i, SZ_EXT_OK[i]).c_str());
		}
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseFileAPI);


#endif // _CPPUNIT_TEST
