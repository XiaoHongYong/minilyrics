// WaitingDlg.h: interface for the CWaitingDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WAITINGDLG_H__8E8BBE95_0587_4DEB_9DAF_7B3769589439__INCLUDED_)
#define AFX_WAITINGDLG_H__8E8BBE95_0587_4DEB_9DAF_7B3769589439__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MPSkinDialog.h"

#define UMSG_WORK_END		1

class CWorkObjBase
{
public:
	CWorkObjBase();
	virtual ~CWorkObjBase();

	virtual DWORD DoTheJob();

	virtual void CancelJob() { m_bJobCanceled = true; }

	void CreateWorkThread(CBaseWnd *hWndNotify);

	static void WorkThread(LPVOID lpParam);

	bool IsCancelEnabled() { return m_bEnableCancel; }

	bool IsJobCanceled() { return m_bJobCanceled; }

protected:
	CBaseWnd		*m_hWndNotify;
	CThread			m_threadWork;
	bool			m_bEnableCancel;
	bool			m_bJobCanceled;

};


class CPageWaitMessage : public CSkinPageHandler
{
public:
	CPageWaitMessage(bool bEndToClose);

	~CPageWaitMessage()
	{
	}

	static LPCTSTR GetPageID();

	void StartWork(LPCTSTR szCaption, LPCTSTR szInfo, CWorkObjBase *pWorkObj);

	void OnShow();

	void OnOK();

	void OnCancel();

	void OnUserMessage(int nMessageID, LPARAM param);

protected:
	bool			m_bEndToClose;
	CWorkObjBase	*m_pWorkObj;

};

//
//
class CWaitingDlg : public CBaseDialog  
{
public:
	CWaitingDlg();
	virtual ~CWaitingDlg();

	BOOL OnInitDialog();

	virtual void OnCancel();
	virtual void OnOK();

	int ShowWaitingDlg(CBaseWnd *pWndParent, CWorkObjBase *pWorkObj, LPCTSTR szTitle, LPCTSTR szMsg);


	virtual void OnUserMessage(int nMessageID, LPARAM param);

protected:
	CWorkObjBase	*m_pWorkObj;

	tstring			m_strTitle, m_strMsg;

};

#endif // !defined(AFX_WAITINGDLG_H__8E8BBE95_0587_4DEB_9DAF_7B3769589439__INCLUDED_)
