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

void OpenUrl(CBaseWnd *pWnd, LPCTSTR szUrl);

void Execute(CBaseWnd *pWnd, LPCTSTR szExe, LPCTSTR szParam);

void MLHtmlHelper(CBaseWnd *pWnd, LPCTSTR szFile);

bool SetClipboardText(CBaseWnd *pWnd, LPCTSTR szText);

void GetFileNameFromURL(const TCHAR *szURL, TCHAR *szFileName, int nLenMax);

void LogToEdit(CWidgetEditBox *editCtrl, LPCTSTR szText, BOOL bNewLine);

LPCTSTR HttpErrorCodeToStr(int nCode);

int MessageOut(LPCTSTR lpText, LPCTSTR szSaveDefault,CBaseWnd *pWnd = NULL, UINT uType = MB_ICONINFORMATION, LPCTSTR lpCaption = NULL);

BOOL SHDeleteFile(LPCTSTR szFile, CBaseWnd *pWndParent);

BOOL SHCopyFile(LPCTSTR szSrcFile, LPCTSTR szTargFile, CBaseWnd *pWndParent);

BOOL SetFileNoReadOnly(LPCTSTR szFile);

void DeleteAllFiles(LPCTSTR szDir, BOOL bIncSubDir, BOOL bDelReadOnlyFile);

bool LoadProxySvrFromIE(bool &bUseProxy, tstring &strSvr, int &nPort);

void ParseMLTextMacro(LPCTSTR szText, tstring &str);

void GetNotepadEditor(tstring &strEditor);

void FormatPlayTime(TCHAR szBuf[], int nmsTime);

void FillRectBorder(IMLGraphicsBase *pGraph, CRect &rc, int nBorderSize, CMLBrush &br);

void GetAppNameLong(TCHAR szName[], int nLen);

uint32 GetSecCount();

PRAW_IMAGE_DATA LoadRawImageDataFromHBitmap(HBITMAP hBmp);

HBITMAP RawImageToHBitmap(CRawImage &image);

//
// Field Read and Write
//

template<class int_t>
void FieldReadInt(LPCSTR &szStr, int_t &nValue)
{
	while (*szStr == ' ')
		szStr++;
	ReadInt(szStr, nValue);
	if (*szStr == ',')
		szStr++;
}

bool FieldReadColor(LPCSTR &szStr, COLORREF &clr);

bool FieldReadStr(LPCSTR &szStr, tstring &str);

void FieldWriteInt(string &strField, int n);

void FieldWriteColor(string &strField, COLORREF clr);

void FieldWriteStr(string &strField, LPCTSTR str);


void ProfileGetColorValue(COLORREF &clr, LPCTSTR szSectName, LPCTSTR szKeyName);

void ProfileGetColorValue(MLColor &clr, LPCTSTR szSectName, LPCTSTR szKeyName);

#endif // !defined(AFX_HELPER_H__12EAEFEA_BAAC_49DE_8ABD_28FF06B85835__INCLUDED_)
