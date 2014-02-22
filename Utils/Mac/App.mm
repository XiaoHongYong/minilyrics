/********************************************************************
	Created  :	2003年11月2日 23:24:29
	FileName :	BaseFrmWork.cpp
	Author   :	xhy
	
	Purpose  :	应用程序的基本框架结构定义
*********************************************************************/

#import <Foundation/Foundation.h>
#include "../stdafx.h"
#include "Utils.h"
#include "App.h"

// 配置文件
class CProfile	g_Profile;

// 日志
class CLog		g_Log;

TCHAR			g_szWorkingFolder[MAX_PATH] = _T("");
TCHAR			g_szAppDataDir[MAX_PATH] = _T("");

void SetWorkingFolder(LPCTSTR szWorkingFolder)
{
	// assert(szWorkingFolder && !IsEmptyString(szWorkingFolder));
	_tcscpy_safe(g_szWorkingFolder, MAX_PATH, szWorkingFolder);
	DirStringFormat(g_szWorkingFolder);
}

void GetAppResourceDir(LPTSTR szWorkingFolder)
{
	if (IsEmptyString(g_szWorkingFolder))
	{
		GetAppResourceDir();
	}

	_tcscpy(szWorkingFolder, g_szWorkingFolder);
}

LPCTSTR GetAppResourceDir()
{
	if (!IsEmptyString(g_szWorkingFolder))
		return g_szWorkingFolder;

#ifdef _IPHONE
	NSArray *paths = NSSearchPathForDirectoriesInDomains(
														 NSDocumentDirectory,
														 NSUserDomainMask,
														 YES);
	NSString *documentsDir = [paths objectAtIndex:0];
	_tcscpy_safe(g_szWorkingFolder, CountOf(g_szWorkingFolder), (const char *)[documentsDir UTF8String]);
#else
	NSString *resourcePath = [[NSBundle mainBundle] resourcePath];
	_tcscpy_safe(g_szWorkingFolder, CountOf(g_szWorkingFolder), (const char *)[resourcePath UTF8String]);
#endif
	
	DirStringAddSlash(g_szWorkingFolder);
	return g_szWorkingFolder;
}

LPCTSTR GetAppDataDir()
{
	if (IsEmptyString(g_szAppDataDir))
	{
		GetAppResourceDir(g_szAppDataDir);
	}
	
	return g_szAppDataDir;
}

void GetAppDataDir(TCHAR *szAppDir)
{
	_tcscpy_safe(szAppDir, MAX_PATH, GetAppDataDir());
}

void InitSetDefaultCharEncoding();

BOOL InitBaseFrameWork(int argc, char *argv[], LPCTSTR szLogFile, LPCTSTR szProfileName, LPCTSTR szDefAppName)
{
	assert(sizeof(byte) == 1);
	assert(sizeof(WORD) == 2);
	assert(sizeof(DWORD) == 4);
	assert(sizeof(int16) == 2);
	assert(sizeof(int32) == 4);
	assert(sizeof(int64) == 8);
	assert(sizeof(WCHAR) == 2);
	assert(sizeof(WCHAR) == 2);

#ifndef _IPHONE

		NSFileManager* fileManager = [NSFileManager defaultManager];

/*		NSError *error = nil;
		NSURL *supportedDir = [fileManager URLForDirectory:NSApplicationSupportDirectory | NSItemReplacementDirectory inDomain:NSUserDomainMask appropriateForURL:[NSURL URLWithString:@"com.crintsoft.zikiplayer"] create:YES error: &error];
		
		_tcscpy_safe(g_szWorkingFolder, CountOf(g_szWorkingFolder), [[[supportedDir filePathURL] absoluteString] UTF8String]);*/

		NSURL* appSupportDir = nil;
		NSArray *urls = [fileManager URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask];
		if ([urls count] > 0) {
			tstring componentName = tstring("com.crintsoft.");
			if (!IsEmptyString(szDefAppName))
				componentName += szDefAppName;
			appSupportDir = [[urls objectAtIndex:0] URLByAppendingPathComponent:[NSString stringWithUTF8String:componentName.c_str()]];
			_tcscpy_safe(g_szAppDataDir, CountOf(g_szAppDataDir), [[appSupportDir path] UTF8String]);
			if ([fileManager fileExistsAtPath:[appSupportDir path]] != YES) {
				[fileManager createDirectoryAtURL:appSupportDir
					  withIntermediateDirectories:YES
									   attributes:nil
											error:nil];
			}
			DirStringAddSlash(g_szAppDataDir);
		}
#endif
	
	InitSetDefaultCharEncoding();

	g_Profile.Init(szProfileName, szDefAppName);

	g_Log.Init(szLogFile);

	return TRUE;
}
