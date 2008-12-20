// SEPropertyListCtrl.cpp: implementation of the CSEPropertyListCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPSkinEditor.h"
#include "SEPropertyListCtrl.h"
#include "DlgImageProperty.h"
#include "DlgChooseIDs.h"

#define IDC_P_EDIT			10
#define IDC_P_BUTTON		3
#define IDC_P_COMB_BT		4
#define IDC_P_COMB_LIST		5

CSESkinPropertyAdapter		g_seSkinPropertyAdapter;

/////////////////////////////////////////////////////////////////////////////
// CComboButton
CComboButton::CComboButton()
{
}

CComboButton::~CComboButton()
{
	// Delete the objects created
	delete m_pBkBrush;
	delete m_pBlackBrush;
	delete m_pGrayPen;
	delete m_pBkPen;
}

BEGIN_MESSAGE_MAP(CComboButton, CButton)
	//{{AFX_MSG_MAP(CComboButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CComboButton message handlers
BOOL CComboButton::Create( CRect Rect, CWnd* pParent, UINT uID)
{
	// Create the Brushes and Pens
	m_pBkBrush		= new CBrush( GetSysColor(COLOR_BTNFACE));
	m_pBkPen		= new CPen( PS_SOLID, 1, GetSysColor(COLOR_BTNFACE));
	m_pGrayPen		= new CPen( PS_SOLID, 1, RGB(128,128,128));
	m_pBlackBrush	= new CBrush(RGB(0,0,0)); 

    // Create the CButton
	if( !CButton::Create( _T(""), WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|BS_OWNERDRAW, Rect, pParent, uID ))
		return FALSE;
		
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Draw the Button
void CComboButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	CDC*	pDC			= CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect 	ButtonRect  = lpDrawItemStruct->rcItem;

	// Fill the Background
	CBrush* pOldBrush = (CBrush*)pDC->SelectObject( m_pBkBrush );
	CPen* pOldPen = (CPen*)pDC->SelectObject(m_pBkPen);
	pDC->Rectangle(ButtonRect);

	// Draw the Correct Border
	if(lpDrawItemStruct->itemState & ODS_SELECTED)
	{
		pDC->DrawEdge(ButtonRect, EDGE_SUNKEN, BF_RECT);
		ButtonRect.left++;
		ButtonRect.right++;
		ButtonRect.bottom++;
		ButtonRect.top++;
	}
	else
		pDC->DrawEdge(ButtonRect, EDGE_RAISED, BF_RECT);

	// Draw the Triangle
	ButtonRect.left		+= 3;
	ButtonRect.right	-= 4;
	ButtonRect.top		+= 5;
	ButtonRect.bottom	-= 5;
	DrawTriangle(pDC, ButtonRect);

	// Return what was used
	pDC->SelectObject( pOldPen );
	pDC->SelectObject( pOldBrush );
}

void CComboButton::DrawTriangle(CDC* pDC, CRect Rect)
{
	POINT 	ptArray[3];	

	// Figure out the Top left
	ptArray[0].x = Rect.left;
	ptArray[0].y = Rect.top;
	ptArray[1].x = Rect.right;
	ptArray[1].y = Rect.top;
	ptArray[2].x = Rect.right - (Rect.Width() / 2);
	ptArray[2].y = Rect.bottom;

	// Select the Brush and Draw the triangle
	pDC->SelectObject(m_pBlackBrush);
	pDC->Polygon(ptArray, 3 );
}

void CComboButton::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSEPropertyListCtrl::CSEPropertyListCtrl()
{
	m_nWidestItem			= 0;
	m_bDeleteFont			= TRUE;
	m_bBoldSelection		= FALSE;

	m_pBkBrush				= NULL;
	m_pBkPropertyBrush		= NULL;
	m_pEditWnd				= NULL;
	m_pButton				= NULL;
	m_pComboButton			= NULL;
	m_pListBox				= NULL;
	m_pBkHighlightBrush		= NULL;
	m_pSelectedFont			= NULL;
	m_pBorderPen			= NULL;
	m_pCurItem				= NULL;
	m_pCurFont				= NULL;
	m_pCurDrawItem			= NULL;
	m_pTextFont				= NULL;
	m_pSelectedFont			= NULL;
	m_pBorderPen			= NULL;

	m_crBorderColor			= RGB(192,192,192);
	m_crBkColor				= GetSysColor(COLOR_WINDOW);
	m_crPropertyBkColor		= m_crBkColor;
	m_crTextColor			= GetSysColor(COLOR_WINDOWTEXT);
	m_crPropertyTextColor	= m_crTextColor;
	m_crHighlightColor		= GetSysColor(COLOR_HIGHLIGHT);
	m_crTextHighlightColor	= GetSysColor(COLOR_HIGHLIGHTTEXT);
}

CSEPropertyListCtrl::~CSEPropertyListCtrl()
{
	if(m_bDeleteFont)		delete m_pTextFont;

	if(m_pEditWnd)			delete m_pEditWnd;
	if(m_pButton)			delete m_pButton;
	if(m_pListBox)			delete m_pListBox;
	if(m_pComboButton)		delete m_pComboButton;

	if(m_pBkBrush)			delete m_pBkBrush;
	if(m_pBkPropertyBrush)	delete m_pBkPropertyBrush;
	if(m_pBkHighlightBrush) delete m_pBkHighlightBrush;
	if(m_pSelectedFont)		delete m_pSelectedFont;
	if(m_pBorderPen)		delete m_pBorderPen;
}

BEGIN_MESSAGE_MAP(CSEPropertyListCtrl, CListBox)
	//{{AFX_MSG_MAP(CSEPropertyListCtrl)
	ON_WM_CREATE()
	ON_WM_CTLCOLOR_REFLECT()
	ON_CONTROL_REFLECT(LBN_SELCHANGE, OnSelchange)
	ON_WM_CTLCOLOR()
	ON_CONTROL_REFLECT(LBN_DBLCLK, OnDblclk)
	ON_EN_KILLFOCUS( IDC_P_EDIT, OnEditLostFocus )
	ON_EN_CHANGE( IDC_P_EDIT, OnEditChange )
	ON_BN_CLICKED( IDC_P_BUTTON, OnButtonClick )
	ON_BN_CLICKED( IDC_P_COMB_BT, OnComboBoxClick )
	ON_LBN_SELCHANGE(IDC_P_COMB_LIST, OnSelChange)
	ON_LBN_KILLFOCUS(IDC_P_COMB_LIST, OnListboxLostFocus)
	ON_WM_LBUTTONDOWN()
	ON_WM_VSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSEPropertyListCtrl message handlers
int CSEPropertyListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CListBox::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Create the default font and set it
	m_pTextFont = new CFont();
	m_pTextFont->CreateFont( 14, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, 
							 CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_SWISS,
							 _T("ARIAL"));	

	// Create the Heading font and set it
	m_pSelectedFont = new CFont();
	m_pSelectedFont->CreateFont( 14, 0, 0, 0, FW_BOLD, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, 
							 CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_SWISS,
							 _T("ARIAL"));	

	// Create the Border Pen
	m_pBorderPen = new CPen(PS_SOLID, 1, m_crBorderColor);

	// Create the Selected Background brush
	m_pBkHighlightBrush = new CBrush(m_crHighlightColor);
	m_pBkBrush			= new CBrush(m_crBkColor);

	// Set the row height
	SetItemHeight(-1,16);	
	return 0;
}

HBRUSH CSEPropertyListCtrl::CtlColor(CDC* pDC, UINT nCtlColor) 
{	
	return (HBRUSH)m_pBkBrush->GetSafeHandle();
}

HBRUSH CSEPropertyListCtrl::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CListBox::OnCtlColor(pDC, pWnd, nCtlColor);

	if( nCtlColor == CTLCOLOR_EDIT) 
	{
		pDC->SetBkColor(m_crPropertyBkColor);
		pDC->SetTextColor(m_crPropertyTextColor);
	}

	if(m_pBkPropertyBrush)
		return (HBRUSH)(m_pBkPropertyBrush->GetSafeHandle() );
	else
		return hbr;
}

void CSEPropertyListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// Make sure its a valid item
	if( lpDrawItemStruct->itemID == LB_ERR )
		return;
     
  	// Obtain the text for this item
	m_csText.Empty();
	GetText(lpDrawItemStruct->itemID, m_csText);

	// Get the drawing DC
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	
	// Set the Current member we are drawing
	if (lpDrawItemStruct->itemID < m_vProperies.size())
		m_pCurDrawItem = &m_vProperies[lpDrawItemStruct->itemID];
	else
		m_pCurDrawItem = NULL;

	// Obtain the Item Rect
	CRect ItemRect(lpDrawItemStruct->rcItem);
	
    // Draw This item
	DrawItem( pDC, ItemRect, lpDrawItemStruct->itemState & ODS_SELECTED);
}

void CSEPropertyListCtrl::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
}

void CSEPropertyListCtrl::OnDblclk() 
{
	POINT	pPoint;
	GetCursorPos(&pPoint);

	// Convert this rect to coordinates of the desktop	
	CRect TempRect = m_CurRect;
	MapWindowPoints(GetDesktopWindow(), TempRect);

	// Display the Correct Control
	switch(m_pCurItem->prop.valueType)
	{
	case CUIObjProperty::VT_FONT_NAME:
	case CUIObjProperty::VT_IMAGE:
	case CUIObjProperty::VT_IMG_FILE:
	case CUIObjProperty::VT_ID:
		m_pButton->SetFocus();		
		OnButtonClick();
		break;
	case CUIObjProperty::VT_COLOR:
		{
			LOGBRUSH lb;
			m_pCurItem->pBrush->GetLogBrush(&lb);
			CColorDialog ColorDialog(lb.lbColor, 0, GetParent());
			if(ColorDialog.DoModal() != IDOK)
				return;
			
			// Destroy the Brush and create a new one
			if(m_pCurItem->pBrush) delete m_pCurItem->pBrush;
			m_pCurItem->pBrush = new CBrush(ColorDialog.GetColor());

			TCHAR	szStr[64];
			COLORREF	clr = ColorDialog.GetColor();
			_stprintf(szStr, _T("%02X%02X%02X"), clr & 0xFF, (clr >> 8) & 0xff, (clr >> 16) & 0xff);

			m_pCurItem->prop.strValue = szStr;

			// Redraw the Widow (Theres probably a better way)
			RedrawWindow();
			
			// Send the message that a property has changed
			OnPropertyChanged(GetCurSel());
		}
		break;
	}
}

void CSEPropertyListCtrl::OnSelchange() 
{		
	HideControls();

	if (!m_pCurItem)
		return;

	// Display the Correct Control
	CRect TempRect = m_CurRect;
	TempRect.InflateRect(-1,-1);

	if (m_pCurItem->IsEdit())
	{
		TempRect.left += 1;
		m_pEditWnd->SetWindowText(m_pCurItem->prop.strValue.c_str());
		m_pEditWnd->MoveWindow(TempRect);
		m_pEditWnd->ShowWindow(SW_SHOWNORMAL);
		m_pEditWnd->SetFocus();
		//				m_pEditWnd->SetSel(0,-1);
	}
	else
	{
		switch (m_pCurItem->prop.valueType)
		{
		case CUIObjProperty::VT_FONT_NAME:
		case CUIObjProperty::VT_IMAGE:
		case CUIObjProperty::VT_IMG_FILE:
		case CUIObjProperty::VT_ID:
			TempRect.left = TempRect.right - 17;
			m_pButton->MoveWindow(TempRect);
			m_pButton->ShowWindow(SW_SHOWNORMAL);
			break;
		case CUIObjProperty::VT_COMB_STR:
		case CUIObjProperty::VT_BOOL_STR:
		case CUIObjProperty::VT_FILE:
			TempRect.left = TempRect.right - 17;
			m_pComboButton->MoveWindow(TempRect);
			m_pComboButton->ShowWindow(SW_SHOWNORMAL);
			
			TempRect.left = m_CurRect.left + 2;
			TempRect.right -= 17;
			if(m_pCurItem->bComboEditable)
			{
				m_pEditWnd->SetWindowText(m_pCurItem->prop.strValue.c_str());
				//					m_pEditWnd->SetFocus();
				//					m_pEditWnd->SetSel(0,-1);
				m_pEditWnd->MoveWindow(TempRect);
				m_pEditWnd->ShowWindow(SW_SHOWNORMAL);
				m_pEditWnd->SetFocus();
			}
			
			// Move the Lsit box
			TempRect.left--;
			TempRect.right += 18;
			TempRect.top = TempRect.bottom;
			
			// Set the Bottom Height
			if(m_pCurItem->prop.vOptions.size() > 5)
				TempRect.bottom += GetItemHeight(0) * 5;
			else
				TempRect.bottom += GetItemHeight(0) * m_pCurItem->prop.vOptions.size();
			m_pListBox->MoveWindow(TempRect);

			// Force the Expansion
			OnComboBoxClick();
			break;
		}

	}
}

void CSEPropertyListCtrl::OnEditLostFocus()
{
	m_pEditWnd->ShowWindow(SW_HIDE);
	if (!m_pCurItem)
	{
		// Send the message that a property has changed
		return;
	}

	// Get the text
	CString csText;
	m_pEditWnd->GetWindowText(csText);

	// Is the current item a text item
	if (m_pCurItem->IsEdit())
	{
		// Did the text change
		if(!m_bChanged)
		{
			return;
		}

		m_pCurItem->prop.strValue = csText;

		// Send the message that a property has changed
		OnPropertyChanged(GetCurSel());
	}
	else
	{
/*
		// Get the window that has the focus now
		if (GetFocus() == m_pComboButton || !m_pListBox->GetCount())
			return;

		// Did the text change
		if(!m_bChanged)
			return;

		m_pCurItem->prop.strValue = csText;

		// Send the message that a property has changed
		OnPropertyChanged(GetCurSel());

		// Look for this text
		m_bChanged = FALSE;
		if( m_pListBox->FindStringExact(-1,csText) != LB_ERR)
			return;

		// Add it and select it
		m_pCurItem->nOptSelected = m_pCurItem->prop.vOptions.push_back(csText);

		// Resort the strings is necessary
		if(m_pCurItem->bComboSorted)
		{
			// m_pCurItem->prop.vOptions.sort();
	
			// Search the the string and set its positon the selected one
			// m_pCurItem->nOptSelected = m_pCurItem->csProperties.FindString(csText);
		}*/

	}
}

void CSEPropertyListCtrl::OnEditChange()
{
	m_bChanged = TRUE;
}

void CSEPropertyListCtrl::OnButtonClick()
{
	if (!m_pCurItem)
		return;

	switch (m_pCurItem->prop.valueType)
	{
	case CUIObjProperty::VT_ID:
		OnChooseIDClick();
		break;
	case CUIObjProperty::VT_IMG_FILE:
	case CUIObjProperty::VT_IMAGE:
		OnImagePropertyClick();
		break;
	case CUIObjProperty::VT_FONT_NAME:
		OnFontPropertyClick();
		break;
	default:
		ASSERT(0);
		break;
	}
}

void CSEPropertyListCtrl::OnChooseIDClick()
{
	CDlgChooseIDs	dlg;

	dlg.m_strID = m_pCurItem->prop.strValue;
	if (dlg.DoModal() != IDOK)
		return;

	m_pCurItem->prop.strValue = dlg.m_strID;

	// Redraw
	RedrawWindow();

	// Send the message that a property has changed
	OnPropertyChanged(GetCurSel());
}

void CSEPropertyListCtrl::OnImagePropertyClick()
{
	CDlgImageProperty	dlg;

	dlg.m_strFile = m_pCurItem->prop.strValue.c_str();
	dlg.m_vNameRects = m_pCurItem->prop.vOptions;

	if(dlg.DoModal() != IDOK)
		return;

	m_pCurItem->prop.vOptions = dlg.m_vNameRects;
	m_pCurItem->prop.strValue = dlg.m_strFile;

	// Redraw
	RedrawWindow();

	// Send the message that a property has changed
	OnPropertyChanged(GetCurSel());
}

void CSEPropertyListCtrl::OnFontPropertyClick()
{
	// Show the Dialog
	CFontDialog FontDialog(&m_pCurItem->LogFont);
	if(FontDialog.DoModal() != IDOK)
		return;
	
	// Set the Font data 
	FontDialog.GetCurrentFont(&m_pCurItem->LogFont);
	m_pCurItem->prop.strValue = m_pCurItem->LogFont.lfFaceName;

	// Redraw
	RedrawWindow();

	// Send the message that a property has changed
	OnPropertyChanged(GetCurSel());
}

void CSEPropertyListCtrl::OnComboBoxClick()
{
	// Add the items
	m_pListBox->ResetContent();

	// Loop for all items
	for( int nItem = 0; nItem < m_pCurItem->prop.vOptions.size(); nItem++)
		m_pListBox->AddString(m_pCurItem->prop.vOptions[nItem].c_str());

	// Select the correct item
	int		nIndex;
	nIndex = m_pListBox->SelectString(-1, m_pCurItem->prop.strValue.c_str());
	m_pListBox->SetTopIndex(nIndex);
	
	// Show the List box
	m_pListBox->ShowWindow(SW_NORMAL);	
}

void CSEPropertyListCtrl::OnSelChange()
{
	CString		str;
	// Set the new current item
	m_pCurItem->nOptSelected = m_pListBox->GetCurSel();
	m_pListBox->GetText(m_pCurItem->nOptSelected, str);
	m_pCurItem->prop.strValue = str;

	// Hide the Windows
	m_pListBox->ShowWindow(SW_HIDE);

	if(m_pCurItem->bComboEditable)
		m_pEditWnd->SetWindowText(m_pCurItem->prop.strValue.c_str());
	else
		RedrawWindow();

	// Send the message that a property has changed
	OnPropertyChanged(GetCurSel());
	m_pComboButton->SetFocus();
}

void CSEPropertyListCtrl::OnListboxLostFocus()
{
	CString		str;
	m_pListBox->GetText(m_pCurItem->nOptSelected, str);
	m_pCurItem->prop.strValue = str;

	m_pListBox->ShowWindow(SW_HIDE);
}

void CSEPropertyListCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// is there an item at this point
	BOOL bOutside;
	UINT uItem = ItemFromPoint(point, bOutside);

	// Is this outside the client
	if(bOutside)
		HideControls();
	
	CListBox::OnLButtonDown(nFlags, point);
}

void CSEPropertyListCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// Hide the Controls
	HideControls();

	CListBox::OnVScroll(nSBCode, nPos, pScrollBar);
}

/////////////////////////////////////////////////////////////////////////////
// GUI User Functions
/////////////////////////////////////////////////////////////////////////////
void CSEPropertyListCtrl::SetFont(CFont* pFont)
{
	// Delete our font and set our font to theirs
	if(m_pTextFont)		delete m_pTextFont;
	if(m_pSelectedFont) delete m_pSelectedFont;
	m_pTextFont = pFont;
	m_bDeleteFont = FALSE;
	
	// Figure out the text size
	LOGFONT lpLogFont;
	m_pTextFont->GetLogFont(&lpLogFont);

	// Set the font and redraw
	CWnd::SetFont(m_pTextFont, FALSE);

	// Create the heading font with the bold attribute
	lpLogFont.lfWeight = FW_BOLD;
	m_pSelectedFont = new CFont();
	m_pSelectedFont->CreateFontIndirect(&lpLogFont);

	// Set the Row height
	SetItemHeight(-1,lpLogFont.lfHeight + 2);

	// ** IMPLEMENT LATER ?? **
	// Recalculate the Width Position

}

void CSEPropertyListCtrl::SetLineStyle(COLORREF crColor, int nStyle)
{
	// Delete the old Pen
	if(m_pBorderPen) delete m_pBorderPen;
	
	// Create the brush
	m_pBorderPen = new CPen(nStyle, 1, crColor);
	m_crBorderColor = crColor;
}

void CSEPropertyListCtrl::SetBkColor(COLORREF crColor)
{
	// Delete the old brush
	if(m_pBkBrush) delete m_pBkBrush;
	
	// Create the brush
	m_pBkBrush = new CBrush(crColor);
	m_crBkColor = crColor;
}

void CSEPropertyListCtrl::SetPropertyBkColor(COLORREF crColor)
{
	// Delete the old brush
	if(m_pBkPropertyBrush) delete m_pBkPropertyBrush;
	
	// Create the brush
	m_pBkPropertyBrush = new CBrush(crColor);
	m_crPropertyBkColor = crColor;
}

void CSEPropertyListCtrl::SetHighlightColor(COLORREF crColor)
{
	// Delete the old brush
	if(m_pBkHighlightBrush) delete m_pBkHighlightBrush;
	
	// Create the brush
	m_pBkHighlightBrush = new CBrush(crColor);
	m_crHighlightColor = crColor;
}

BOOL CSEPropertyListCtrl::AddProperty(CUIObjProperty &prop, int nAlignment)
{
	SEPLItem		item;

	item.prop = prop;
	item.nAlignment = nAlignment;
	item.nWidth = GetTextExtent(prop.strName.c_str());		
	if (prop.valueType == CUIObjProperty::VT_COLOR)
		item.pBrush = new CBrush(GetColorValue(prop.strValue.c_str(), RGB(0, 0, 0)));
	else if (prop.valueType == CUIObjProperty::VT_BOOL_STR)
	{
		item.prop.vOptions.push_back(SZ_TRUE);
		item.prop.vOptions.push_back(SZ_FALSE);
	}
	else if (prop.valueType == CUIObjProperty::VT_FILE)
	{
		item.prop.vOptions.clear();
		item.prop.vOptions.push_back(_T(""));
		for (int i = 0; i < prop.vOptions.size(); i++)
		{
			g_skinFactory.GetResourceMgr()->EnumFiles(prop.vOptions[i].c_str(), 
				item.prop.vOptions, false);
		}
	}
	else if (prop.valueType == CUIObjProperty::VT_FONT_NAME)
	{
		_tcscpy_safe(item.LogFont.lfFaceName, CountOf(item.LogFont.lfFaceName), prop.strValue.c_str());
	}

	m_vProperies.push_back(item);

	// Add the string to the list box
	int nPos = CListBox::AddString(prop.strName.c_str());

	// Create the Control if Needed
	CreateControl(prop.valueType);

	return TRUE;
}

BOOL CSEPropertyListCtrl::AddProperties(CUIObjProperties &properties, int nAlignment)
{
	CUIObjProperties::iterator	it, itEnd;

	itEnd = properties.end();
	for (it = properties.begin(); it != itEnd; ++it)
		AddProperty(*it, nAlignment);

	return TRUE;
}

BOOL CSEPropertyListCtrl::SetProperty(LPCTSTR szProp, LPCTSTR szValue)
{
	V_SEPLITEM::iterator	it, itEnd;

	itEnd = m_vProperies.end();
	for (it = m_vProperies.begin(); it != itEnd; ++it)
	{
		SEPLItem	&item = *it;
		if (_tcsicmp(szProp, item.prop.strName.c_str()) == 0)
		{
			item.prop.strValue = szValue;
			ASSERT(item.prop.valueType != CUIObjProperty::VT_IMAGE);
			Invalidate();
			return TRUE;
		}
	}

	return FALSE;
}

CUIObjProperty *CSEPropertyListCtrl::GetProperty(int nIndex)
{
	if (nIndex >= 0 && nIndex < m_vProperies.size())
	{
		return &(m_vProperies[nIndex].prop);
	}

	return NULL;
}

void CSEPropertyListCtrl::GetProperties(vector<tstring> &vProperties)
{
	V_SEPLITEM::iterator	it, itEnd;

	itEnd = m_vProperies.end();
	for (it = m_vProperies.begin(); it != itEnd; ++it)
	{
		SEPLItem	&item = *it;
		item.prop.ToUIObjProperties(vProperties);
	}
}

void CSEPropertyListCtrl::ResetContent()
{
	CListBox::ResetContent();
	HideControls();
	m_vProperies.clear();
	m_pCurItem = NULL;
	m_pCurDrawItem = NULL;
}

void CSEPropertyListCtrl::DrawItem(CDC* pDC, CRect ItemRect, BOOL bSelected)
{
	/////////////////////////////////////////
	// Paint the Background rectangle (Property Value)
	if (m_pCurDrawItem->prop.valueType == CUIObjProperty::VT_COLOR)
		pDC->SelectObject(m_pCurDrawItem->pBrush);
	else
		pDC->SelectObject(m_pBkBrush);
	pDC->SelectObject(m_pBorderPen);

	// Draw the Rectangle
	ItemRect.left = m_nWidestItem - 1;
	ItemRect.top--;
	ItemRect.right++;
	pDC->Rectangle(ItemRect);
	CRect OrginalRect = ItemRect;
	
	/////////////////////////////////////////
	// Draw the Property Text
	pDC->SetBkMode(TRANSPARENT);
	pDC->SelectObject(m_pBkBrush);
	pDC->SelectObject(m_pTextFont);
	pDC->SetTextColor(m_crTextColor);
	DrawPropertyText(pDC, ItemRect);

	/////////////////////////////////////////
	// Paint the Background rectangle (Property Name)
	if( bSelected )
		pDC->SelectObject(m_pBkHighlightBrush);

	// Draw the Rectangle
	ItemRect.right = m_nWidestItem;
	ItemRect.left = -1;
	pDC->Rectangle(ItemRect);
	
	/////////////////////////////////////////
	// Paint the Property name Text
	// Is this item selected?
	if( bSelected )
	{		
		if(m_bBoldSelection) pDC->SelectObject(m_pSelectedFont);
		pDC->SetTextColor(m_crTextHighlightColor);
		m_pCurItem = m_pCurDrawItem;
		m_CurRect = OrginalRect;
	}
	
	// Draw the Text
	ItemRect.left += 6;
	ItemRect.right -= 5;
	pDC->DrawText( m_csText, m_csText.GetLength(), ItemRect, DT_SINGLELINE|DT_VCENTER|m_pCurDrawItem->nAlignment);
}

void CSEPropertyListCtrl::DrawPropertyText(CDC* pDC, CRect ItemRect)
{
	ItemRect.left += 5;
	switch(m_pCurDrawItem->prop.valueType)
	{
	case CUIObjProperty::VT_INT:
	case CUIObjProperty::VT_BOOL_STR:
	case CUIObjProperty::VT_STR:
	case CUIObjProperty::VT_FILE:
	case CUIObjProperty::VT_VAR_INT:
	case CUIObjProperty::VT_RECT:
	case CUIObjProperty::VT_COMB_STR:
	case CUIObjProperty::VT_FONT_NAME:
	case CUIObjProperty::VT_IMAGE:
	case CUIObjProperty::VT_IMG_FILE:
	case CUIObjProperty::VT_ID:
		pDC->DrawText(m_pCurDrawItem->prop.strValue.c_str(), ItemRect, DT_SINGLELINE | DT_VCENTER | DT_LEFT);
		break;
	case CUIObjProperty::VT_COLOR:
		break;
	default:
		ASSERT(0);
		break;
	}
}

void CSEPropertyListCtrl::CreateControl(CUIObjProperty::VALUE_TYPE valueType)
{
	switch (valueType)
	{
	case CUIObjProperty::VT_INT:
	case CUIObjProperty::VT_STR:
	case CUIObjProperty::VT_VAR_INT:
	case CUIObjProperty::VT_RECT:
		// Edit Window
		if(!m_pEditWnd)
		{
			m_pEditWnd = new CEdit();
			m_pEditWnd->Create(WS_CHILD | ES_AUTOHSCROLL | ES_LEFT, CRect(0,0,100,100), this, IDC_P_EDIT);
			m_pEditWnd->SetFont(m_pTextFont);
		}
		break;
	case CUIObjProperty::VT_FONT_NAME:
	case CUIObjProperty::VT_IMAGE:
	case CUIObjProperty::VT_IMG_FILE:
	case CUIObjProperty::VT_ID:
		if(!m_pButton)
		{
			m_pButton = new CButton();
			m_pButton->Create(_T("..."), WS_CHILD|BS_PUSHBUTTON, CRect(0,0,100,100), this, IDC_P_BUTTON);
			m_pButton->SetFont(m_pTextFont);
		}
		break;
	case CUIObjProperty::VT_BOOL_STR:
	case CUIObjProperty::VT_COMB_STR:
	case CUIObjProperty::VT_FILE:
		if(!m_pEditWnd)
		{
			m_pEditWnd = new CEdit();
			m_pEditWnd->Create(WS_CHILD|ES_AUTOHSCROLL|ES_LEFT, CRect(0,0,100,100), this, IDC_P_EDIT);
			m_pEditWnd->SetFont(m_pTextFont);
		}
		if(!m_pListBox)
		{
			m_pListBox = new CListBox();
			m_pListBox->Create(WS_CHILD|WS_BORDER|LBS_NOTIFY|WS_VSCROLL|LBS_HASSTRINGS, CRect(0,0,100,100), this, IDC_P_COMB_LIST);
			m_pListBox->SetFont(m_pTextFont);
			
			m_pComboButton = new CComboButton();
			m_pComboButton->Create(CRect(0,0,0,0), this, IDC_P_COMB_BT ); 
		}
		break;
	case CUIObjProperty::VT_COLOR:
		break;
	default:
		ASSERT(0);
		break;
	}
}

void CSEPropertyListCtrl::HideControls()
{
	// Hide the controls
	if(m_pEditWnd)		m_pEditWnd->ShowWindow(SW_HIDE);
	if(m_pButton)		m_pButton->ShowWindow(SW_HIDE);
	if(m_pListBox)		m_pListBox->ShowWindow(SW_HIDE);
	if(m_pComboButton)	m_pComboButton->ShowWindow(SW_HIDE);	
}

// Calculate the Width of the string based on the font set
int CSEPropertyListCtrl::GetTextExtent(LPCTSTR szText)
{
	CDC* pDC = GetDC();
	if (!pDC)
		return 10;

	pDC->SelectObject(m_pSelectedFont);
	CSize Size = pDC->GetTextExtent(szText);
	if(Size.cx + 10 > m_nWidestItem)
		m_nWidestItem = Size.cx + 10;

	ReleaseDC(pDC);

	return Size.cx;
}

void CSEPropertyListCtrl::OnPropertyChanged(int nIndex)
{
	g_seSkinPropertyAdapter.UpdatePropertyToSkin();
}

