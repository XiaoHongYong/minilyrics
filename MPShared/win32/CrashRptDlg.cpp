// CrashHandler.cpp: implementation of the CCrashHandler class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "crashRptDlg.h"

#ifdef _WIN32_DESKTOP

#include "../Utils/win32/MailFileTo.h"

CMiniDumperNotify	dumpNotify;

bool CMiniDumperNotify::OnBeginDump(HMODULE hCrashMod, TCHAR szDumpFileToSave[], int nLen)
{
#ifndef _MPLAYER
	// Only except occurs in our module, send this bug report.
	if (hCrashMod != GetAppInstance() && hCrashMod != g_Player.GetPlayerPlugInstance())
		return false;
#endif

	GetWorkingFolder(szDumpFileToSave);
	_tcscat_safe(szDumpFileToSave, nLen, SZ_APP_NAME _T("_crash.dmp"));

	return true;
}

bool CMiniDumperNotify::OnDumpFinished(HMODULE hCrashMod, LPCTSTR szDumpFileToSave)
{
	CCrashRptDlg	dlg;

	dlg.m_strDumpFile = szDumpFileToSave;

	// Show error report dialog
	dlg.DoModal(CMPlayerAppBase::GetMainWnd());

	return true;
}


BOOL CCrashRptDlg::OnInitDialog()
{
	CBaseDialog::OnInitDialog();

	return TRUE;
}

void CCrashRptDlg::OnOK()
{
	CMailFileTo		mail;
	tstring			strSubjet;

	strSubjet = GetAppNameLong();
	strSubjet += _T(" Minidumps");

	CBaseDialog::OnOK();

	mail.SendMail(HWND_DESKTOP, SZ_COMPANY_NAME _T(" support"), GetStrName(SN_SUPPORT_MAIL), m_strDumpFile.c_str(), strSubjet.c_str());
}

#endif // #ifdef _WIN32_DESKTOP
