/********************************************************************
	Created  :	2001Äê12ÔÂ15ÈÕ 0:42:06
	FileName :	SkinButton.cpp
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#include "StdAfx.h"
#include "MLSkin.h"
#include "SkinButton.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CSkinButton, _T("Button"))

CSkinButton::CSkinButton()
{
	m_vBtStatImg.push_back(new BtStatImg);
	m_bRadioBt = false;
}

CSkinButton::~CSkinButton()
{
}

BOOL CSkinButton::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CSkinNStatusButton::SetProperty(szProperty, szValue))
		return TRUE;

	if (_tcsicmp(szProperty, _T("CheckBox")) == 0)
	{
		if (IsTRUE(szValue))
		{
			SetMaxStat(2);
			m_nMsgWant &= ~UO_MSG_WANT_ENTER_KEY;
		}
		else
			SetMaxStat(1);
	}
	else if (_tcsicmp(szProperty, _T("Radio")) == 0)
	{
		m_bRadioBt = IsTRUE(szValue);
		if (m_bRadioBt)
		{
			SetMaxStat(2);
			m_nMsgWant &= ~UO_MSG_WANT_ENTER_KEY;
		}
		else
			SetMaxStat(1);
	}
	else if (_tcsicmp(szProperty, _T("Checked")) == 0)
		m_nCurStatus = IsTRUE(szValue) ? 1 : 0;
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

#ifdef _SKIN_EDITOR_
void CSkinButton::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObject::EnumProperties(listProperties);

	if (m_vBtStatImg.empty())
		return;

	listProperties.AddPropBoolStr(_T("CheckBox"), m_vBtStatImg.size() == 2, m_vBtStatImg.size() == 2);
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
#endif // _SKIN_EDITOR_

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

void CSkinButton::ButtonUpAction()
{
	if (m_bRadioBt)
	{
		if (m_nCurStatus >= (int)m_vBtStatImg.size() - 1)
		{
			m_bLBtDown = FALSE;
			Invalidate();
			return;
		}
	}

	CSkinNStatusButton::ButtonUpAction();
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
				btimg->imgHover.MaskBlt(pGraph, m_rcObj.left, m_rcObj.top, &m_imgMask, m_bpm);
			else
				btimg->imgHover.Blt(pGraph, m_rcObj.left, m_rcObj.top, m_bpm);
			return;
		}
	}

	CSkinNStatusButton::Draw(pGraph);
}

// class CSkinActiveButton
///////////////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CSkinImageButton, _T("ImageButton"))

CSkinImageButton::CSkinImageButton()
{
	m_bContentImage = true;
	m_bAutoSelColor = false;
	m_nContentMarginX = 5, m_nContentMarginY = 5;
}

CSkinImageButton::~CSkinImageButton()
{

}

void CSkinImageButton::Draw(CRawGraph *pGraph)
{
	CSkinButton::Draw(pGraph);

	CRect	rc = m_rcObj;
	rc.DeflateRect(m_nContentMarginX, m_nContentMarginY);
	if (m_bContentImage)
	{
		// DrawImage
		if (m_contentImage.IsValid())
			m_contentImage.StretchBlt(pGraph, rc.left, rc.top, rc.Width(), rc.Height());
	}
	else
	{
		// Fill with color
		pGraph->FillRect(&rc, m_clrContent);
	}
}

BOOL CSkinImageButton::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CSkinButton::SetProperty(szProperty, szValue))
		return TRUE;

	if (IsPropertyName(szProperty, _T("ContentMarginX")))
		m_nContentMarginX = _ttoi(szValue);
	else if (IsPropertyName(szProperty, _T("ContentMarginY")))
		m_nContentMarginY = _ttoi(szValue);
	else
		return FALSE;

	return TRUE;
}

void CSkinImageButton::SetContentImage(PRAW_IMAGE_DATA pRawImage)
{
	m_contentImage.Attach(pRawImage);
	m_bContentImage = true;
}

void CSkinImageButton::SetColor(const MLColor &clr, bool bAutoSelColor)
{
	m_clrContent = clr;
	m_bContentImage = false;
	m_bAutoSelColor = bAutoSelColor;
}

void CSkinImageButton::ButtonUpAction()
{
	if (!m_bContentImage && m_bAutoSelColor)
	{
		CDlgChooseColor		dlg;

		if (dlg.DoModal(m_pSkin, m_clrContent) == IDOK)
			m_clrContent = dlg.GetColor();
		else
		{
			m_bLBtDown = FALSE;
			Invalidate();
			return;
		}
	}

	return CSkinButton::ButtonUpAction();
}

///////////////////////////////////////////////////////////////////////////////
UIOBJECT_CLASS_NAME_IMP(CSkinHoverActionButton, _T("HoverActionButton"))

CSkinHoverActionButton::CSkinHoverActionButton()
{
	m_nTimerIdHoverAction = 0;
	m_bHoverActionBegin = false;
	m_dwBeginHoverTime = 0;
}

CSkinHoverActionButton::~CSkinHoverActionButton()
{
}

bool CSkinHoverActionButton::OnMouseMove(UINT nFlags, CPoint point)
{
	bool	bHoverPrev = m_bHover;

	bool bRet = CSkinButton::OnMouseMove(nFlags, point);

	if (bHoverPrev != m_bHover)
	{
		if (m_bHover)
		{
			m_nTimerIdHoverAction = m_pSkin->RegisterTimerObject(this, 500);
			m_bHoverActionBegin = true;
		}
		else
			EndHover();
	}

	return bRet;
}

void CSkinHoverActionButton::OnTimer(int nId)
{
	if (nId == m_nTimerIdHoverAction)
	{
		if (m_bHoverActionBegin)
		{
			m_bHoverActionBegin = false;
			m_pSkin->UnregisterTimerObject(this, m_nTimerIdHoverAction);
			m_nTimerIdHoverAction = m_pSkin->RegisterTimerObject(this, 100);
			m_dwBeginHoverTime = GetTickCount();
			DispatchEvent(CSkinHoverActionBtEventNotify::C_BEGIN_HOVER);
		}
		else
		{
			DWORD	dwTimeNew = GetTickCount();
			const int MAX_HOVER_ACTION_TIME	= 20 * 1000;
			if (dwTimeNew - m_dwBeginHoverTime >= MAX_HOVER_ACTION_TIME)
				EndHover();
			else
				DispatchEvent(CSkinHoverActionBtEventNotify::C_HOVER_ACTION);
		}

		return;
	}

	CSkinButton::OnTimer(nId);
}

void CSkinHoverActionButton::DispatchEvent(CSkinHoverActionBtEventNotify::Command cmd)
{
	CSkinHoverActionBtEventNotify	event(this);

	event.cmd = cmd;
	m_pSkin->DispatchUIObjNotify(&event);
}

void CSkinHoverActionButton::EndHover()
{
	m_pSkin->UnregisterTimerObject(this, m_nTimerIdHoverAction);
	m_nTimerIdHoverAction = 0;
	DispatchEvent(CSkinHoverActionBtEventNotify::C_END_HOVER);
}

void CSkinHoverActionButton::ButtonUpAction()
{
	if (m_nTimerIdHoverAction != 0)
		EndHover();

	DispatchEvent(CSkinHoverActionBtEventNotify::C_CLICK);

	return CSkinButton::ButtonUpAction();
}
