// WaitingDlg.cpp: implementation of the CWaitingDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "WaitingDlg.h"
#include "LyricsDownloadMgr.h"
#include "Helper.h"

CWorkObjBase::CWorkObjBase()
{
	m_hWndNotify = NULL;
	m_bEnableCancel = false;
	m_bJobCanceled = false;
}

CWorkObjBase::~CWorkObjBase()
{
	m_threadWork.Join();
}

void CWorkObjBase::CreateWorkThread(CBaseWnd *hWndNotify)
{
	m_hWndNotify = hWndNotify;
	m_threadWork.Create(WorkThread, this);
}

DWORD CWorkObjBase::DoTheJob()
{
	return 0;
}

void CWorkObjBase::WorkThread(LPVOID lpParam)
{
	CWorkObjBase		*pThis;
	DWORD				dwRet;

	pThis = (CWorkObjBase *)lpParam;
	dwRet = pThis->DoTheJob();

#ifdef _WIN32
	PostMessage(pThis->m_hWndNotify->GetHandle(), WM_WORKTHREAD_END_NOTIFY, 0, 0);
#else
	pThis->m_hWndNotify->Destroy();
#endif
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWaitingDlg::CWaitingDlg() : CBaseDialog(IDD_WAITING)
{
	m_pWorkObj = NULL;
}

CWaitingDlg::~CWaitingDlg()
{
}

BOOL CWaitingDlg::OnInitDialog()
{
	CBaseDialog::OnInitDialog();

	SetWindowText(m_strTitle.c_str());
	SetDlgItemText(IDC_MSG, m_strMsg.c_str());

	if (m_pWorkObj)
	{
		m_pWorkObj->CreateWorkThread(this);
		if (m_pWorkObj->IsCancelEnabled())
			EnableDlgItem(IDCANCEL, true);
	}

	return TRUE;
}

int CWaitingDlg::ShowWaitingDlg(CBaseWnd *pWndParent, CWorkObjBase *pWorkObj, LPCTSTR szTitle, LPCTSTR szMsg)
{
	m_pWorkObj = pWorkObj;
	m_strTitle = szTitle;
	m_strMsg = szMsg;

	return DoModal(pWndParent);
}

void CWaitingDlg::OnCancel()
{
	if (m_pWorkObj && m_pWorkObj->IsCancelEnabled())
	{
		m_pWorkObj->CancelJob();
	}
}

void CWaitingDlg::OnOK()
{
}

#ifdef _WIN32
LRESULT CWaitingDlg::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_WORKTHREAD_END_NOTIFY)
	{
		if (m_pWorkObj && m_pWorkObj->IsJobCanceled())
			CBaseDialog::OnCancel();
		else
			CBaseDialog::OnOK();
		return 0;
	}
	else
		return CBaseDialog::WndProc(message, wParam, lParam);
}
#endif
