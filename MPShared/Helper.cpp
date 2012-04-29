// Helper.cpp: implementation of the Helper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "Helper.h"
#include <time.h>

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

	strTitleFull = FormatMediaTitle(g_Player.GetArtist(), g_Player.GetTitle());

	strrep(str, _T("$$title$$"), g_Player.GetTitle());
	strrep(str, _T("$$artist$$"), g_Player.GetArtist());
	strrep(str, _T("$$fullname$$"), strTitleFull.c_str());
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

void DeleteAllFiles(LPCTSTR szDir, BOOL bIncSubDir, BOOL bDelReadOnlyFile)
{
	CMLFileFind			find;
	tstring				strDir = szDir;

	DirStringFormat(strDir);

	if (!find.OpenDir(szDir))
		return;

	while (find.FindNext())
	{
		tstring		strFile;
		strFile = strDir + find.GetCurName();
		if (find.IsCurDir())
		{
			if (bIncSubDir)
			{
				DeleteAllFiles(strFile.c_str(), bIncSubDir, bDelReadOnlyFile);
				RemoveDirectory(strFile.c_str());
			}
		}
		else
		{
			if (!DeleteFile(strFile.c_str()) && bDelReadOnlyFile)
			{
				SetFileNoReadOnly(strFile.c_str());
				DeleteFile(strFile.c_str());
			}
		}
	}
}
