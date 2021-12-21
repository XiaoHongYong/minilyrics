// Helper.cpp: implementation of the Helper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "../MPlayerApp.h"
#include "../Helper.h"

int CanOpenFileForWrite(LPCTSTR szFile, bool bMustExist)
{
	FILE		*fp;

	fp = _tfopen(szFile, _T("r+b"));
	if (!fp)
		return ERR_FALSE;

	fclose(fp);

	return ERR_OK;
}

void Execute(CBaseWnd *pWnd, LPCTSTR szExe, LPCTSTR szParam)
{
}

bool SetClipboardText(CBaseWnd *pWnd, LPCTSTR szText)
{
	gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_PRIMARY), szText, _tcslen(szText));
	gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), szText, _tcslen(szText));
}

// 向用户显示消息框
int MessageOut(LPCTSTR lpText, LPCTSTR szSaveDefault,CBaseWnd *pWnd/* = NULL*/, UINT uType/* = MB_ICONINFORMATION*/, LPCTSTR lpCaption/* = NULL*/)
{
	GtkWidget		*msgDlg;
	int				nRet;

	GtkMessageType		msgType;
	GtkButtonsType		btType;
	if (IsFlagSet(uType, MB_OK))
		btType = GTK_BUTTONS_OK;
	else if (IsFlagSet(uType, MB_YESNO))
		btType = GTK_BUTTONS_YES_NO;
	else if (IsFlagSet(uType, MB_OKCANCEL))
		btType = GTK_BUTTONS_OK_CANCEL;
	else
		btType = GTK_BUTTONS_OK;

	if (IsFlagSet(uType, MB_ICONWARNING))
		msgType = GTK_MESSAGE_WARNING;
	else if (IsFlagSet(uType, MB_ICONQUESTION))
		msgType = GTK_MESSAGE_QUESTION;
	else if (IsFlagSet(uType, MB_ICONINFORMATION))
		msgType = GTK_MESSAGE_INFO;
	else if (IsFlagSet(uType, MB_ICONERROR))
		msgType = GTK_MESSAGE_ERROR;
	else
		msgType = GTK_MESSAGE_INFO;

	msgDlg = gtk_message_dialog_new(pWnd ? GTK_WINDOW(pWnd->GetHandle()) : NULL, GTK_DIALOG_MODAL, msgType, btType,  "%s", lpText);
	if (lpCaption)
		gtk_window_set_title(GTK_WINDOW(msgDlg), lpCaption);
	nRet = gtk_dialog_run(GTK_DIALOG (msgDlg));
	gtk_widget_destroy (msgDlg);

	return nRet;
}

BOOL SHDeleteFile(LPCTSTR szFile, CBaseWnd *pWndParent)
{
	return DeleteFile(szFile);
}

BOOL SHCopyFile(LPCTSTR szSrcFile, LPCTSTR szTargFile, CBaseWnd *pWndParent)
{
	// return CopyFile(szSrcFile, szTargFile, true);
	return false;
}

BOOL SetFileNoReadOnly(LPCTSTR szFile)
{
	return FALSE;
}

bool LoadProxySvrFromIE(bool &bUseProxy, tstring &strSvr, int &nPort)
{
	return false;
}

void GetNotepadEditor(tstring &strEditor)
{
}

uint32 GetSecCount()
{
	time_t		curTime;
	return time(&curTime);
}
