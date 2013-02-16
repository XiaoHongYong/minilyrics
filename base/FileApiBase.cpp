#include "stdafx.h"
#include "base.h"
#include "safestr.h"
#include "base.h"
#include "FileApiBase.h"

IMPLEMENT_CPPUNIT_TEST_REG(FileAPI)

bool CreateDirectoryAll(LPCTSTR szDir)
{
	TCHAR	szTemp[512];
	TCHAR	*szBeg;

	_tcscpy_safe(szTemp, CountOf(szTemp), szDir);

	szBeg = _tcschr(szTemp, DIR_SLASH);
	while (szBeg != NULL)
	{
		*szBeg = '\0';

		if (!IsEmptyString(szTemp) && !IsDirExist(szTemp))
		{
			if (!CreateDirectory(szTemp))
				return FALSE;
		}

		*szBeg = DIR_SLASH;
		szBeg = _tcschr(szBeg + 1, DIR_SLASH);
	}

	if (!IsDirExist(szTemp))
	{
		if (!CreateDirectory(szDir))
			return FALSE;
	}

	return TRUE;
}

bool EnumFilesInDir(LPCTSTR szBaseDir, LPCTSTR szFilter, vector<tstring> &vFiles, bool bEnumFullPath)
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
				tstring		strFile;

				strFile = szBaseDir;
				DirStringAddSlash(strFile);
				strFile += find.GetCurName();

				vFiles.push_back(strFile);
			}
			else
				vFiles.push_back(find.GetCurName());
		}
	}

	return TRUE;
}

// typedef void (*FUNProcessFile)(LPCTSTR szFileName, LPCTSTR szDir, LPVOID lpUserData);
bool ProcessFilesInfolder(LPCTSTR szBaseDir, FUNProcessFile funProc, LPVOID lpUserData)
{
	CMLFileFind			find;

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
//		将目录字符串格式化为末尾带DIR_SLASH的
void DirStringAddSlash(WCHAR *szDir)
{
	size_t nLen;

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
//		将目录字符串格式化为末尾带DIR_SLASH的
void DirStringAddSlash(char *szDir)
{
	size_t		nLen;

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
//		将目录字符串格式化为末尾带DIR_SLASH的
void DirStringAddSlash(tstring &strDir)
{
	if (!strDir.empty())
	{
		if (strDir[strDir.size() - 1] != DIR_SLASH)
			strDir += DIR_SLASH;
	}
}

tstring DirStringJoint(LPCTSTR szDir, LPCTSTR szSubFileDir)
{
	tstring		strFile = szDir;

	DirStringAddSlash(strFile);
	strFile += szSubFileDir;

	return strFile;
}

tstring DirStringJoint(LPCTSTR szDir, LPCTSTR szSubFileDir1, LPCTSTR szSubFileDir2)
{
	tstring		strFile = szDir;

	DirStringAddSlash(strFile);
	strFile += szSubFileDir1;

	DirStringAddSlash(strFile);
	strFile += szSubFileDir2;

	return strFile;
}

// Dir: /temp/a/b/c		
// Basedir: /temp/a
// Return related dir: b/c
void GetRelatedPath(LPCTSTR szDir, LPCTSTR szBaseDir, LPTSTR szRelatedPath, int nRelatedPathLen)
{
	LPCTSTR		pDir = szDir, pBaseDir = szBaseDir;

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
		pDir++, pBaseDir++;
	}
#else
	while (*pDir == *pBaseDir && *pDir != '\0')
	{
		pDir++;
		pBaseDir++;
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

LPCTSTR UrlGetExt(LPCTSTR szFile)
{
	LPCTSTR		szBeg;
	
	szBeg = _tcsrchr(szFile, '.');
	if (szBeg == NULL)
		return _T("");
	else
	{
		LPCTSTR szTemp = _tcsrchr(szFile, '/');
		LPCTSTR szTemp2 = _tcsrchr(szFile, '\\');
		if (szTemp2 > szTemp)
			szTemp = szTemp2;
		if (szTemp == NULL || szTemp < szBeg)
			return szBeg;
		else
			return _T("");
	}
}

tstring UrlGetTitle(LPCTSTR szFile)
{
	LPCTSTR		szBeg, szBeg2, szEnd;
	size_t		nLen;
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

tstring FileGetPath(LPCTSTR szFile)
{
	tstring strPath;
	long ilen = _tcslen(szFile);
	ilen -= 2;
	while (ilen >= 0)
	{
		if (szFile[ilen] == DIR_SLASH)
			break;
		ilen--;
	}

	strPath.resize(0);
	if (ilen >= 0)
		strPath.append(szFile, ilen + 1);

	return strPath;
}

// Set the file extention.
// File name format: path/title/.ext
void FileSetExt(LPTSTR szFile, int nFileBufLen, LPCTSTR szExt)
{
	long len = _tcslen(szFile);

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
	long		len = strFile.size();

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

bool FileIsExtSame(LPCTSTR szFile, LPCTSTR szExt)
{
	return StrIEndWith(szFile, szExt);
}

// /:*?"<>|
#ifdef _WIN32
const TCHAR	_SZ_FILE_NAME_INVALID_CHARS[] = _T("/:*?\"<>|");
#else
const TCHAR	_SZ_FILE_NAME_INVALID_CHARS[] = _T("\\:*?\"<>|");
#endif
bool FileNameIsIncInvalidChars(LPCTSTR szFile)
{
	LPCTSTR		szBeg;

	szBeg = szFile;

#ifdef _WIN32_DESKTOP
	// Ignore string like "c:\\"
	if (IsCharAlpha(*szBeg))
	{
		szBeg++;
		if (*szBeg == ':')
		{
			if (*(szBeg + 1) == DIR_SLASH)
				szBeg += 2;
		}
	}
#endif // _WIN32_DESKTOP

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

tstring FileNameFilterInvalidChars(LPCTSTR szFile)
{
	LPCTSTR		szBeg = szFile;
	tstring		strNewName;

#ifdef _WIN32_DESKTOP
	if (IsCharAlpha(*szBeg))
	{
		strNewName += *szBeg;
		szBeg++;
		if (*szBeg == ':' && *(szBeg + 1) == DIR_SLASH)
		{
			strNewName.append(szBeg, 2);
			szBeg += 2;
		}
	}
#endif // _WIN32_DESKTOP

	while (*szBeg)
	{
		LPCTSTR		szInv;

		szInv = _SZ_FILE_NAME_INVALID_CHARS;
		while (*szInv && *szBeg != *szInv)
			szInv++;
		if (*szInv != '\0')
			strNewName += '_';
		else
			strNewName += *szBeg;

		szBeg++;
	}

	return strNewName;
}

const TCHAR	_SZ_FILE_PATH_INVALID_CHARS[] = _T(":*?\"<>|")_T(SZ_DIR_SLASH);
void FilePathFilterInvalidChars(LPTSTR szFile)
{
	LPTSTR		szBeg;

	szBeg = szFile;

#ifdef _WIN32_DESKTOP
	// Ignore string like "c:\\"
	if (IsCharAlpha(*szBeg))
	{
		szBeg++;
		if (*szBeg == ':')
		{
			if (*(szBeg + 1) == DIR_SLASH)
				szBeg += 2;
		}
	}
#endif // _WIN32_DESKTOP

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

bool CopyDir(LPCTSTR lpExistingDir, LPCTSTR lpNewDir)
{
	CMLFileFind finder;
	if (!finder.OpenDir(lpExistingDir))
		return false;
	
	if (!IsFileExist(lpNewDir))
		CreateDirectory(lpNewDir);

	while (finder.FindNext())
	{
		tstring src = DirStringJoint(lpExistingDir, finder.GetCurName());
		tstring dst = DirStringJoint(lpNewDir, finder.GetCurName());
		
		bool ret;
		if (finder.IsCurDir())
			ret = CopyDir(src.c_str(), dst.c_str());
		else
			ret = Tobool(CopyFile(src.c_str(), dst.c_str(), false));
		if (!ret)
			return false;
	}
	
	return true;
}

//////////////////////////////////////////////////////////////////////////
//

bool IsCurOrParentDir(LPCTSTR szDir)
{
	if (_T('.') != szDir[0])
		return false;
	if ( (_T('\0') == szDir[1]) || ((_T('.') == szDir[1]) && (_T('\0') == szDir[2])))
		return true;
	return false;
}

// Convert UTF-8 lyrics content to most proper file content, add FILE BOM
void TStringToProperFileData(LPCTSTR str, CCharBuffer &bufData)
{
	if (IsAnsiStr(str))
		TStringToAnsi(str, -1, bufData);
	else
	{
		TStringToUTF8(str, -1, bufData);
		if (bufData.size() < _tcslen(str) * sizeof(TCHAR))
			bufData.insert(0, SZ_FE_UTF8, strlen(SZ_FE_UTF8));
		else
		{
			bufData.clear();
			bufData.append(SZ_FE_UCS2);
			bufData.append((LPCSTR)str, _tcslen(str) * sizeof(TCHAR));
		}
	}
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
	CPPUNIT_TEST(TestFileNameFilterInvalidChars);
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
		LPCTSTR		SZ_DIR_CMP_OK[] = { _T("c:\\folder\\"), _T("c:\\folder\\"), _T("") };
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
			DirStringAddSlash(szDir);
			if (!(_tcscmp(szDir, SZ_DIR_CMP_OK[i]) == 0))
				CPPUNIT_FAIL_T(CStrPrintf(_T("DirStringAddSlash, case: %d, %s"), i, SZ_DIR_CMP_OK[i]).c_str());

			strDir = SZ_DIR_CMP[i];
			DirStringAddSlash(strDir);
			if (!(_tcscmp(strDir.c_str(), SZ_DIR_CMP_OK[i]) == 0))
				CPPUNIT_FAIL_T(CStrPrintf(_T("DirStringAddSlash(stl), case: %d, %s"), i, SZ_DIR_CMP_OK[i]).c_str());
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

	void TestFileNameFilterInvalidChars()
	{
#ifdef _WIN32
		// /:*?\"<>|
		LPCTSTR		szFile[] = {
			_T("c:\\aa?:!/<>|*.txt"), _T("c:\\aa__!_____.txt"),
			_T("c:\\bb\\cc\aa.txt"), _T("c:\\bb\\cc\aa.txt"),
			_T("a?:a.txt"), _T("a__a.txt"),
		};
#else
		LPCTSTR		szFile[] = {
			_T("/aa?:!\\<>|*.txt"), _T("/aa__!_____.txt"),
			_T("/bb/cc/aa.txt"), _T("/bb/cc/aa.txt"),
			_T("a?:a.txt"), _T("a__a.txt"),
		};
#endif

		for (int i = 0; i < CountOf(szFile); i += 2)
		{
			tstring strOut = FileNameFilterInvalidChars(szFile[i]);
			CPPUNIT_ASSERT(_tcscmp(strOut.c_str(), szFile[i + 1]) == 0);
		}
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseFileAPI);


#endif // _CPPUNIT_TEST
