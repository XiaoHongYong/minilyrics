#include "stdafx.h"
#include "SkinActivateCtrl.h"

UIOBJECT_CLASS_NAME_IMP(CSkinActivateCtrl, _T("ActivateCtrl"));

CSkinActivateCtrl::CSkinActivateCtrl()
{
}

CSkinActivateCtrl::~CSkinActivateCtrl()
{
}

BOOL CSkinActivateCtrl::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CUIObject::SetProperty(szProperty, szValue))
		return true;

	if (IsPropertyName(szProperty, _T("StartPercent")))
	{
	}
	else
		return false;

	return true;
}

void CSkinActivateCtrl::Draw(CRawGraph *pGraph)
{
	// pGraph->VertAlphaFadeOut()
	pGraph->FillRect(&m_rcObj, MLColor(RGB(0, 0, 255)));
}
