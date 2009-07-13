// WidgetImageButton.cpp: implementation of the CWidgetImageButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BaseWnd.h"
#include "WidgetImageButton.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWidgetImageButton::CWidgetImageButton()
{
	m_pImage = NULL;
}

CWidgetImageButton::~CWidgetImageButton()
{
	if (m_pImage)
		delete m_pImage;
}

BOOL CWidgetImageButton::Attach(CBaseWnd *pWndParent, int nCtrlID)
{
	CWidgetCtrlBase::Attach(pWndParent, nCtrlID);

	return TRUE;
}

void CWidgetImageButton::SetImage(CMLImage *pImage)
{
	if (m_pImage)
	{
		delete m_pImage;
		m_pImage = NULL;
	}

	if (!pImage)
		return;
}
