// ErrorResolveDlg.cpp: implementation of the CErrorResolveDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "ErrorResolveDlg.h"
#include "Helper.h"
#include "PreferenceDlg.h"

void ShowInetErrorDlg(CBaseWnd *pWnd, int nError)
{
	tstring		strError;
	tstring		strResolve;
	TCHAR		szErrorId[256];

	_stprintf_s(szErrorId, CountOf(szErrorId), _T("%s %d,  "), _TLT("Error Code:"), nError);
	strError = szErrorId;
	strError += ERROR2STR_LOCAL(nError);
	strError += _T("\r\n\r\n");
	strError += _TLT("Do you want to visit our website for more information?");
	if (pWnd->MessageOut(strError.c_str(), MB_ICONINFORMATION | MB_OKCANCEL) == IDOK) {
		OpenUrl(pWnd, GetStrName(SN_HTTP_FAQ_INET));
	}
}
