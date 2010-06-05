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

	pThis->m_hWndNotify->PostUserMessage(UMSG_WORK_END, 0);
}

//////////////////////////////////////////////////////////////////////


CPageWaitMessage::CPageWaitMessage(bool bEndToClose) : CSkinPageHandler(GetPageID(),
	NULL, _T("CID_AM_SHOW_WAIT"), NULL, _T("CID_AM_HIDE_WAIT"))
{
	m_bEndToClose = bEndToClose;
	m_pWorkObj = NULL;
}

LPCTSTR CPageWaitMessage::GetPageID()
{
	return _T("CID_WAIT");
}

void CPageWaitMessage::StartWork(LPCTSTR szCaption, LPCTSTR szInfo, CWorkObjBase *pWorkObj)
{
	m_pSkinWnd->SetCaptionText(szCaption);
	m_pSkinWnd->SetUIObjectText(_T("CID_WAIT_INFO"), szInfo, true);

	m_pWorkObj = pWorkObj;
	ASSERT(pWorkObj);

	m_pParent->SwitchToPage(GetPageID());
}

void CPageWaitMessage::OnShow()
{
	CSkinPageHandler::OnShow();

	if (m_pWorkObj)
	{
		m_pWorkObj->CreateWorkThread(m_pSkinWnd);
		if (m_pWorkObj->IsCancelEnabled())
			EnableUIObject(CMD_CANCEL, true);
		else
			EnableUIObject(CMD_CANCEL, false);
	}
}

void CPageWaitMessage::OnOK()
{
}

void CPageWaitMessage::OnCancel()
{
	if (m_pWorkObj && m_pWorkObj->IsCancelEnabled())
	{
		m_pWorkObj->CancelJob();
	}
}

void CPageWaitMessage::OnUserMessage(int nMessageID, LPARAM param)
{
	if (nMessageID == UMSG_WORK_END)
	{
		if (m_bEndToClose)
			CSkinPageHandler::OnCancel();
		else
			m_pParent->SwitchToLastPage();
	}
}

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

void CWaitingDlg::OnUserMessage(int nMessageID, LPARAM param)
{
	if (nMessageID == UMSG_WORK_END)
	{
		if (m_pWorkObj && m_pWorkObj->IsJobCanceled())
			CBaseDialog::OnCancel();
		else
			CBaseDialog::OnOK();
	}
}
