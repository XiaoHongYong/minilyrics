// WndCrackDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WndCrack.h"
#include "WndCrackDlg.h"
#include "SetLayedOptDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define EnableDlgItem(hWnd, nid, bEnalbe)		::EnableWindow(::GetDlgItem(hWnd, nid), bEnalbe)

BOOL	MyScreenToClient(HWND hWnd, CRect &rc)
{
	CPoint	pt1;
	pt1.x = rc.left;
	pt1.y = rc.top;
//	ScreenToClient(hWnd, (LPPOINT)&rc);
//	ScreenToClient(hWnd, (LPPOINT)(&rc + sizeof(POINT)));
	ScreenToClient(hWnd, &pt1);
	rc.left = pt1.x;
	rc.top = pt1.y;
	pt1.x = rc.right;
	pt1.y = rc.bottom;
	ScreenToClient(hWnd, &pt1);
	rc.right = pt1.x;
	rc.bottom = pt1.y;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWndCrackDlg dialog

CWndCrackDlg::CWndCrackDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWndCrackDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWndCrackDlg)
	m_strWndCaptured = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_dwProcessID = 0;

	m_hbrColor = NULL;
}

void CWndCrackDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWndCrackDlg)
	DDX_Control(pDX, IDC_TREE_WND, m_wndTree);
	DDX_Text(pDX, IDC_HWND_CAPTURED, m_strWndCaptured);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CWndCrackDlg, CDialog)
	//{{AFX_MSG_MAP(CWndCrackDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_TOPMOST, OnTopmost)
	ON_BN_CLICKED(IDC_SHOWWINDOW, OnShowwindow)
	ON_BN_CLICKED(IDC_MOVEWINDOW, OnMovewindow)
	ON_BN_CLICKED(IDC_CLOSEWND, OnClosewnd)
	ON_BN_CLICKED(IDC_UPDATE, OnUpdate)
	ON_BN_CLICKED(IDC_SET_WND_CAPTION, OnSetWndCaption)
	ON_BN_CLICKED(IDC_MINIMIZE_WND, OnMinimizeWnd)
	ON_BN_CLICKED(IDC_SET_WND_OPAQUE, OnSetWndOpaque)
	ON_BN_CLICKED(IDC_ENABLE_WND, OnEnableWnd)
	ON_BN_CLICKED(IDC_ENUM, OnEnum)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_WND, OnSelchangedTreeWnd)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_MAXIMIZE_WND, &CWndCrackDlg::OnBnClickedMaximizeWnd)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWndCrackDlg message handlers

BOOL CWndCrackDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
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
	m_hWndMouseSelect = NULL;
	GetDlgItem(IDC_SELWND)->GetWindowRect(&m_rcSelectingBt);
	GetDlgItem(IDC_SELWND)->ShowWindow(SW_HIDE);//>GetWindowRect(&m_rcSelectingBt);
	ScreenToClient(&m_rcSelectingBt);

	m_hcursorSelecting = LoadCursor(AfxGetResourceHandle(), MAKEINTRESOURCE(IDC_CURSORSELWND));
	m_bSelectingWnd = FALSE;
	m_hPenWhite = CreatePen(PS_SOLID, 3, RGB(255, 255, 255));

	if (!IsLayeredWndSupported())
		EnableDlgItem(m_hWnd, IDC_SET_WND_OPAQUE, FALSE);


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWndCrackDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CWndCrackDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	if (IsIconic())
	{
		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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
		HICON	hIcon;

		if (m_bSelectingWnd)
			hIcon = LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_DRAGAFTER));
		else
			hIcon = LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_DRAGBEFORE));

		dc.DrawIcon(m_rcSelectingBt.left, m_rcSelectingBt.top, hIcon);

		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWndCrackDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CWndCrackDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_bSelectingWnd)
	{
		CPoint	pt;
		CRect	rcw;

		pt = point;
		ClientToScreen(&pt);
		GetWindowRect(&rcw);
		if (!rcw.PtInRect(pt))
		{
			HWND	hWndTemp;
			hWndTemp = ::WindowFromPoint(pt);
			if (hWndTemp != m_hWndMouseSelect)
			{
				HDC	hdc;

				if (m_hWndMouseSelect != NULL)
				{
					// if it is a window handle.
					// hdc = ::GetDC(m_hWndMouseSelect);
					hdc = ::GetDC(NULL);
					::SetROP2(hdc, R2_XORPEN);
					::SelectObject(hdc, m_hPenWhite);
					::SelectObject(hdc, ::GetStockObject(HOLLOW_BRUSH));
					::GetWindowRect(m_hWndMouseSelect, &rcw);
					rcw.left += 1, rcw.top += 1, rcw.right -= 2, rcw.bottom -= 2;
					// ::MyScreenToClient(m_hWndMouseSelect, rcw);
					::Rectangle(hdc, rcw.left, rcw.top, rcw.right, rcw.bottom);
					// ::ReleaseDC(m_hWndMouseSelect, hdc);
					::ReleaseDC(NULL, hdc);
				}

				m_hWndMouseSelect = hWndTemp;
				SetCapturedWnd(m_hWndMouseSelect);

				// hdc = ::GetDC(m_hWndMouseSelect);
				hdc = ::GetDC(NULL);
				::SetROP2(hdc, R2_XORPEN);
				::SelectObject(hdc, m_hPenWhite);
				::SelectObject(hdc, ::GetStockObject(HOLLOW_BRUSH));
				::GetWindowRect(m_hWndMouseSelect, &rcw);
				// ::MyScreenToClient(m_hWndMouseSelect, rcw);
				rcw.left += 1, rcw.top += 1, rcw.right -= 2, rcw.bottom -= 2;
				::Rectangle(hdc, rcw.left, rcw.top, rcw.right, rcw.bottom);
				// ::ReleaseDC(m_hWndMouseSelect, hdc);
				::ReleaseDC(NULL, hdc);
			}
		}

		{
			CPoint	pt;
			CRect	rcw;

			pt = point;
			ClientToScreen(&pt);

			HDC	hdc;
			COLORREF	clrMask;

			hdc = ::GetDC(NULL);
			clrMask = GetPixel(hdc, pt.x, pt.y);

			SetMaskColor(clrMask);

			::ReleaseDC(NULL, hdc);
		}
	}
	
	CDialog::OnMouseMove(nFlags, point);
}

void CWndCrackDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (m_rcSelectingBt.PtInRect(point))
	{
		SetCapture();
		SetCursor(m_hcursorSelecting);
		m_bSelectingWnd = TRUE;

		Invalidate();
	}
	
	CDialog::OnLButtonDown(nFlags, point);
}

void CWndCrackDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_bSelectingWnd)
	{
		ReleaseCapture();
		m_bSelectingWnd = FALSE;

		HDC	hdc;
		// hdc = ::GetDC(m_hWndMouseSelect);
		hdc = ::GetDC(NULL);
		::SetROP2(hdc, R2_XORPEN);
		::SelectObject(hdc, m_hPenWhite);
		::SelectObject(hdc, ::GetStockObject(HOLLOW_BRUSH));

		if (m_hWndMouseSelect != NULL)
		{
			// if it is a window handle.
			CRect	rcw;
			::GetWindowRect(m_hWndMouseSelect, &rcw);
			rcw.left += 1, rcw.top += 1, rcw.right -= 2, rcw.bottom -= 2;
			// ::MyScreenToClient(m_hWndMouseSelect, rcw);
			::Rectangle(hdc, rcw.left, rcw.top, rcw.right, rcw.bottom);
		}
		// ::ReleaseDC(m_hWndMouseSelect, hdc);
		::ReleaseDC(NULL, hdc);
		m_hWndMouseSelect = NULL;

		Invalidate();
	}
	
	CDialog::OnLButtonUp(nFlags, point);
}

void CWndCrackDlg::OnTopmost() 
{
	HWND	hWnd = GetCapturedWnd();
	if (hWnd == NULL)
		return;

	HWND	hWndParent = hWnd;

	while (hWndParent)
	{
		hWnd = hWndParent;
		hWndParent = ::GetParent(hWnd);
	}

	if (IsTopmostWindow(hWnd))
	{
		TopmostWindow(hWnd, FALSE);
	}
	else
	{
		TopmostWindow(hWnd, TRUE);
	}
}

void CWndCrackDlg::OnShowwindow() 
{
    HWND	hWnd = GetCapturedWnd();
    if (hWnd == NULL)
        return;

	if (::IsWindowVisible(hWnd))
	{
		::ShowWindow(hWnd, SW_HIDE);
	}
	else
	    ::ShowWindow(hWnd, SW_SHOW);
}

void CWndCrackDlg::OnMovewindow() 
{
	HWND		hWndParent;
    HWND		hWnd;

	hWnd = GetCapturedWnd();
    if (hWnd == NULL)
        return;

    CRect   rc;
	BOOL	bTrans = TRUE;
	BOOL	bError = FALSE;

	rc.left = GetDlgItemInt(IDC_HWND_X, &bTrans, TRUE);
	if (!bTrans)
		bError = TRUE;

	rc.top = GetDlgItemInt(IDC_HWND_Y, &bTrans, TRUE);
	if (!bTrans)
		bError = TRUE;

	rc.right = rc.left + GetDlgItemInt(IDC_HWND_CX, &bTrans, FALSE);
	if (!bTrans)
		bError = TRUE;

	rc.bottom = rc.top + GetDlgItemInt(IDC_HWND_CY, &bTrans, FALSE);
	if (!bTrans)
		bError = TRUE;

	if (bError)
	{
		AfxMessageBox("请输入正确的窗口坐标！");
		return;
	}

	hWndParent = ::GetParent(hWnd);
	if (hWndParent != NULL)
		MyScreenToClient(hWndParent, rc);

    ::MoveWindow(hWnd, rc.left, rc.top, rc.Width(), rc.Height(), TRUE);
}

void CWndCrackDlg::OnClosewnd() 
{
    HWND	hWnd = GetCapturedWnd();
    if (hWnd == NULL)
        return;

	if (AfxMessageBox("您确实要关闭该窗口，您关闭之后，就不能再恢复了！", MB_OKCANCEL | MB_ICONWARNING) == IDCANCEL)
		return;
    
    CRect   rc;

    ::PostMessage(hWnd, WM_CLOSE, 0, 0);
}

void CWndCrackDlg::SetCapturedWnd(HWND hWndCap)
{
	TCHAR		szBuffer[1024];

	sprintf(szBuffer, "%X", (DWORD)hWndCap);

	SetDlgItemText(IDC_HWND_CAPTURED, szBuffer);

	//
	// 取得窗口的属性
	//

	// 窗口标题
	::SendMessage(hWndCap, WM_GETTEXT, 1024, (LPARAM)szBuffer);
	SetDlgItemText(IDC_HWND_CAPTION, szBuffer);

	// 类名
	::GetClassName(hWndCap, szBuffer, 1024);
	SetDlgItemText(IDC_HWND_CLASSNAME, szBuffer);

	// Process Id
	DWORD		dwProcessId;
	GetWindowThreadProcessId(hWndCap, &dwProcessId);
	SetDlgItemInt(IDC_E_PROCESSID, dwProcessId, false);

	// Ctrl Id
	SetDlgItemInt(IDC_E_CTRLID, ::GetDlgCtrlID(hWndCap), false);

	// 坐标
	CRect		rc;
	::GetWindowRect(hWndCap, &rc);
	SetDlgItemInt(IDC_HWND_X, rc.left, TRUE);
	SetDlgItemInt(IDC_HWND_Y, rc.top, TRUE);
	SetDlgItemInt(IDC_HWND_CX, rc.Width(), FALSE);
	SetDlgItemInt(IDC_HWND_CY, rc.Height(), FALSE);
}

HWND CWndCrackDlg::GetCapturedWnd()
{
	TCHAR		szBuffer[256];
	HWND		hWndCap = NULL;

	if (GetDlgItemText(IDC_HWND_CAPTURED, szBuffer, 256) <= 0)
	{
		AfxMessageBox("请选择一个窗口！");
		return NULL;
	}

	if (sscanf(szBuffer, "%X", &hWndCap) < 1)
	{
		AfxMessageBox("请选择一个窗口！");
		return NULL;
	}

	if (!IsWindow(hWndCap))
	{
		AfxMessageBox("您选择的窗口可能已经关闭！");
		return NULL;
	}

	return hWndCap;
}

void CWndCrackDlg::OnUpdate() 
{
	HWND	hWnd = GetCapturedWnd();
	if (hWnd == NULL)
		return;

	SetCapturedWnd(hWnd);
}

void CWndCrackDlg::OnSetWndCaption() 
{
	TCHAR		szBuffer[256];

	HWND	hWnd = GetCapturedWnd();
	if (hWnd == NULL)
		return;

	GetDlgItemText(IDC_HWND_CAPTION, szBuffer, 256);
	::SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)szBuffer);
}

void CWndCrackDlg::OnMinimizeWnd() 
{
	HWND	hWnd = GetCapturedWnd();
	if (hWnd == NULL)
		return;

	if (::IsIconic(hWnd))
		::ShowWindow(hWnd, SW_RESTORE);
	else
		::ShowWindow(hWnd, SW_MINIMIZE);
}

void CWndCrackDlg::OnSetWndOpaque() 
{
	CSetLayedOptDlg		dlg;

	HWND	hWnd = GetCapturedWnd();
	if (hWnd == NULL)
		return;

	HWND	hWndParent = hWnd;

	while (hWndParent)
	{
		hWnd = hWndParent;
		hWndParent = ::GetParent(hWnd);
	}

	dlg.m_hWndModify = hWnd;
	if (dlg.m_hWndModify == NULL)
		return;

	dlg.DoModal();
}

void CWndCrackDlg::OnEnableWnd() 
{
	HWND	hWnd = GetCapturedWnd();
	if (hWnd == NULL)
		return;

	if (::IsWindowEnabled(hWnd))
		::EnableWindow(hWnd, FALSE);
	else
		::EnableWindow(hWnd, TRUE);
}

void EnumChildWnd(CTreeCtrl *pTreeCtrl, HTREEITEM hItemParent, HWND hWndParent)
{
	HWND	hWndChild;

	hWndChild = NULL;

	do
	{
		hWndChild = FindWindowEx(hWndParent, hWndChild, NULL, NULL);
		if (hWndChild)
		{
			CString		strName;
			TCHAR		szName[256];
			HTREEITEM	hItem;

			::GetWindowText(hWndChild, szName, 256);
			if (!IsEmptyString(szName))
				strName.Format("%X(%s)", (DWORD)hWndChild, szName);
			else
				strName.Format("%X", (DWORD)hWndChild);
			hItem = pTreeCtrl->InsertItem(strName, hItemParent);

			pTreeCtrl->SetItemData(hItem, (DWORD)hWndChild);

			EnumChildWnd(pTreeCtrl, hItem, hWndChild);
		}
	}
	while(hWndChild);
	
}

BOOL CALLBACK CWndCrackDlg::EnumWindowProc(
  HWND hwnd,      // handle to parent window
  LPARAM lParam   // application-defined value
)
{
	CWndCrackDlg	*pdlg;

	pdlg = (CWndCrackDlg *)lParam;
	if (!pdlg)
		return FALSE;

	DWORD		dwProcessID;

	GetWindowThreadProcessId(hwnd, &dwProcessID);
	if (dwProcessID == pdlg->m_dwProcessID)
	{
		CString		strName;
		TCHAR		szName[256];
		HTREEITEM	hItem;

		::GetWindowText(hwnd, szName, 256);
		if (!IsEmptyString(szName))
			strName.Format("%X(%s)", (DWORD)hwnd, szName);
		else
			strName.Format("%X", (DWORD)hwnd);
		hItem = pdlg->m_wndTree.InsertItem(strName);

		pdlg->m_wndTree.SetItemData(hItem, (DWORD)hwnd);

		EnumChildWnd(&(pdlg->m_wndTree), hItem, hwnd);
	}

	return TRUE;
}

void CWndCrackDlg::OnEnum() 
{
	HWND		hWndCapture;

	hWndCapture = GetCapturedWnd();
	if (!hWndCapture)
		return;

	GetWindowThreadProcessId(hWndCapture, &m_dwProcessID);

	m_wndTree.DeleteAllItems();

	EnumWindows(EnumWindowProc, (LPARAM)this);
}

void CWndCrackDlg::OnSelchangedTreeWnd(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	HTREEITEM		hItem = m_wndTree.GetSelectedItem();

	SetCapturedWnd((HWND)m_wndTree.GetItemData(hItem));
	
	*pResult = 0;
}

void CWndCrackDlg::SetMaskColor(COLORREF clrMask)
{
	TCHAR	szBuffer[256];
	TCHAR	szOld[256];

	sprintf(szBuffer, "%02X%02X%02X", clrMask & 0xFF, (clrMask >> 8) & 0xff, (clrMask >> 16) & 0xff);
	GetDlgItemText(IDC_MASK_COLOR, szOld, 256);
	if (stricmp(szBuffer, szOld) != 0)
	{
		SetDlgItemText(IDC_MASK_COLOR, szBuffer);

		sprintf(szBuffer, "RGB(%d, %d, %d)", clrMask & 0xFF, (clrMask >> 8) & 0xff, (clrMask >> 16) & 0xff);
		SetDlgItemText(IDC_MASK_COLOR2, szBuffer);
		if (m_hbrColor)
			DeleteObject(m_hbrColor);

		m_hbrColor = CreateSolidBrush(clrMask);

		::InvalidateRect(::GetDlgItem(m_hWnd, IDC_SEL_COLOR), NULL, TRUE);
	}
}

LRESULT CWndCrackDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if (message == WM_CTLCOLORBTN)
	{
		if ((HWND)lParam == ::GetDlgItem(m_hWnd, IDC_SEL_COLOR))
			return (LRESULT)m_hbrColor;
	}
	
	return CDialog::WindowProc(message, wParam, lParam);
}

void CWndCrackDlg::OnBnClickedMaximizeWnd()
{
	HWND	hWnd = GetCapturedWnd();
	if (hWnd == NULL)
		return;

	if (::IsZoomed(hWnd))
		::ShowWindow(hWnd, SW_RESTORE);
	else
		::ShowWindow(hWnd, SW_MAXIMIZE);
}
