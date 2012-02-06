#pragma once

DECLAR_CPPUNIT_TEST_REG(FileAPI)

#ifndef INVALID_FILE_SIZE
#define INVALID_FILE_SIZE ((DWORD)0xFFFFFFFF) 
#endif

bool filetruncate(FILE *fp, int nLen);

bool IsFileExist(LPCTSTR szFileName);
bool IsDirExist(LPCTSTR szDir);

bool GetFileLength(LPCTSTR szFileName, uint64 &nLength);

DWORD GetFileLength(LPCSTR szFileName);
DWORD GetFileLength(LPCWSTR szFileName);

bool CreateDirectory(LPCTSTR lpPathName);
bool CreateDirectoryAll(LPCTSTR szDir);

DWORD CanWriteInDir(LPCTSTR szDir);

bool EnumFilesInDir(LPCTSTR szBaseDir, LPCTSTR szFilter, vector<tstring> &vFiles, bool bEnumFullPath);

typedef void (*FUNProcessFile)(LPCTSTR szFileName, LPCTSTR szDir, LPVOID lpUserData);
bool ProcessFilesInfolder(LPCTSTR szBaseDir, FUNProcessFile funProc, LPVOID lpUserData);

void DirStringAddSlash(WCHAR *szDir);
void DirStringAddSlash(char *szDir);
void DirStringAddSlash(tstring &strDir);

tstring DirStringJoint(LPCTSTR szDir, LPCTSTR szSubFileDir);
tstring DirStringJoint(LPCTSTR szDir, LPCTSTR szSubFileDir1, LPCTSTR szSubFileDir2);

#define DirStringFormat		DirStringAddSlash

void GetRelatedPath(LPCTSTR szDir, LPCTSTR szBaseDir, LPTSTR szRelatedPath, int nRelatedPathLen);

LPCTSTR UrlGetName(LPCTSTR szFile);
LPCTSTR UrlGetExt(LPCTSTR szFile);
tstring UrlGetTitle(LPCTSTR szFile);

LPCTSTR FileGetName(LPCTSTR szFile);

void FileGetTitle(LPTSTR szBuff, int nBuffLen, LPCTSTR szFile);
void FileGetTitle(LPCTSTR szFile, tstring &strFileTitle);

void FileGetPath(LPCTSTR szFile, tstring &strPath);
void FileGetPath(LPTSTR szBuff, int nBufLen, LPCTSTR szFile);

void FileSetExt(tstring &strFile, LPCTSTR szExt);
void FileSetExt(LPTSTR szFile, int nFileBufLen, LPCTSTR szExt);
LPCTSTR FileGetExt(LPCTSTR szFile);

bool FileIsExtSame(LPCTSTR szFile, LPCTSTR szExt);

bool FileNameIsIncInvalidChars(LPCTSTR szFile);
tstring FileNameFilterInvalidChars(LPCTSTR szFile);
void FilePathFilterInvalidChars(LPTSTR szFile);

#ifndef WIN32
bool DeleteFile(LPCTSTR lpFileName);
#endif

void TStringToProperFileData(LPCTSTR str, CCharBuffer &bufData);

bool IsCurOrParentDir(LPCTSTR szDir);

#ifdef _WIN32
#include "win32/fileapi.h"
#endif

#ifdef _MAC
#include "Mac/fileapi.h"
#endif

#ifdef _LINUX
#include "linux/fileapi.h"
#endif
