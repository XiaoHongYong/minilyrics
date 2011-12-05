// DlgUpdateLibrary.cpp: implementation of the CDlgUpdateLibrary class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DlgUpdateLibrary.h"
#include "MPlayerAppBase.h"
#include "Player.h"

#define WM_WORKTHREAD_END_NOTIFY (WM_USER+1) 

CUpdateLibraryObj::CUpdateLibraryObj()
{
	m_pWnd = NULL;
}

CUpdateLibraryObj::~CUpdateLibraryObj()
{
	m_thread.Join();
}

void CUpdateLibraryObj::CreateWorkThread(CBaseWnd *hWndNotify)
{
	DWORD	dwThreadId = 0;

	m_pWnd = hWndNotify;
	m_thread.Create(WorkThread, this);
}

void CUpdateLibraryObj::WorkThread(LPVOID lpParam)
{
	CUpdateLibraryObj		*pThis;

	pThis = (CUpdateLibraryObj *)lpParam;
	pThis->DoUpdating();

#ifdef _WIN32
	PostMessage(pThis->m_pWnd->GetHandle(), WM_WORKTHREAD_END_NOTIFY, 0, 0);
#endif
}

void CUpdateLibraryObj::SetMsg1(LPCTSTR szMsg)
{
	m_pWnd->SetDlgItemText(IDC_S_MSG1, szMsg);
}

void CUpdateLibraryObj::SetMsg2(LPCTSTR szMsg)
{
	m_pWnd->SetDlgItemText(IDC_S_MSG2, szMsg);
}

//////////////////////////////////////////////////////////////////////////

void CAddMediaObj::DoUpdating()
{
	CMPAutoPtr<IMediaLibrary>	pMediaLib;
	int							nRet;
	TCHAR						szMsg[256];
	int							n = 0;

	if (!m_bAddFiles)
	{
		_stprintf(szMsg, TOLOCAL(_T("%d files found")), 0);
		SetMsg1(szMsg);
		SetMsg2(TOLOCAL(_T("Searching for files...")));
		ListMedia(m_strDir.c_str());
	}

	_stprintf(szMsg, TOLOCAL(_T("%d files added")), 0);
	SetMsg1(szMsg);
	SetMsg2(TOLOCAL(_T("Adding files...")));

	if (g_Player.GetMediaLibrary(&pMediaLib) == ERR_OK)
	{
		for (int i = 0; i < (int)m_vFiles.size(); i++)
		{
			CMPAutoPtr<IMedia>			pMedia;

			if (pMediaLib->GetMediaByUrl(m_vFiles[i].c_str(), &pMedia) == ERR_OK)
				continue;

			nRet = pMediaLib->Add(m_vFiles[i].c_str(), &pMedia);
			if (nRet == ERR_OK)
			{
				n++;
				_stprintf(szMsg, TOLOCAL(_T("%d files added")), n);
				SetMsg1(szMsg);
			}
		}
	}

	SetMsg2(TOLOCAL(_T("The library has been updated. To continue, click Done.")));
}

void CAddMediaObj::ListMedia(LPCTSTR szDir)
{
	CMLFileFind		find;
	tstring			strDir;
	tstring			strFile;
	TCHAR			szMsg[256];

	if (!find.OpenDir(szDir))
		return;

	strDir = szDir;
	DirStringFormat(strDir);

	while (find.FindNext())
	{
		if (find.IsCurDir())
		{
			if (_tcscmp(_T("."), find.GetCurName()) != 0 &&
				_tcscmp(_T(".."), find.GetCurName()) != 0)
			{
				strFile = strDir + find.GetCurName();
				ListMedia(strFile.c_str());
			}
		}
		else
		{
			if (g_Player.IsExtAudioFile(FileGetExt(find.GetCurName())))
			{
				strFile = strDir + find.GetCurName();
				m_vFiles.push_back(strFile);
				_stprintf(szMsg, TOLOCAL(_T("%d files found")), m_vFiles.size());
				SetMsg1(szMsg);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDlgUpdateLibrary::CDlgUpdateLibrary() : CBaseDialog(IDD_UPDATE_LIBRARY)
{
	m_pWorkObj = NULL;
}

CDlgUpdateLibrary::~CDlgUpdateLibrary()
{
}

BOOL CDlgUpdateLibrary::OnInitDialog()
{
	CBaseDialog::OnInitDialog();

	SetDlgItemText(IDC_S_MSG1, _T(""));
	SetDlgItemText(IDC_S_MSG2, _T(""));

	m_bCanQuit = false;
	EnableDlgItem(IDOK, false);
	if (m_pWorkObj)
		m_pWorkObj->CreateWorkThread(this);

	CeSHInitFullScrDlg(this);

	return TRUE;
}

BOOL CDlgUpdateLibrary::DoModal(CBaseWnd *pWndParent, CUpdateLibraryObj *pWorkObj)
{
	m_pWorkObj = pWorkObj;

	return CBaseDialog::DoModal(pWndParent);
}

void CDlgUpdateLibrary::OnCancel()
{
	if (m_bCanQuit)
		CBaseDialog::OnCancel();
}

void CDlgUpdateLibrary::OnOK()
{
	if (m_bCanQuit)
		CBaseDialog::OnOK();
}

#ifdef _WIN32
LRESULT CDlgUpdateLibrary::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_WORKTHREAD_END_NOTIFY)
	{
		m_bCanQuit = true;
		EnableDlgItem(IDOK, true);

		return 0;
	}
	else
		return CBaseDialog::WndProc(message, wParam, lParam);
}
#endif
