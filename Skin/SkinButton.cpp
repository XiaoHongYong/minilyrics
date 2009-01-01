/********************************************************************
	Created  :	2001Äê12ÔÂ15ÈÕ 0:42:06
	FileName :	SkinButton.cpp
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#include "stdafx.h"
#include "MLSkin.h"
#include "SkinButton.h"

#define SZ_IMAGE_CHECK		_T("ImageCheck")

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CSkinButton, _T("Button"))

CSkinButton::CSkinButton()
{
	m_vBtStatImg.push_back(new BtStatImg);
}

CSkinButton::~CSkinButton()
{
}

BOOL CSkinButton::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CSkinNStatusButton::SetProperty(szProperty, szValue))
		return TRUE;

	if (_tcsicmp(szProperty, _T("CanCheck")) == 0)
	{
		if (IsTRUE(szValue))
			SetMaxStat(2);
		else
			SetMaxStat(1);
	}
	else if (_tcsicmp(szProperty, _T("Checked")) == 0)
		m_nCurStatus = IsTRUE(szValue) ? 1 : 0;
	else if (_tcsnicmp(szProperty, SZ_IMAGE_CHECK, _tcslen(SZ_IMAGE_CHECK)) == 0)
	{
		tstring		strNewProperty = _T("S1_image");
		strNewProperty += szProperty + _tcslen(SZ_IMAGE_CHECK);
		return CSkinNStatusButton::SetProperty(strNewProperty.c_str(), szValue);
	}
	else if (_tcsnicmp(szProperty, SZ_PN_IMAGE, _tcslen(SZ_PN_IMAGE)) == 0)
	{
		tstring		strNewProperty = _T("S0_");
		strNewProperty += szProperty;
		return CSkinNStatusButton::SetProperty(strNewProperty.c_str(), szValue);
	}
	else
		return FALSE;

	return TRUE;
}

void CSkinButton::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObject::EnumProperties(listProperties);

	if (m_vBtStatImg.empty())
		return;

	listProperties.AddPropBoolStr(_T("CanCheck"), m_vBtStatImg.size() == 2, m_vBtStatImg.size() == 2);
	listProperties.AddPropBoolStr(_T("Checked"), m_nCurStatus == 1, m_vBtStatImg.size() == 2);

	{
		BtStatImg		*statImg = m_vBtStatImg[0];

		listProperties.AddPropImage(SZ_PN_IMAGE, SZ_PN_IMAGERECT, statImg->strBkFile.c_str(), statImg->imgBk);
		listProperties.AddPropImage(_T("ImageSel"), _T("ImageSelRect"), statImg->strSelFile.c_str(), statImg->imgSel, !statImg->strSelFile.empty());
		listProperties.AddPropImage(_T("ImageFocus"), _T("ImageFocusRect"), statImg->strHoverFile.c_str(), statImg->imgHover, !statImg->strHoverFile.empty());
	}

	if (m_vBtStatImg.size() > 1)
	{
		BtStatImg		*statImg = m_vBtStatImg[1];

		listProperties.AddPropImage(_T("ImageCheck"), _T("ImageCheckRect"), statImg->strBkFile.c_str(), statImg->imgBk);
		listProperties.AddPropImage(_T("ImageCheckSel"), _T("ImageCheckSelRect"), statImg->strSelFile.c_str(), statImg->imgSel, !statImg->strSelFile.empty());
		listProperties.AddPropImage(_T("ImageCheckFocus"), _T("ImageCheckFocusRect"), statImg->strHoverFile.c_str(), statImg->imgHover, !statImg->strHoverFile.empty());
		listProperties.AddPropImage(_T("ImageDisable"), _T("ImageDisableRect"), statImg->strDisabledFile.c_str(), statImg->imgDisabled, !statImg->strDisabledFile.empty());
	}
}

void CSkinButton::SetCheck(bool bCheck)
{
	ASSERT(m_vBtStatImg.size() == 2);

	int		nStatusNew = m_nCurStatus;
	if (m_vBtStatImg.size() == 2)
	{
		if (bCheck)
			nStatusNew = 1;
		else
			nStatusNew = 0;
	}
	if (nStatusNew != m_nCurStatus)
	{
		m_nCurStatus = nStatusNew;
		Invalidate();
	}
}

bool CSkinButton::IsCheck()
{
	ASSERT(m_vBtStatImg.size() == 2);

	return m_nCurStatus == 1;
}

///////////////////////////////////////////////////////////////////////////////
// class CSkinActiveButton
// set classname
UIOBJECT_CLASS_NAME_IMP(CSkinActiveButton, _T("ActiveButton"))

CSkinActiveButton::CSkinActiveButton()
{
}

CSkinActiveButton::~CSkinActiveButton()
{

}

void CSkinActiveButton::Draw(CRawGraph *pGraph)
{
	if (m_vBtStatImg.empty())
		return;

	if (m_nCurStatus < 0 || m_nCurStatus >= (int)m_vBtStatImg.size())
		m_nCurStatus = 0;

	BtStatImg		*btimg = m_vBtStatImg[m_nCurStatus];

	if (m_bEnable && !m_bLBtDown && !m_bHover)
	{
		if (m_pSkin->IsWndActive())
		{
			if (m_imgMask.IsValid())
				btimg->imgHover.Blt(pGraph, m_rcObj.left, m_rcObj.top, &m_imgMask, m_bpm);
			else
				btimg->imgHover.Blt(pGraph, m_rcObj.left, m_rcObj.top, m_bpm);
			return;
		}
	}

	CSkinNStatusButton::Draw(pGraph);
}

// class CSkinActiveButton
///////////////////////////////////////////////////////////////////////////////
