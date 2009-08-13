// SkinRateCtrl.cpp: implementation of the CSkinRateCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SkinRateCtrl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// set classname
LPCTSTR CSkinRateCtrl::ms_szClassName = _T("RateButton");

CSkinRateCtrl::CSkinRateCtrl()
{
	m_nMsgWant = UO_MSG_WANT_MOUSEMOVE | UO_MSG_WANT_LBUTTON;

	m_bLBtDown = FALSE;
	m_nRating = 3;
	m_nRatingMax = 5;
	m_nRateStarWidth = 0;
}

CSkinRateCtrl::~CSkinRateCtrl()
{

}

bool CSkinRateCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	assert(m_pSkin);

	m_bLBtDown = TRUE;

//	DBGLOG1(_T("Rating: %d"), m_nRating);

	// 捕捉鼠标输入
//	m_pSkin->SetCaptureMouse(this);

//	Invalidate();

	return true;
}

bool CSkinRateCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bLBtDown = FALSE;

	m_nRating++;
	if (m_nRating > m_nRatingMax)
		m_nRating = 0;

	// 释放鼠标输入
//	m_pSkin->ReleaseCaptureMouse(this);

	Invalidate();

	m_pSkin->OnCustomCommand(m_nId);

	return true;
}

bool CSkinRateCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	return false;
/*	assert(m_pSkin);

	if (IsPtIn(point))
	{
		if (!m_bLBtDown && (nFlags & MK_LBUTTON))
		{
			m_bLBtDown = TRUE;

			if (m_bHover)
				m_bHover = false;
			else
			{
				// 捕捉鼠标输入
				m_pSkin->SetCaptureMouse(this);
			}

			Invalidate();
		}
		else if (!m_bLBtDown && m_bEnableHover)
		{
			m_bHover = true;

			// 捕捉鼠标输入
			m_pSkin->SetCaptureMouse(this);

			Invalidate();
		}
	}
	else
	{
		if (m_bLBtDown)
		{
			m_bLBtDown = FALSE;

			// 释放鼠标输入
			m_pSkin->ReleaseCaptureMouse(this);

			Invalidate();
		}
		else if (m_bHover)
		{
			m_bHover = false;

			// 释放鼠标输入
			m_pSkin->ReleaseCaptureMouse(this);

			Invalidate();
		}
	}

	return true;*/
}

LPCTSTR CSkinRateCtrl::GetClassName()
{
	return ms_szClassName;
}

void CSkinRateCtrl::Draw(CRawGraph *pGraph)
{
	if (m_nRating < 0 || m_nRating > m_nRatingMax)
		m_nRating = 0;

	int		i;
	int		x = m_rcObj.left;

	for (i = 1; i <= m_nRating; i++)
	{
		m_img.Blt(pGraph,
			x, m_rcObj.top, m_nRateStarWidth, m_img.m_cy, 0, 0);
		x += m_nRateStarWidth;
	}

	for (i = m_nRating + 1; i <= m_nRatingMax; i++)
	{
		m_img.Blt(pGraph,
			x, m_rcObj.top, m_nRateStarWidth, m_img.m_cy, m_nRateStarWidth, 0);
		x += m_nRateStarWidth;
	}
}

BOOL CSkinRateCtrl::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CUIObject::SetProperty(szProperty, szValue))
		return TRUE;

	if (_tcsicmp(szProperty, SZ_PN_IMAGE) == 0)
	{
		m_strBmpFile = szValue;
		m_img.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
	}
	else if (_tcsicmp(szProperty, SZ_PN_IMAGERECT) == 0)
	{
		if (!GetRectValue(szValue, m_img))
			ERRLOG2(_T("Analysis Value: %s = %s FAILED."), szProperty, szValue);
		m_nRateStarWidth = m_img.m_cx / 2;
	}
	else
		return FALSE;

	return TRUE;
}

#ifdef _SKIN_EDITOR_
void CSkinRateCtrl::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObject::EnumProperties(listProperties);

	listProperties.AddPropImage(SZ_PN_IMAGE, SZ_PN_IMAGERECT, m_strBmpFile.c_str(), m_img);
}
#endif // _SKIN_EDITOR_

void CSkinRateCtrl::SetRating(int nRating)
{
	m_nRating = nRating;

	Invalidate();
}

BOOL CSkinRateCtrl::IsKindOf(LPCTSTR szClassName)
{
	if (CUIObject::IsKindOf(szClassName))
		return TRUE;

	return _tcsicmp(szClassName, ms_szClassName) == 0;
}
