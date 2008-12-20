// WaitingDlg.h: interface for the CWaitingDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WAITINGDLG_H__8E8BBE95_0587_4DEB_9DAF_7B3769589439__INCLUDED_)
#define AFX_WAITINGDLG_H__8E8BBE95_0587_4DEB_9DAF_7B3769589439__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#ifdef _WIN32
#define WM_WORKTHREAD_END_NOTIFY		(WM_USER+1)
#endif

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

#ifdef _WIN32
	LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);
#endif

protected:
	CWorkObjBase	*m_pWorkObj;

	tstring			m_strTitle, m_strMsg;

};

#endif // !defined(AFX_WAITINGDLG_H__8E8BBE95_0587_4DEB_9DAF_7B3769589439__INCLUDED_)
