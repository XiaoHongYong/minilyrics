
// ID3v2TagRemoverDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ID3v2TagRemover.h"
#include "ID3v2TagRemoverDlg.h"
#include "../ID3Lib/ID3v2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CID3v2TagRemoverDlg dialog




CID3v2TagRemoverDlg::CID3v2TagRemoverDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CID3v2TagRemoverDlg::IDD, pParent)
	, m_strDir(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CID3v2TagRemoverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_E_FOLDER, m_strDir);
}

BEGIN_MESSAGE_MAP(CID3v2TagRemoverDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BR_FOLDER, &CID3v2TagRemoverDlg::OnBnClickedBrFolder)
	ON_BN_CLICKED(IDOK, &CID3v2TagRemoverDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CID3v2TagRemoverDlg message handlers

BOOL CID3v2TagRemoverDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CID3v2TagRemoverDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CID3v2TagRemoverDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CID3v2TagRemoverDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CID3v2TagRemoverDlg::OnBnClickedBrFolder()
{
	TCHAR	szDir[MAX_PATH];

	if (BrowserForFolder(m_hWnd, _T("Please browse the folder contains the corrupted files by MiniLyrics:"), szDir))
	{
		m_strDir = szDir;
		UpdateData(FALSE);
	}
}

class ID3v2Remover
{
public:
	int			nNotFound, nFailedCount, nSucceeded;
	VecString	vFiles;

public:
	ID3v2Remover()
	{
		nNotFound = 0;
		nFailedCount = 0;
		nSucceeded = 0;
	}
	void GetToFixFiles(LPCTSTR szDir)
	{
		CMLFileFind	finder;

		if (!finder.OpenDir(szDir))
			return;

		while (finder.FindNext())
		{
			tstring file = DirStringJoint(szDir, finder.GetCurName());
			if (finder.IsCurDir())
			{
				DirStringAddSlash(file);
				GetToFixFiles(file.c_str());
			}
			else if (FileIsExtSame(finder.GetCurName(), _T(".m4a")))
			{
				CID3v2		id3v2(ED_SYSDEF);

				int nRet = id3v2.Open(file.c_str(), false, false);
				if (nRet == ERR_NOT_FOUND_ID3V2)
					nNotFound++;
				else if (nRet == ERR_OK)
					vFiles.push_back(file);
				else
					nFailedCount++;
			}
		}
	}

	void Remove()
	{
		for (int i = 0; i < vFiles.size(); i++)
		{
			CID3v2		id3v2(ED_SYSDEF);

			int nRet = id3v2.Open(vFiles[i].c_str(), true, false);
			if (nRet == ERR_OK)
			{
				nRet = id3v2.RemoveTagAndClose();
				if (nRet == ERR_OK)
					nSucceeded++;
				else
					nFailedCount++;
			}
		}
	}

};

void CID3v2TagRemoverDlg::OnBnClickedOk()
{
	UpdateData();

	if (m_strDir.IsEmpty())
	{
		AfxMessageBox(_T("Please browse the folder contains corrupted m4a files."));
		return;
	}

	ID3v2Remover	remover;

	remover.GetToFixFiles(m_strDir);

	if (remover.vFiles.size() > 0)
	{
		tstring msg = _T("Are you sure that you want to fix these files?\r\n");

		int n = min(remover.vFiles.size(), 5);
		for (int i = 0; i < n; i++)
			msg += remover.vFiles[i] + _T("\r\n");
		if (n < remover.vFiles.size())
			msg += CStrPrintf(_T("...\r\nTotal %d files\r\n"), remover.vFiles.size()).c_str();
		else
			msg += _T("\r\n");

		int nRet = AfxMessageBox(msg.c_str(), MB_OKCANCEL | MB_ICONQUESTION);
		if (nRet != IDOK)
			return;

		remover.Remove();
	}

	tstring msg;
	msg += CStrPrintf(_T("%d files were fixed.\r\n"), remover.nSucceeded).c_str();
	if (remover.nFailedCount > 0)
		msg += CStrPrintf(_T("%d files were failed to get info or fixed.\r\n"), remover.nFailedCount).c_str();
	if (remover.nNotFound > 0)
		msg += CStrPrintf(_T("%d files are OK, and needn't to fix.\r\n"), remover.nNotFound).c_str();
	
	AfxMessageBox(msg.c_str());

	// OnOK();
}
