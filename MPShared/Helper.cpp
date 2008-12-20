// Helper.cpp: implementation of the Helper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "Helper.h"
#include <time.h>

// COMMENT:
//		根据链接（URL）取得对应的文件名
// INPUT:
//		szURL		-	URL
//		nLenMax		-	szFileName 的最大长度
// OUTPUT:
//		szFileNmae	-	文件名
void GetFileNameFromURL(const TCHAR *szURL, TCHAR *szFileName, int nLenMax)
{
	LPCTSTR		szBeg;

	assert(_tcsnicmp(szURL, _T("http://"), 7) == 0 || 
		_tcsnicmp(szURL, _T("ftp://"), 6) == 0);

	szBeg = _tcsrchr(szURL, '/');
	if (szBeg == NULL)
	{
		// Invalid URL format
		_tcscpy_safe(szFileName, nLenMax, _T("index.htm"));
		return;
	}

	if (*(szBeg-1) == '/')
	{
		// Invalid URL format
		_tcscpy_safe(szFileName, nLenMax, _T("index.htm"));
		return;
	}

	_tcscpy_safe(szFileName, nLenMax, szBeg + 1);

	if (_tcslen(szFileName) == 0)
		_tcscpy_safe(szFileName, nLenMax, _T("index.htm"));
}

LPCTSTR HttpErrorCodeToStr(int nCode)
{
	switch (nCode)
	{
	// case 2xx:	return "[Miscellaneous successes]";
	case 200:	return _T("OK");								// # HTTP request OK
	case 201:	return _T("Created");
	case 202:	return _T("Request recorded, will be executed later");
	case 203:	return _T("Non-authoritative information");
	case 204:	return _T("Request executed");
	case 205:	return _T("Reset document");
	case 206:	return _T("Partial Content");
	// #[Miscellaneous redirections]
	// case 3xx:	return _T("[Miscellaneous redirections]");
	case 300:	return _T("Multiple documents available");
	case 301:	return _T("Moved permanently (redirect)");
	case 302:	return _T("Moved temporarily (redirect)");
	case 303:	return _T("See other document");
	case 304:	return _T("Not Modified since last retrieval");	//	# HTTP request OK
	case 305:	return _T("Use proxy");
	case 306:	return _T("Switch proxy");
	case 307:	return _T("Moved temporarily");
	// #[Miscellaneous client/user errors]
	// case 4xx:	return _T("[Miscellaneous client/user errors]");
	case 400:	return _T("Bad Request");
	case 401:	return _T("Unauthorized");
	case 402:	return _T("Payment required");
	case 403:	return _T("Forbidden");
	case 404:	return _T("Document Not Found");
	case 405:	return _T("Method not allowed");
	case 406:	return _T("Document not acceptable to client");
	case 407:	return _T("Proxy authentication required");
	case 408:	return _T("Request Timeout");
	case 409:	return _T("Request conflicts with state of resource");
	case 410:	return _T("Document gone permanently");
	case 411:	return _T("Length required");
	case 412:	return _T("Precondition failed");
	case 413:	return _T("Request too long");
	case 414:	return _T("Requested filename too long");
	case 415:	return _T("Unsupported media type");
	case 416:	return _T("Requested range not valid");
	case 417:	return _T("Failed");
	//#[Miscellaneous server errors]
	//	case 5xx:	return _T("[Miscellaneous server errors]");
	case 500:	return _T("Internal server Error");
	case 501:	return _T("Not implemented");
	case 502:	return _T("Received bad response from real server");
	case 503:	return _T("Server busy");
	case 504:	return _T("Gateway timeout");
	case 505:	return _T("HTTP version not supported");
	case 506:	return _T("Redirection failed");
	default:
		if (nCode > 206 && nCode < 300)
			return _T("Miscellaneous successes");
		else if (nCode > 307 && nCode < 400)
			return _T("Miscellaneous redirections");
		else if (nCode > 418 && nCode < 500)
			return _T("Miscellaneous client/user errors");
		else if (nCode > 506 && nCode < 600)
			return _T("Miscellaneous server errors");
		else
			return _T("Unknown Http error code");
	// #[Unknown]
	// 'xxx'=>'[Unknown]'
	}
}

//
// $$title$$, 
//
void ParseMLTextMacro(LPCTSTR szText, tstring &str)
{
	str = szText;
	tstring		strTitleFull;

	strTitleFull = g_Player.GetArtist();
	if (!strTitleFull.empty())
		strTitleFull += _T(" - ");
	strTitleFull += g_Player.GetTitle();

	strrep(str, _T("$$title$$"), g_Player.GetTitle());
	strrep(str, _T("$$artist$$"), g_Player.GetArtist());
	strrep(str, _T("$$fullname$$"), strTitleFull.c_str());
}

void FormatPlayTime(TCHAR szBuf[], int nmsTime)
{
	int		nMinute, nSec, nHour;

	nSec = nmsTime / 1000;
	nMinute = nSec / 60;
	nSec %= 60;
	nHour = nMinute / 60;
	nMinute %= 60;

	if (nHour > 0)
		_stprintf(szBuf, _T("%d:%02d:%02d"), nHour, nMinute, nSec);
	else
		_stprintf(szBuf, _T("%02d:%02d"), nMinute, nSec);
}

void FillRectBorder(IMLGraphicsBase *pGraph, CRect &rc, int nBorderSize, CMLBrush &br)
{
	pGraph->FillRect(0, 0, rc.Width(), nBorderSize, &br);
	pGraph->FillRect(rc.right - nBorderSize, nBorderSize, nBorderSize, rc.Height() - nBorderSize * 2, &br);
	pGraph->FillRect(0, rc.Height() - nBorderSize, rc.Width(), nBorderSize, &br);
	pGraph->FillRect(0, nBorderSize, nBorderSize, rc.Height() - nBorderSize * 2, &br);
}

void GetAppNameLong(TCHAR szName[], int nLen)
{
#ifdef _MPLAYER
	_tcscpy(szName, SZ_APP_NAME);
#else
	TCHAR		szPlayer[128];
	g_Player.GetPlayerName(szPlayer, CountOf(szPlayer));
	_sntprintf(szName, nLen, _T("%s for %s"), SZ_APP_NAME, szPlayer);
#endif
}

void LogToEdit(CWidgetEditBox *editCtrl, LPCTSTR szText, BOOL bNewLine)
{
	if (bNewLine)
	{
		TCHAR		szDate[256];
#ifdef _WIN32_WCE
		_tcscpy(szDate, _T("\r\n"));
#else
		time_t		ltime;
		tm			*curtime;

		time(&ltime);
		curtime = localtime(&ltime);
		_tcsftime(szDate, CountOf(szDate), _T("\r\n%H:%M:%S  "), curtime);
#endif
		editCtrl->AppendText(szDate);
	}

	editCtrl->AppendText(szText);
}

/*
bool StatisticAnsiChar(const char *szStr)
{
	int		nAnsiChar = 0;
	int		nNoneAnsiChar = 0;

	while (*szStr)
	{
		if ((unsigned char)*szStr >= 128)
		{
			nNoneAnsiChar++;
			if (*szStr)
				szStr++;
		}
		else
			nAnsiChar++;
		szStr++;
	}

	return (nAnsiChar >= nNoneAnsiChar);
}

bool StatisticAnsiChar(const WCHAR *szStr)
{
	int		nAnsiChar = 0;
	int		nNoneAnsiChar = 0;

	while (*szStr)
	{
		if (*szStr >= 128)
			nNoneAnsiChar++;
		else
			nAnsiChar++;
		szStr++;
	}
	return (nAnsiChar >= nNoneAnsiChar * 2);
}
*/

PRAW_IMAGE_DATA LoadRawImageDataFromHBitmap(HBITMAP hBmp)
{
	BITMAP		bitmap;
	HDC			hdcDesktop = NULL;
	HBITMAP		hBmpOld = NULL;
	bool		bRet;
	PRAW_IMAGE_DATA	pImageData = NULL;
	BITMAPINFO	bmpInfo;
	BITMAPINFOHEADER	&bih = bmpInfo.bmiHeader;

	ASSERT(hBmp);
	if (GetObject(hBmp, sizeof(bitmap), &bitmap) != sizeof(bitmap))
		return NULL;

	pImageData = new RAW_IMAGE_DATA;
	pImageData->Create(bitmap.bmWidth, bitmap.bmHeight, 24);

	memset(&bmpInfo, 0, sizeof(bmpInfo));
	bih.biBitCount = (BYTE)pImageData->bitCount;
	bih.biClrUsed = 0;
	bih.biCompression = BI_RGB;
	bih.biHeight = bitmap.bmHeight;
	bih.biPlanes = 1;
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biWidth = bitmap.bmWidth;
	bih.biSizeImage = pImageData->GetBuffSize();

	hdcDesktop = ::GetDC(NULL);
	ASSERT(hdcDesktop);

	hBmpOld = (HBITMAP)SelectObject(hdcDesktop, hBmp);

#ifndef _WIN32_WCE
	if (GetDIBits(hdcDesktop, hBmp, 0, bitmap.bmHeight, pImageData->buff, &bmpInfo, DIB_RGB_COLORS))
		bRet = true;
	else
		bRet = false;
#else
	ERRLOG0(_T("Not support GetDIBits on CE"));
	bRet = false;
#endif

	SelectObject(hdcDesktop, hBmpOld);

	::ReleaseDC(NULL, hdcDesktop);

	if (bRet)
		return pImageData;
	else
	{
		FreeRawImage(pImageData);
		return NULL;
	}
}

//
// Field Read and Write
//

#define			SZ_THEME_FEILD_SEP	","

// Field format: "#00ff00,"
// or			"-1,"
bool FieldReadColor(LPCSTR &szStr, COLORREF &clr)
{
	LPCSTR			szEnd;

	while (*szStr == ' ')
		szStr++;

	if (szStr[0] == '-' && szStr[1] == '1')
	{
		clr = -1;
		szStr += 2;
	}
	else
	{
		szEnd = ReadColorValue_t(szStr, clr);
		szStr = szEnd;
	}
	if (*szStr == ',')
		szStr++;

	return true;
}

// Field format: "abcsss,"
bool FieldReadStr(LPCSTR &szStr, tstring &str)
{
	const char			*szEnd;

	while (*szStr == ' ')
		szStr++;
	szEnd = strchr(szStr, ','); if (!szEnd) return false;

#ifdef _UNICODE
	Utf8ToUCS2(szStr, (int)(szEnd - szStr), str);
#else
	str.append(szStr, (int)(szEnd - szStr));
#endif
	szStr = szEnd; szStr++;

	return true;
}

// Field format: "123,"
void FieldWriteInt(string &strField, int n)
{
	char			szBuff[64];

	itoa(n, szBuff, 10);
	strField += szBuff; strField += SZ_THEME_FEILD_SEP;
}

void ColorToStr(COLORREF clr, char szStr[]);

void FieldWriteColor(string &strField, COLORREF clr)
{
	char			szBuff[64];

	if (clr == -1)
	{
		strField += "-1";
	}
	else
	{
		ColorToStr(clr, szBuff);
		strField += szBuff;
	}
	strField += SZ_THEME_FEILD_SEP;
}

void FieldWriteStr(string &strField, LPCTSTR str)
{
	stringex	strUtf8;

#ifdef _UNICODE
	UCS2ToUtf8(str, -1, strUtf8);
	strField += strUtf8.c_str();
#else
	strField += str;
#endif

	strField += SZ_THEME_FEILD_SEP;
}

void ProfileGetColorValue(COLORREF &clr, LPCTSTR szSectName, LPCTSTR szKeyName)
{
	MLColor		c;

	c.set(clr);
	GetColorValue(c, g_Profile.GetProfileString(szSectName, szKeyName, _T("")));

	clr = c.get();
}

void ProfileGetColorValue(MLColor &clr, LPCTSTR szSectName, LPCTSTR szKeyName)
{
	GetColorValue(clr, g_Profile.GetProfileString(szSectName, szKeyName, _T("")));
}
