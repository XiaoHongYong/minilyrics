// BaseDialog.h: interface for the CBaseDialog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BASEDIALOG_H__97C9A49F_2742_4A43_95A3_E25F59A568FF__INCLUDED_)
#define AFX_BASEDIALOG_H__97C9A49F_2742_4A43_95A3_E25F59A568FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseWnd.h"
#include "../ResourceMgr.h"

#ifndef IDOK
#define IDOK                GTK_RESPONSE_OK
#define IDCANCEL            GTK_RESPONSE_CANCEL
// #define IDABORT             3
// #define IDRETRY             4
// #define IDIGNORE            5
#define IDYES               GTK_RESPONSE_YES
#define IDNO                GTK_RESPONSE_NO
#define IDCLOSE				GTK_RESPONSE_CLOSE
#define IDHELP				GTK_RESPONSE_HELP
//				GTK_RESPONSE_APPLY
#endif

class CWidgetWin32Layout : public CBaseWnd
{
public:
	CWidgetWin32Layout()
	{
		m_dlgRes = NULL;
	}

	virtual bool Create(CBaseWnd *pWndParent, CDlgResource *pdlgRes);

	void PutChild(CBaseWnd *pChild, int x, int y);

	virtual bool ReplaceChildPos(int nIDChildSrcPos, CBaseWnd *pChildNew);

protected:
	int InitCreateDlgItems();

	static void Cb_SizeAllocate(GtkWidget *widget, GtkAllocation *allocation, gpointer user_data);

	void OnSizeAllocate(GtkAllocation *allocation);

	void ReAllocateItemSize(CDlgResource::CWidgetItem &item);

	int CreateWidget(CDlgResource::CWidgetItem &item);

protected:
	CDlgResource		*m_dlgRes;

};

class CBaseDialog : public CBaseWnd  
{
public:
	virtual BOOL Create(UINT nIDTemplate, CBaseWnd *pWndParent);

	virtual bool Destroy();

	int DoModal(CBaseWnd *pWndParent);
	BOOL IsModal();

	virtual void OnCancel();
	virtual void OnOK();

	void OnCommand(UINT uID, UINT nNotifyCode);
	virtual BOOL OnInitDialog();

	virtual void CheckButton(int nIDButton, bool bCheck);
	virtual bool IsButtonChecked(int nIDButton);

	UINT GetDlgItemText(int nIDItem, LPTSTR szString, int nMaxCount);
	UINT GetDlgItemText(int nIDItem, tstring &str);
	BOOL SetDlgItemText(int nIDItem, LPCTSTR szString);
	UINT GetDlgItemInt(int nIDItem, BOOL *pTranslated, BOOL bSigned);
	BOOL SetDlgItemInt(int nIDItem, UINT uValue, BOOL bSigned);
	bool EnableDlgItem(int nIDItem, bool bEnable);
	bool ShowDlgItem(int nIDItem, int nCmdShow);
	CBaseWnd *GetDlgItem(int nIDItem);
	bool GetDlgItemRect(int nIDItem, RECT* lpRect);

	virtual bool ReplaceChildPos(int nIDChildSrcPos, CBaseWnd *pChildNew);

	GtkWidget *GetDlgItemHandle(int nIDItem);

protected:
	void ConnectGtkSignals();

	static void cb_response(GtkDialog *dialog, gint arg1, gpointer user_data);

public:
	CBaseDialog(UINT nIDTemplate);
	virtual ~CBaseDialog();

	BOOL		m_bModal;	// 是否为Modal Dialog

	CDlgResource		m_dlgRes;
	UINT				m_nIDTemplate;
	CWidgetWin32Layout	m_layout;

};

#endif // !defined(AFX_BASEDIALOG_H__97C9A49F_2742_4A43_95A3_E25F59A568FF__INCLUDED_)
