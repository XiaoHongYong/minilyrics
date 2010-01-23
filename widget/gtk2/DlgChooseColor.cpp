#include "stdafx.h"
#include "BaseWnd.h"
#include "DlgChooseColor.h"

CDlgChooseColor::CDlgChooseColor()
{
	m_clr = RGB(255, 255, 255);
}

CDlgChooseColor::~CDlgChooseColor()
{

}

int CDlgChooseColor::DoModal(CBaseWnd *pWndParent, const MLColor &clr)
{

	return IDCANCEL;
}
