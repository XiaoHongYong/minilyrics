// Helper.cpp: implementation of the Helper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "Helper.h"

void AnalysisProxySetting(LPCTSTR szProxySetting, TCHAR szServer[], int nMaxSize, int &nPort);

int CanOpenFileForWrite(LPCTSTR szFile, bool bMustExist)
{
	if (bMustExist && !IsFileExist(szFile))
		return ERR_FILE_NOT_EXIST;

/*	FILE		*fp;

	fp = _tfopen(szFile, _T("r+b"));
	if (!fp)
		return false;

	fclose(fp);*/

	return ERR_OK;
}

void Execute(CBaseWnd *pWnd, LPCTSTR szExe, LPCTSTR szParam)
{
	// TODO: TBD
}

bool SetClipboardText(CBaseWnd *pWnd, LPCTSTR szText)
{
	return Tobool(CopyTextToClipboard(szText));
}

BOOL SHDeleteFile(LPCTSTR szFile, CBaseWnd *pWndParent)
{
	return DeleteFile(szFile);
}

BOOL SHCopyFile(LPCTSTR szSrcFile, LPCTSTR szTargFile, CBaseWnd *pWndParent)
{
	return CopyFile(szSrcFile, szTargFile, true);
}

#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES	0xFFFFFFFF
#endif

BOOL SetFileNoReadOnly(LPCTSTR szFile)
{
	return TRUE;
}

bool LoadProxySvrFromIE(bool &bUseProxy, tstring &strSvr, int &nPort)
{
	return false;
}

static bool FindStringValue(LPCTSTR szBuff, LPCTSTR szName, LPCTSTR szEndTag, tstring &strValue)
{
	LPCTSTR		szBeg, szEnd;

	strValue.resize(0);

	szBeg = _tcsstr(szBuff, szName);
	if (!szBeg)
		return false;

	szBeg += _tcslen(szName);

	szEnd = _tcsstr(szBeg, szEndTag);
	if (!szEnd)
		return false;

	strValue.append(szBeg, szEnd);

	return true;
}

bool LoadProxySvrFromFireFox(bool &bUseProxy, tstring &strSvr, int &nPort)
{
	return false;
}


void GetNotepadEditor(tstring &strEditor)
{
}

uint32 GetSecCount()
{
	return GetTickCount() / 1000;
}
