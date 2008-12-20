// ShellNotifyIcon.cpp: implementation of the CShellNotifyIcon class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ShellNotifyIcon.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CShellNotifyIcon::CShellNotifyIcon()
{

}

CShellNotifyIcon::~CShellNotifyIcon()
{

}

bool CShellNotifyIcon::AddIcon(CBaseWnd *pWnd, UINT nID, LPCTSTR szTip, HICON hIcon, UINT nCallbackMessage)
{
	NOTIFYICONDATA		nif;

	nif.cbSize = sizeof(nif);
	nif.hWnd = pWnd->GetHandle();
	nif.hIcon = hIcon;
	_tcscpy_safe(nif.szTip, CountOf(nif.szTip), szTip);
	nif.uID = nID;
	nif.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	nif.uCallbackMessage = nCallbackMessage;

	return Tobool(::Shell_NotifyIcon(NIM_ADD, &nif));
}

bool CShellNotifyIcon::DelIcon(CBaseWnd *pWnd, UINT nID)
{
	NOTIFYICONDATA		nif;

	nif.cbSize = sizeof(nif);
	nif.uID = nID;
	nif.hWnd = pWnd->GetHandle();
	nif.uFlags = 0;

	return Tobool(::Shell_NotifyIcon(NIM_DELETE, &nif));
}

bool CShellNotifyIcon::ModifyIcon(CBaseWnd *pWnd, UINT nID, LPCTSTR szTip, HICON hIcon)
{
	NOTIFYICONDATA		nif;

	nif.cbSize = sizeof(nif);
	nif.hWnd = pWnd->GetHandle();
	if (szTip)
		_tcscpy_safe(nif.szTip, CountOf(nif.szTip), szTip);
	nif.uID = nID;
	nif.hIcon = hIcon;
	nif.uFlags = 0;
	if (szTip)
		nif.uFlags |= NIF_TIP;
	if (hIcon)
		nif.uFlags |= NIF_ICON;

	return Tobool(::Shell_NotifyIcon(NIM_MODIFY, &nif));
}
