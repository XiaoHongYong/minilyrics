// SetLayedOptDlg.cpp : implementation file
//

#include "stdafx.h"
#include "wndcrack.h"
#include "SetLayedOptDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*******************************************************************/
// 下面的几行是Win2000中的透明窗口定义用的
#define WS_EX_LAYERED           0x00080000

#define LWA_COLORKEY            0x00000001
#define LWA_ALPHA               0x00000002


#define ULW_COLORKEY            0x00000001
#define ULW_ALPHA               0x00000002
#define ULW_OPAQUE              0x00000004
/*******************************************************************/
/*

// COMMENT:
//		判断系统是否支持半透明窗口
//		只有Windows2000或以上才支持
BOOL IsLayeredWndSupported()
{
	OSVERSIONINFO		version;

	memset(&version, 0, sizeof(version));
	version.dwOSVersionInfoSize = sizeof(version);

	if (!GetVersionEx(&version))
	{
		return FALSE;
	}

	// Major version: 5, windows 2000 系列
	if (version.dwMajorVersion < 5)
		return FALSE;

	return TRUE;
}*/

/*
// 定义SetLayeredWindowAttributes函数
typedef BOOL (WINAPI *FNSetLayeredWindowAttributes)(HWND, COLORREF, BYTE, DWORD);


//	COMMENT:
//		sets the opacity and transparency color key of a layered window
//	INPUT:
//		crKey,		specifies the color key
//		bAlpha,		value for the blend function, When bAlpha is 0, 
//					the window is completely transparent. 
//					When bAlpha is 255, the window is opaque. 
//		dwFlags		Specifies an action to take. This parameter can be one or 
//					more of the following values. Value Meaning 
//					LWA_COLORKEY Use crKey as the transparency color.  
//					LWA_ALPHA Use bAlpha to determine the opacity of the layered window 
BOOL SetLayeredWindow(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags)
{
	FNSetLayeredWindowAttributes		fnSetLayeredWindowAttributes;
	HMODULE				hModule;
	DWORD				dwStyle;

	// 首先设置WS_EX_LAYERED属性
	dwStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
	dwStyle |= WS_EX_LAYERED;
	SetWindowLong(hWnd, GWL_EXSTYLE, dwStyle);

	// 调用SetLayeredWindowAttributes设置透明色
	hModule = GetModuleHandle("user32.dll");
	fnSetLayeredWindowAttributes = (FNSetLayeredWindowAttributes)GetProcAddress(hModule, "SetLayeredWindowAttributes");
	if (fnSetLayeredWindowAttributes)
	{
		if (!(*fnSetLayeredWindowAttributes)(hWnd, crKey, bAlpha, dwFlags))
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}


void UnSetLayeredWindow(HWND hWnd)
{
	DWORD				dwStyle;

	// 首先设置WS_EX_LAYERED属性
	dwStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
	dwStyle &= ~WS_EX_LAYERED;
	SetWindowLong(hWnd, GWL_EXSTYLE, dwStyle);
}

BOOL SetLayeredWindowAttributes(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags)
{
	FNSetLayeredWindowAttributes		fnSetLayeredWindowAttributes;
	HMODULE				hModule;

	// 调用SetLayeredWindowAttributes设置透明色
	hModule = GetModuleHandle("user32.dll");
	fnSetLayeredWindowAttributes = (FNSetLayeredWindowAttributes)GetProcAddress(hModule, "SetLayeredWindowAttributes");
	if (fnSetLayeredWindowAttributes)
	{
		if (!(*fnSetLayeredWindowAttributes)(hWnd, crKey, bAlpha, dwFlags))
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}
*/

/////////////////////////////////////////////////////////////////////////////
// CSetLayedOptDlg dialog


CSetLayedOptDlg::CSetLayedOptDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSetLayedOptDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSetLayedOptDlg)
	m_bUseLayeredOpt = TRUE;
	m_strMaskColor = _T("");
	m_byOpaque = 200;
	m_bTransColor = FALSE;
	m_bOpaque = FALSE;
	m_bClickThrough = FALSE;
	//}}AFX_DATA_INIT
	m_bUseLayeredOpt = g_Profile.GetInt("m_bUseLayeredOpt", TRUE);
	m_bTransColor = g_Profile.GetInt("m_bTransColor", TRUE);
	m_bOpaque = g_Profile.GetInt("m_bOpaque", TRUE);
	m_byOpaque = g_Profile.GetInt("m_byOpaque", 200);

	m_strMaskColor = g_Profile.GetString("m_strMaskColor", "0");
}


void CSetLayedOptDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetLayedOptDlg)
	DDX_Check(pDX, IDC_ENABLE_LAYERED, m_bUseLayeredOpt);
	DDX_Text(pDX, IDC_MASK_COLOR, m_strMaskColor);
	DDX_Text(pDX, IDC_OPAQUE, m_byOpaque);
	DDV_MinMaxByte(pDX, m_byOpaque, 100, 255);
	DDX_Check(pDX, IDC_C_TRANS_COLOR, m_bTransColor);
	DDX_Check(pDX, IDC_R_OPAQUE, m_bOpaque);
	DDX_Check(pDX, IDC_C_CLICKTHROUGH, m_bClickThrough);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSetLayedOptDlg, CDialog)
	//{{AFX_MSG_MAP(CSetLayedOptDlg)
	ON_BN_CLICKED(IDC_LAYERED_OPAQUE, OnLayeredOpaque)
	ON_BN_CLICKED(IDC_ENABLE_LAYERED, OnEnableLayered)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_SEL_COLOR, OnSelColor)
	ON_BN_CLICKED(IDC_RADIO1, OnLayeredOpaque)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetLayedOptDlg message handlers

void CSetLayedOptDlg::OnOK() 
{
	UpdateData();

	COLORREF	clrKey = 0;
	DWORD		dwFlag = 0;

	if (m_bOpaque)
	{
		// 使用透明度
		dwFlag |= LWA_ALPHA;
	}

	if (m_bTransColor)
	{
		// 使用透明色
		if (sscanf(m_strMaskColor, "%X", &clrKey) < 1)
		{
			AfxMessageBox("请输入透明色彩！", MB_ICONWARNING);
			return;
		}
		dwFlag |= LWA_COLORKEY;
	}

	if (m_bUseLayeredOpt)
	{
		if (m_bClickThrough)
			SetWindowLong(m_hWndModify, GWL_EXSTYLE, GetWindowLong(m_hWndModify, GWL_EXSTYLE) | WS_EX_TRANSPARENT);
		else
			SetWindowLong(m_hWndModify, GWL_EXSTYLE, GetWindowLong(m_hWndModify, GWL_EXSTYLE) & ~WS_EX_TRANSPARENT);

		SetLayeredWindow(m_hWndModify, clrKey, m_byOpaque, dwFlag);
	}
	else
	{
		SetWindowLong(m_hWndModify, GWL_EXSTYLE, GetWindowLong(m_hWndModify, GWL_EXSTYLE) & ~WS_EX_TRANSPARENT);

		UnSetLayeredWindow(m_hWndModify);
		// ::InvalidateRect(m_hWndModify, NULL, TRUE);
		// ::UpdateWindow(m_hWndModify);
		::ShowWindow(m_hWndModify, SW_HIDE);
		::ShowWindow(m_hWndModify, SW_SHOW);
	}
}

void CSetLayedOptDlg::OnLayeredOpaque() 
{
	UpdateData();
//
//	if (m_nLayerOpaque)
//	{
//		EnableDlgItem(m_hWnd, IDC_MASK_COLOR, FALSE);
//		EnableDlgItem(m_hWnd, IDC_OPAQUE, TRUE);
//	}
//	else
//	{
//		EnableDlgItem(m_hWnd, IDC_MASK_COLOR, TRUE);
//		EnableDlgItem(m_hWnd, IDC_OPAQUE, FALSE);
//	}
}

#define EnableDlgItem(hWnd, nid, bEnalbe)		::EnableWindow(::GetDlgItem(hWnd, nid), bEnalbe)

void CSetLayedOptDlg::OnEnableLayered() 
{
	UpdateData();

	if (m_bUseLayeredOpt)
	{
		EnableDlgItem(m_hWnd, IDC_C_TRANS_COLOR, TRUE);
		EnableDlgItem(m_hWnd, IDC_R_OPAQUE, TRUE);
		EnableDlgItem(m_hWnd, IDC_LAYERED_OPAQUE, TRUE);
		EnableDlgItem(m_hWnd, IDC_OPAQUE, TRUE);
	}
	else
	{
		EnableDlgItem(m_hWnd, IDC_C_TRANS_COLOR, FALSE);
		EnableDlgItem(m_hWnd, IDC_R_OPAQUE, FALSE);
		EnableDlgItem(m_hWnd, IDC_LAYERED_OPAQUE, FALSE);
		EnableDlgItem(m_hWnd, IDC_OPAQUE, FALSE);
	}

	OnLayeredOpaque();
}

void CSetLayedOptDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (m_rcSelectingBt.PtInRect(point))
	{
		SetCapture();
		SetCursor(m_hcursorSelecting);
		m_bSelectingColor = TRUE;

		Invalidate();
	}

	CDialog::OnLButtonDown(nFlags, point);
}

void CSetLayedOptDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_bSelectingColor)
	{
		ReleaseCapture();
		m_bSelectingColor = FALSE;

		Invalidate();

//		HDC	hdc;
//		hdc = ::GetDC(NULL);
//		::ReleaseDC(NULL, hdc);
	}
	
	CDialog::OnLButtonUp(nFlags, point);
}

void CSetLayedOptDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_bSelectingColor)
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
	
	CDialog::OnMouseMove(nFlags, point);
}

BOOL CSetLayedOptDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	GetDlgItem(IDC_SELWND)->GetWindowRect(&m_rcSelectingBt);
	GetDlgItem(IDC_SELWND)->ShowWindow(SW_HIDE);
	ScreenToClient(&m_rcSelectingBt);

	m_hcursorSelecting = LoadCursor(AfxGetResourceHandle(), MAKEINTRESOURCE(IDC_CURSORSELWND));
	m_bSelectingColor = FALSE;

	COLORREF	clrKey;

	// 使用透明色
	if (sscanf(m_strMaskColor, "%X", &clrKey) < 1)
	{
		clrKey = 0;
	}
	m_hbrColor = CreateSolidBrush(clrKey);

	OnEnableLayered();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSetLayedOptDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	if (!IsIconic())
	{
		HICON	hIcon;

		if (m_bSelectingColor)
			hIcon = LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_DRAGAFTER));
		else
			hIcon = LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_DRAGBEFORE));

		dc.DrawIcon(m_rcSelectingBt.left, m_rcSelectingBt.top, hIcon);

		CDialog::OnPaint();
	}
}

void CSetLayedOptDlg::SetMaskColor(COLORREF clrMask)
{
	TCHAR	szBuffer[256];
	TCHAR	szOld[256];

	sprintf(szBuffer, "%02X%02X%02X", clrMask & 0xFF, (clrMask >> 8) & 0xff, (clrMask >> 16) & 0xff);

	GetDlgItemText(IDC_MASK_COLOR, szOld, 256);
	if (stricmp(szBuffer, szOld) != 0)
	{
		SetDlgItemText(IDC_MASK_COLOR, szBuffer);
		if (m_hbrColor)
			DeleteObject(m_hbrColor);

		m_hbrColor = CreateSolidBrush(clrMask);

		::InvalidateRect(::GetDlgItem(m_hWnd, IDC_SEL_COLOR), NULL, TRUE);
	}
}

void CSetLayedOptDlg::OnSelColor() 
{
	
}

LRESULT CSetLayedOptDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if (message == WM_CTLCOLORBTN)
	{
		if ((HWND)lParam == ::GetDlgItem(m_hWnd, IDC_SEL_COLOR))
			return (LRESULT)m_hbrColor;
	}
	
	return CDialog::WindowProc(message, wParam, lParam);
}

void CSetLayedOptDlg::OnDestroy() 
{
	CDialog::OnDestroy();

	if (m_hbrColor)
		DeleteObject(m_hbrColor);

	g_Profile.WriteInt("m_bUseLayeredOpt", m_bUseLayeredOpt);
	g_Profile.WriteInt("m_bTransColor", m_bTransColor);
	g_Profile.WriteInt("m_bOpaque", m_bOpaque);
	g_Profile.WriteInt("m_byOpaque", m_byOpaque);

	g_Profile.WriteString("m_strMaskColor", m_strMaskColor);
}
