// DlgImageProperty.cpp : implementation file
//

#include "stdafx.h"
#include "MPSkinEditor.h"
#include "DlgImageProperty.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgImageProperty dialog


CDlgImageProperty::CDlgImageProperty(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgImageProperty::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgImageProperty)
	m_ncx = 0;
	m_ncy = 0;
	m_nx = 0;
	m_ny = 0;
	m_nStretchStart = 0;
	m_nStretchStart2 = 0;
	m_nStretchEnd = 0;
	m_nStretchEnd2 = 0;
	//}}AFX_DATA_INIT
}


void CDlgImageProperty::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgImageProperty)
	DDX_Control(pDX, IDC_IMAGE, m_wndImageRectSel);
	DDX_Control(pDX, IDC_CB_IMAGE, m_cbImage);
	DDX_Text(pDX, IDC_E_CX, m_ncx);
	DDX_Text(pDX, IDC_E_CY, m_ncy);
	DDX_Text(pDX, IDC_E_X, m_nx);
	DDX_Text(pDX, IDC_E_Y, m_ny);
	DDX_Text(pDX, IDC_E_STRETCH_START, m_nStretchStart);
	DDX_Text(pDX, IDC_E_STRETCH_START2, m_nStretchStart2);
	DDX_Text(pDX, IDC_E_STRETCH_END, m_nStretchEnd);
	DDX_Text(pDX, IDC_E_STRETCH_END2, m_nStretchEnd2);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgImageProperty, CDialog)
	//{{AFX_MSG_MAP(CDlgImageProperty)
	ON_CBN_SELCHANGE(IDC_CB_IMAGE, OnSelchangeCbImage)
	ON_BN_CLICKED(IDC_UPDATE_RC, OnUpdateRc)
	ON_EN_CHANGE(IDC_E_CX, OnChangeECx)
	ON_EN_CHANGE(IDC_E_Y, OnChangeEY)
	ON_EN_CHANGE(IDC_E_X, OnChangeEX)
	ON_EN_CHANGE(IDC_E_CY, OnChangeECy)
	ON_BN_CLICKED(IDC_ROOM_IN, OnRoomIn)
	ON_BN_CLICKED(IDC_ROOM_OUT, OnRoomOut)
	ON_WM_DESTROY()
	ON_EN_CHANGE(IDC_E_STRETCH_START, OnChangeEStretchStart)
	ON_EN_CHANGE(IDC_E_STRETCH_END, OnChangeEStretchEnd)
	ON_EN_CHANGE(IDC_E_STRETCH_START2, OnChangeEStretchStart2)
	ON_EN_CHANGE(IDC_E_STRETCH_END2, OnChangeEStretchEnd2)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgImageProperty message handlers

BOOL CDlgImageProperty::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CString		str;
	GetWindowText(str);
	str += _T(": ");
	str += m_strName.c_str();
	SetWindowText(str);
	
	vector<tstring>		vFiles;

	vFiles.push_back(_T(""));
	g_skinFactory.GetResourceMgr()->EnumFiles(_T("*.bmp"), vFiles, false);
	g_skinFactory.GetResourceMgr()->EnumFiles(_T("*.png"), vFiles, false);

	for (int i = 0; i < (int)vFiles.size(); i++)
	{
		m_cbImage.AddString(vFiles[i].c_str());
	}
	m_cbImage.SelectString(-1, m_strFile.c_str());

	m_wndImageRectSel.SetImageFile(m_strFile.c_str());
	str.Format(_T("Width: %d, Height: %d"), m_wndImageRectSel.GetImageWidth(), m_wndImageRectSel.GetImageHeight());
	SetDlgItemText(IDC_S_IMG_SIZE, str);
	SetDlgItemText(IDC_S_CURSOR, _T(""));

	if (m_vNameRects.size() >= 2)
	{
		// rect image
		CRect		rcObj;
		long		x = 0, y = 0, cx = 0, cy = 0;

		GetRectValue(m_vNameRects[1].c_str(), x, y, cx, cy);
		m_nx = x;
		m_ny = y;
		m_ncx = cx;
		m_ncy = cy;
		rcObj.SetRect(m_nx, m_ny, m_nx + m_ncx, m_ny + m_ncy);

		::EnableDlgItem(m_hWnd, IDC_E_X, true);
		::EnableDlgItem(m_hWnd, IDC_E_Y, true);
		::EnableDlgItem(m_hWnd, IDC_E_CX, true);
		::EnableDlgItem(m_hWnd, IDC_E_CY, true);
		if (m_vNameRects.size() == 2)
		{
			m_wndImageRectSel.InitAsRectImg(rcObj);
		}
		else if (m_vNameRects.size() >= 6)
		{
			::EnableDlgItem(m_hWnd, IDC_E_STRETCH_START, true);
			::EnableDlgItem(m_hWnd, IDC_E_STRETCH_END, true);

			m_nStretchStart = _ttoi(m_vNameRects[3].c_str());
			m_nStretchEnd = _ttoi(m_vNameRects[5].c_str());

			if (m_vNameRects.size() == 6)
			{
				// horz or vert stretched image
				if (_tcsicmp(m_vNameRects[2].c_str(), _T("StretchStartX")) == 0)
				{
					// horz 
					m_wndImageRectSel.InitAsHorzStretchImg(rcObj, m_nStretchStart, m_nStretchEnd);
				}
				else
				{
					// vert
					m_wndImageRectSel.InitAsVertStretchImg(rcObj, m_nStretchStart, m_nStretchEnd);
				}
			}
			else if (m_vNameRects.size() == 10)
			{
				// caption image
				::EnableDlgItem(m_hWnd, IDC_E_STRETCH_START2, true);
				::EnableDlgItem(m_hWnd, IDC_E_STRETCH_END2, true);

				m_nStretchStart2 = _ttoi(m_vNameRects[7].c_str());
				m_nStretchEnd2 = _ttoi(m_vNameRects[9].c_str());
				m_wndImageRectSel.InitAsCaptionImg(rcObj, m_nStretchStart, m_nStretchEnd, m_nStretchStart2, m_nStretchEnd2);
			}
			else
				ASSERT(0);
		}

		UpdateData(FALSE);
	}

	m_wndHelper.Init(m_hWnd);
	m_wndHelper.Add(IDC_IMAGE, true, true, false, false);
	m_wndHelper.AddAlignBottom(IDC_ROOM_OUT, true);
	m_wndHelper.AddAlignBottom(IDC_ROOM_IN, true);
	m_wndHelper.AddAlignBottom(IDC_S_CURSOR, true);
	m_wndHelper.AddAlignBottom(IDC_S_IMG_SIZE, true);
	m_wndHelper.AddAlignBottom(IDC_S_FILE, true);
	m_wndHelper.AddAlignBottom(IDC_CB_IMAGE, true);
	m_wndHelper.AddAlignBottom(IDC_S_X, true);
	m_wndHelper.AddAlignBottom(IDC_E_X, true);
	m_wndHelper.AddAlignBottom(IDC_S_CX, true);
	m_wndHelper.AddAlignBottom(IDC_E_CX, true);
	m_wndHelper.AddAlignBottom(IDC_S_Y, true);
	m_wndHelper.AddAlignBottom(IDC_E_Y, true);
	m_wndHelper.AddAlignBottom(IDC_S_CY, true);
	m_wndHelper.AddAlignBottom(IDC_E_CY, true);
	m_wndHelper.AddAlignBottom(IDC_S_SSX, true);
	m_wndHelper.AddAlignBottom(IDC_E_STRETCH_START, true);
	m_wndHelper.AddAlignBottom(IDC_S_SEX, true);
	m_wndHelper.AddAlignBottom(IDC_E_STRETCH_END, true);
	m_wndHelper.AddAlignBottom(IDC_S_SSX2, true);
	m_wndHelper.AddAlignBottom(IDC_E_STRETCH_START2, true);
	m_wndHelper.AddAlignBottom(IDC_S_SEX2, true);
	m_wndHelper.AddAlignBottom(IDC_E_STRETCH_END2, true);
	m_wndHelper.AddAlignBottom(IDOK, true);
	m_wndHelper.AddAlignBottom(IDCANCEL, true);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgImageProperty::OnSelchangeCbImage() 
{
	int		nSel = m_cbImage.GetCurSel();
	CString	str;

	m_cbImage.GetLBText(nSel, str);
	m_strFile = str;
	m_wndImageRectSel.SetImageFile(m_strFile.c_str());

	str.Format(_T("Width: %d, Height: %d"), m_wndImageRectSel.GetImageWidth(), m_wndImageRectSel.GetImageHeight());
	SetDlgItemText(IDC_S_IMG_SIZE, str);
}

void CDlgImageProperty::OnUpdateRc() 
{
	CRect		rcObj;

	m_wndImageRectSel.GetRect(rcObj);
	m_nx = rcObj.left;
	m_ny = rcObj.top;
	m_ncx = rcObj.Width();
	m_ncy = rcObj.Height();

	m_nStretchStart = m_wndImageRectSel.GetStretchStart();
	m_nStretchEnd = m_wndImageRectSel.GetStretchEnd();
	m_nStretchStart2 = m_wndImageRectSel.GetStretchStart2();
	m_nStretchEnd2 = m_wndImageRectSel.GetStretchEnd2();

	UpdateData(FALSE);
}

void CDlgImageProperty::OnChangeEY() 
{
	UpdateData();

	m_wndImageRectSel.SetRectY(m_ny);
	m_wndImageRectSel.Invalidate();
}

void CDlgImageProperty::OnChangeEX() 
{
	UpdateData();

	m_wndImageRectSel.SetRectX(m_nx);
	m_wndImageRectSel.Invalidate();
}

void CDlgImageProperty::OnChangeECx() 
{
	UpdateData();

	m_wndImageRectSel.SetRectCx(m_ncx);
	m_wndImageRectSel.Invalidate();
}

void CDlgImageProperty::OnChangeECy() 
{
	UpdateData();

	m_wndImageRectSel.SetRectCy(m_ncy);
	m_wndImageRectSel.Invalidate();
}

void CDlgImageProperty::OnChangeEStretchStart() 
{
	UpdateData();

	m_wndImageRectSel.SetStretchStart(m_nStretchStart);
	m_wndImageRectSel.Invalidate();
}

void CDlgImageProperty::OnChangeEStretchEnd() 
{
	UpdateData();

	m_wndImageRectSel.SetStretchStart(m_nStretchEnd);
	m_wndImageRectSel.Invalidate();
}

void CDlgImageProperty::OnChangeEStretchStart2() 
{
	UpdateData();

	m_wndImageRectSel.SetStretchStart(m_nStretchStart2);
	m_wndImageRectSel.Invalidate();
}

void CDlgImageProperty::OnChangeEStretchEnd2() 
{
	UpdateData();

	m_wndImageRectSel.SetStretchStart(m_nStretchEnd2);
	m_wndImageRectSel.Invalidate();
}

void CDlgImageProperty::OnRoomIn() 
{
	if (m_wndImageRectSel.GetScale() < 4)
	{
		m_wndImageRectSel.SetScale(m_wndImageRectSel.GetScale() + 1);
		m_wndImageRectSel.Invalidate();
	}
}

void CDlgImageProperty::OnRoomOut() 
{
	if (m_wndImageRectSel.GetScale() > 1)
	{
		m_wndImageRectSel.SetScale(m_wndImageRectSel.GetScale() - 1);
		m_wndImageRectSel.Invalidate();
	}
}

void CDlgImageProperty::OnDestroy() 
{
	CDialog::OnDestroy();
}

void CDlgImageProperty::OnOK() 
{
	TCHAR		szValue[256];

	if (m_vNameRects.size() >= 2)
	{
		// rect image
		_stprintf(szValue, _T("%d,%d,%d,%d"), m_nx, m_ny, m_ncx, m_ncy);
		m_vNameRects[1] = szValue;

		if (m_vNameRects.size() >= 6)
		{
			_stprintf(szValue, _T("%d"), m_nStretchStart);
			m_vNameRects[3] = szValue;

			_stprintf(szValue, _T("%d"), m_nStretchEnd);
			m_vNameRects[5] = szValue;

			if (m_vNameRects.size() == 10)
			{
				// caption image
				_stprintf(szValue, _T("%d"), m_nStretchStart2);
				m_vNameRects[7] = szValue;

				_stprintf(szValue, _T("%d"), m_nStretchEnd2);
				m_vNameRects[9] = szValue;
			}
			else
				ASSERT(0);
		}
	}

	CDialog::OnOK();
}

void CDlgImageProperty::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	m_wndHelper.OnResize();
}
