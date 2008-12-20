// ShellNotifyIcon.h: interface for the CShellNotifyIcon class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SHELLNOTIFYICON_H__237180DB_EC4C_465C_8833_0476272F3281__INCLUDED_)
#define AFX_SHELLNOTIFYICON_H__237180DB_EC4C_465C_8833_0476272F3281__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CShellNotifyIcon
{
public:
	CShellNotifyIcon();
	~CShellNotifyIcon();

	static bool AddIcon(CBaseWnd *pWnd, UINT nID, LPCTSTR szTip, HICON hIcon, UINT nCallbackMessage);
	static bool DelIcon(CBaseWnd *pWnd, UINT nID);
	static bool ModifyIcon(CBaseWnd *pWnd, UINT nID, LPCTSTR szTip, HICON hIcon = NULL);

};

#endif // !defined(AFX_SHELLNOTIFYICON_H__237180DB_EC4C_465C_8833_0476272F3281__INCLUDED_)
