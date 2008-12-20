// ComboBox.cpp: implementation of the CWidgetComboBox class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BaseWnd.h"
#include "WidgetComboBox.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//CWidgetComboBox::CWidgetComboBox()
//{
//
//}

CWidgetComboBox::~CWidgetComboBox()
{

}
// 	virtual BOOL Create(DWORD dwStyle, int x, int y, int nWidth, int nHeight, CBaseWnd *pWndParent);

void CWidgetComboBox::GetSelItemText(tstring &str)
{
	int		n = GetCurSel();
	TCHAR	szText[512];

	szText[0] = 0;
	GetLBText(n, szText);
	str = szText;
}
