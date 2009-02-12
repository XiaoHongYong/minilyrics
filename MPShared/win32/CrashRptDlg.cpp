// CrashHandler.cpp: implementation of the CCrashHandler class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "crashRptDlg.h"

#define MAIL_RT	_T("%0d%0a")

BOOL MLCrashCallback(LPCTSTR szRpt, HMODULE hCrashMod, LPVOID lpUserData)
{
	CCrashRptDlg	dlg;

	// dlg.m_strRpt = szRpt;
	tstring		&strRptShort = dlg.m_strRptShort;

	strRptShort;

	TCHAR	szMLModue[256];
#ifndef _MPLAYER
	_stprintf_s(szMLModue, CountOf(szMLModue), _T("Version: \"%s\", MiniLyrics HINST= 0x%X, Plugin HINST= 0x%X"), SZ_ML_VERSION, (long)GetAppInstance(), (long)g_Player.GetPlayerPlugInstance());
#else
	_stprintf_s(szMLModue, CountOf(szMLModue), _T("Version: \"%s\", ZikiPlayer HINST= 0x%X"), SZ_ML_VERSION, (long)GetAppInstance());
#endif
	// LOG1(LOG_LVL_ERROR, "%s", szMLModue);

	strRptShort += szMLModue;
	strRptShort += MAIL_RT;
	strRptShort += szRpt;

	strrep(strRptShort, _T("\r\n"), MAIL_RT);
	strrep(strRptShort, _T("\n"), MAIL_RT);


#ifdef _MPLAYER
	if (hCrashMod == GetAppInstance())
#else
	if (hCrashMod == GetAppInstance() || hCrashMod == g_Player.GetPlayerPlugInstance())
#endif
	{
		// 异常发生在我们的代码之间
		TCHAR	szRptFile[MAX_PATH];

		_stprintf_s(szRptFile, CountOf(szRptFile), _T("%sMLCrashRpt.txt"), GetAppDataDir());
		FILE	*fp;

		fp = _tfopen(szRptFile, _T("a+"));
		if (fp)
		{
			LPCTSTR		szSep = _T("==============================\r\n");
			fwrite(szSep, sizeof(TCHAR), _tcslen(szSep), fp);
			fwrite(szMLModue, sizeof(TCHAR), _tcslen(szMLModue), fp);
			fwrite(_T("\r\n"), sizeof(TCHAR), 2, fp);
			fwrite(szRpt, sizeof(TCHAR), _tcslen(szRpt), fp);
			fwrite(_T("\r\n"), sizeof(TCHAR), 2, fp);
			fclose(fp);
		}

//		tstring	strExcpFileTitle;
//		strExcpFileTitle = _T("MLCrashRpt");
//		char	szRptFile[MAX_PATH];
//
//		//
//		// Save the crash report to CrashRpt-x.txt
//		//
//		for (int i = 0; i < 10; i++)
//		{
//			wsprintf(szRptFile, _T("%s%s-%d.txt"), GetAppDataDir(), strExcpFileTitle.c_str(), i);
//			if (!IsFileExist(szRptFile))
//			{
//				break;
//			}
//		}
//		SaveStringAsFile(szRptFile, szRpt, strlen(szRpt));
//
//		i = (i + 1) % 10;
//		wsprintf(szRptFile, _T("%s%s-%d.txt"), GetAppDataDir(), strExcpFileTitle.c_str(), i);
//		if (IsFileExist(szRptFile))
//		{
//			DeleteFile(szRptFile);
//		}

		//
		// 显示报告对话框
		dlg.DoModal(g_mpApp.GetMainWnd());
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CCrashRptDlg::OnInitDialog()
{
	CBaseDialog::OnInitDialog();

	return TRUE;
}

void CCrashRptDlg::OnOK()
{
	TCHAR		szText[1024] = _T("");

	tstring		strMail;

	strMail = GetStrName(SN_EMAIL);
	strMail += _T("?subject=");
	strMail += SZ_ML_VERSION;
	strMail += _T(" exceptions&body=");
	strMail += m_strRptShort;

	GetDlgItemText(IDC_PROBLEM, szText, CountOf(szText));
	strMail += MAIL_RT _T("Problem: ");
	strMail += szText;

	GetDlgItemText(IDC_HOW_MAKE_PROBLEM, szText, CountOf(szText));
	strMail += MAIL_RT _T("HowTo: ");
	strMail += szText;

	ShellExecute(NULL, _T("open"), strMail.c_str(), NULL, NULL, SW_SHOWNORMAL);

	CBaseDialog::OnOK();
}

