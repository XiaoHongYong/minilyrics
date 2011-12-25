// Helper.h: interface for the Helper class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HELPER_H__12EAEFEA_BAAC_49DE_8ABD_28FF06B85835__INCLUDED_)
#define AFX_HELPER_H__12EAEFEA_BAAC_49DE_8ABD_28FF06B85835__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _WIN32_WCE
#define CeSHInitFullScrDlg(pDlg)
#endif

int CanOpenFileForWrite(LPCTSTR szFile, bool bMustExist = true);

void Execute(CBaseWnd *pWnd, LPCTSTR szExe, LPCTSTR szParam);

bool SetClipboardText(CBaseWnd *pWnd, LPCTSTR szText);

LPCTSTR HttpErrorCodeToStr(int nCode);

BOOL SHDeleteFile(LPCTSTR szFile, CBaseWnd *pWndParent);

BOOL SHCopyFile(LPCTSTR szSrcFile, LPCTSTR szTargFile, CBaseWnd *pWndParent);

BOOL SetFileNoReadOnly(LPCTSTR szFile);

void DeleteAllFiles(LPCTSTR szDir, BOOL bIncSubDir, BOOL bDelReadOnlyFile);

bool LoadProxySvrFromIE(bool &bUseProxy, tstring &strSvr, int &nPort);

bool LoadProxySvrFromFireFox(bool &bUseProxy, tstring &strSvr, int &nPort);

void ParseMLTextMacro(LPCTSTR szText, tstring &str);

void GetNotepadEditor(tstring &strEditor);

uint32 GetSecCount();

void ProfileGetColorValue(COLORREF &clr, LPCTSTR szSectName, LPCTSTR szKeyName);

void ProfileGetColorValue(MLColor &clr, LPCTSTR szSectName, LPCTSTR szKeyName);

#endif // !defined(AFX_HELPER_H__12EAEFEA_BAAC_49DE_8ABD_28FF06B85835__INCLUDED_)
