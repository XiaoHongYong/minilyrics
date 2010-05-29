// SkinNStatusButton.cpp: implementation of the CSkinNStatusButton class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MLSkin.h"
#include "SkinNStatusButton.h"

#define FADE_BUTTON_TIME_OUT		300

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CSkinNStatusButton, _T("NStatusButton"))

CSkinNStatusButton::CSkinNStatusButton()
{
	m_nMsgWant = UO_MSG_WANT_MOUSEMOVE | UO_MSG_WANT_LBUTTON | UO_MSG_WANT_ENTER_KEY;

	m_xMargin = m_yMargin = 0;
	m_xExtendStart = m_xExtendEnd = -1;
	m_bTile = true;

	m_bLBtDown = FALSE;
	m_nCurStatus = 0;

	m_bHover = false;
	m_bEnableHover = false;

	m_bContinuousBegin = false;
	m_bContinuousCmd = false;
	m_bpm = BPM_BLEND;
	m_nTimerIdContinuous = 0;

	m_bFadein = true;
	m_nTimerIdFadein = 0;
	m_dwBeginFadeinTime = 0;
	m_pLastImage = NULL;
}

CSkinNStatusButton::~CSkinNStatusButton()
{
	Destroy();
}

bool CSkinNStatusButton::OnLButtonDown(UINT nFlags, CPoint point)
{
	// m_pSkin->SetFocusUIObject(this);

	assert(m_pSkin);

	if (!IsMouseHitBt(point))
		return false;

	if (m_bHover)
		m_bHover = false;
	else
	{
		// 捕捉鼠标输入
		m_pSkin->SetCaptureMouse(this);
	}

	ButtonDownAction();

	return true;
}

void CSkinNStatusButton::OnTimer(int nId)
{
	if (m_dwBeginFadeinTime != 0)
	{
		// In fade in, just redraw
		Invalidate();
		return;
	}

	if (IsKeyPressed(VK_LBUTTON))
	{
		CPoint	pt;

		MLWidget::GetCursorPos(&pt);
		m_pSkin->ScreenToClient(pt);

		if (IsMouseHitBt(pt))
		{
			if (m_nId != SKIN_UIOBJ_ID_UNDEFINE)
				m_pSkin->PostCustomCommandMsg(m_nId);

			if (m_nCurStatus >= 0 && m_nCurStatus < (int)m_vBtStatImg.size())
			{
				if (m_vBtStatImg[m_nCurStatus]->nIDCmd != SKIN_UIOBJ_ID_UNDEFINE)
					m_pSkin->PostCustomCommandMsg(m_vBtStatImg[m_nCurStatus]->nIDCmd);
			}
		}

		if (m_bContinuousBegin)
		{
			if (m_nTimerIdContinuous)
				m_pSkin->UnregisterTimerObject(this, m_nTimerIdContinuous);
			m_nTimerIdContinuous = m_pSkin->RegisterTimerObject(this, 100);
			m_bContinuousBegin = false;
		}
	}
	else
	{
		m_pSkin->UnregisterTimerObject(this, m_nTimerIdContinuous);
		m_nTimerIdContinuous = 0;
	}
}

bool CSkinNStatusButton::OnLButtonUp(UINT nFlags, CPoint point)
{
	// 释放鼠标输入
	m_pSkin->ReleaseCaptureMouse(this);

	if (!m_bLBtDown)
		return true;

	ButtonUpAction();

	return true;
}

bool CSkinNStatusButton::OnMouseMove(UINT nFlags, CPoint point)
{
	assert(m_pSkin);

	if (IsMouseHitBt(point))
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
		else if (!m_bLBtDown && m_bEnableHover && !m_bHover)
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

	return true;
}

void CSkinNStatusButton::OnKeyUp(UINT nChar, UINT nFlags)
{
	if (nChar == VK_SPACE)
		ButtonUpAction();
}

void CSkinNStatusButton::OnKeyDown(UINT nChar, UINT nFlags)
{
	if (nChar == VK_SPACE && !m_bLBtDown)
		ButtonDownAction();
	else if (nChar == VK_RETURN)
	{
		ButtonDownAction();
		ButtonUpAction();
	}
}

void CSkinNStatusButton::OnSetFocus()
{
	m_bHover = true;
	Invalidate();
}

void CSkinNStatusButton::OnKillFocus()
{
	if (m_bLBtDown)
		m_bLBtDown = false;

	m_bHover = false;
	Invalidate();
}

void CSkinNStatusButton::OnSize(int cx, int cy)
{
	m_rcObj.left -= m_xMargin;
	m_rcObj.right += m_xMargin;
	m_rcObj.top -= m_yMargin;
	m_rcObj.bottom += m_yMargin;

	CUIObject::OnSize(cx, cy);
}

void CSkinNStatusButton::Draw(CRawGraph *pGraph)
{
	if (m_vBtStatImg.empty())
		return;

	if (m_nCurStatus < 0 || m_nCurStatus >= (int)m_vBtStatImg.size())
		m_nCurStatus = 0;

	BtStatImg		*btimg = m_vBtStatImg[m_nCurStatus];
	CSFImage		*pImage = NULL;

	if (m_bEnable)
	{
		if (m_bLBtDown && btimg->imgSel.IsValid())
			pImage = &btimg->imgSel;
		else if (m_bEnableHover && m_bHover && btimg->imgHover.IsValid())
			pImage = &btimg->imgHover;
		else if (btimg->imgBk.IsValid())
			pImage = &btimg->imgBk;
	}
	else
	{
		if (btimg->imgDisabled.IsValid())
			pImage = &btimg->imgDisabled;
		else if (btimg->imgBk.IsValid())
			pImage = &btimg->imgBk;
	}

	if (!pImage)
		return;


	if (m_nTimerIdFadein != 0)
	{
		DWORD		dwTimeCur = GetTickCount();

		if (dwTimeCur - m_dwBeginFadeinTime >= FADE_BUTTON_TIME_OUT)
		{
			m_pSkin->UnregisterTimerObject(this, m_nTimerIdFadein);
			m_nTimerIdFadein = 0;
		}
		else
		{
			int			nAlpha = (dwTimeCur - m_dwBeginFadeinTime) * 255 / FADE_BUTTON_TIME_OUT;
			FadeInDrawBt(pGraph, pImage, nAlpha);
			return;
		}
	}

	if (m_xExtendStart == -1)
	{
		if (m_imgMask.IsValid())
			pImage->MaskBlt(pGraph, m_rcObj.left, m_rcObj.top, &m_imgMask, m_bpm);
		else
			pImage->Blt(pGraph, m_rcObj.left, m_rcObj.top, m_bpm);
	}
	else
	{
		pImage->XScaleBlt(pGraph, m_rcObj.left, m_rcObj.top, 
			m_rcObj.Width(), m_rcObj.Height(),
			m_xExtendStart, m_xExtendEnd, m_bTile, m_bpm);
	}
}

void CSkinNStatusButton::FadeInDrawBt(CRawGraph *pGraph, CSFImage *pImage, int nAlpha)
{
	ASSERT(m_pLastImage && m_bFadein);

	int		nAlphaOld = pGraph->GetOpacityPainting();

	pGraph->SetOpacityPainting((255 - nAlpha) * nAlphaOld / 255);

	if (m_xExtendStart == -1)
	{
		if (m_imgMask.IsValid())
			m_pLastImage->MaskBlt(pGraph, m_rcObj.left, m_rcObj.top, &m_imgMask, m_bpm);
		else
			m_pLastImage->Blt(pGraph, m_rcObj.left, m_rcObj.top, m_bpm);
	}
	else
	{
		m_pLastImage->XScaleBlt(pGraph, m_rcObj.left, m_rcObj.top, 
			m_rcObj.Width(), m_rcObj.Height(),
			m_xExtendStart, m_xExtendEnd, m_bTile, m_bpm);
	}

	pGraph->SetOpacityPainting(nAlpha * nAlphaOld / 255);

	if (m_xExtendStart == -1)
	{
		if (m_imgMask.IsValid())
			pImage->MaskBlt(pGraph, m_rcObj.left, m_rcObj.top, &m_imgMask, m_bpm);
		else
			pImage->Blt(pGraph, m_rcObj.left, m_rcObj.top, m_bpm);
	}
	else
	{
		pImage->XScaleBlt(pGraph, m_rcObj.left, m_rcObj.top, 
			m_rcObj.Width(), m_rcObj.Height(),
			m_xExtendStart, m_xExtendEnd, m_bTile, m_bpm);
	}

	pGraph->SetOpacityPainting(nAlphaOld);
}


BOOL CSkinNStatusButton::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CUIObject::SetProperty(szProperty, szValue))
		return TRUE;

	if (_tcsicmp(szProperty, _T("CurStatus")) == 0)
		m_nCurStatus = _ttoi(szValue);
	else if (szProperty[0] == 'S' && isdigit(szProperty[1]) && szProperty[2] == '_')
	{
		LPCTSTR		szPropertyNew = szProperty + 3;
		int		n = szProperty[1] - '0';
		if (n >= (int)m_vBtStatImg.size())
			SetMaxStat(n + 1);
		BtStatImg	*btimg = m_vBtStatImg[n];

		if (_tcsicmp(szPropertyNew, SZ_PN_IMAGE) == 0)
		{
			btimg->strBkFile = szValue;
			btimg->imgBk.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
			if (!btimg->imgBk.IsValid())
				DBGLOG1(_T("Load image file: %s, FAILED"), szValue);
			ASSERT(btimg->imgBk.IsValid());
		}
		else if (IsPropertyName(szPropertyNew, SZ_PN_IMAGE_SIZE))
		{
			Scan2IntX(szValue, btimg->imgBk.m_cx, btimg->imgBk.m_cy);
		}
		else if (IsPropertyName(szPropertyNew, SZ_PN_IMAGE_POS))
		{
			Scan2IntX(szValue, btimg->imgBk.m_x, btimg->imgBk.m_y);
		}
		else if (IsPropertyName(szPropertyNew, _T("ImageSelPos")))
		{
			btimg->imgSel.LoadFromSRM(m_pSkin->GetSkinFactory(), btimg->strBkFile.c_str());
			btimg->imgSel.m_cx = btimg->imgBk.m_cx;
			btimg->imgSel.m_cy = btimg->imgBk.m_cy;
			Scan2IntX(szValue, btimg->imgSel.m_x, btimg->imgSel.m_y);
		}
		else if (IsPropertyName(szPropertyNew, _T("ImageFocusPos")))
		{
			btimg->imgHover.LoadFromSRM(m_pSkin->GetSkinFactory(), btimg->strBkFile.c_str());
			btimg->imgHover.m_cx = btimg->imgBk.m_cx;
			btimg->imgHover.m_cy = btimg->imgBk.m_cy;
			Scan2IntX(szValue, btimg->imgHover.m_x, btimg->imgHover.m_y);
			m_bEnableHover = true;
		}
		else if (IsPropertyName(szPropertyNew, _T("ImageDisabledPos")))
		{
			btimg->imgDisabled.LoadFromSRM(m_pSkin->GetSkinFactory(), btimg->strBkFile.c_str());
			btimg->imgDisabled.m_cx = btimg->imgBk.m_cx;
			btimg->imgDisabled.m_cy = btimg->imgBk.m_cy;
			Scan2IntX(szValue, btimg->imgDisabled.m_x, btimg->imgDisabled.m_y);
		}
		else if (IsPropertyName(szPropertyNew, _T("IDCmd")))
		{
			btimg->nIDCmd = m_pSkin->GetSkinFactory()->GetIDByName(szValue);
		}

		// Old property styles, decrepted
		else if (_tcsicmp(szPropertyNew, SZ_PN_IMAGERECT) == 0)
		{
			if (!GetRectValue(szValue, btimg->imgBk))
				goto PARSE_VALUE_FAILED;
		}
		else if (_tcsicmp(szPropertyNew, _T("ImageSel")) == 0)
		{
			btimg->strSelFile = szValue;
			btimg->imgSel.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
		}
		else if (_tcsicmp(szPropertyNew, _T("ImageSelRect")) == 0)
		{
			if (!GetRectValue(szValue, btimg->imgSel))
				goto PARSE_VALUE_FAILED;
		}
		else if (_tcsicmp(szPropertyNew, _T("ImageFocus")) == 0)
		{
			btimg->strHoverFile = szValue;
			btimg->imgHover.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
			if (n == m_nCurStatus)
				m_bEnableHover = true;
		}
		else if (_tcsicmp(szPropertyNew, _T("ImageFocusRect")) == 0)
		{
			if (!GetRectValue(szValue, btimg->imgHover))
				goto PARSE_VALUE_FAILED;
		}
		else if (_tcsicmp(szPropertyNew, _T("ImageDisabled")) == 0)
		{
			btimg->strDisabledFile = szValue;
			btimg->imgDisabled.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
		}
		else if (_tcsicmp(szPropertyNew, _T("ImageDisabledRect")) == 0)
		{
			if (!GetRectValue(szValue, btimg->imgDisabled))
				goto PARSE_VALUE_FAILED;
		}
		else
			return FALSE;
	}
	else if (_tcsicmp(szProperty, _T("Continuous")) == 0)
	{
		m_bContinuousCmd = IsTRUE(szValue);
	}
	else if (_tcsicmp(szProperty, _T("IDCmd")) == 0)
	{
		if (m_vBtStatImg.size() == 0)
			m_vBtStatImg.push_back(new BtStatImg);
		m_vBtStatImg[0]->nIDCmd = m_pSkin->GetSkinFactory()->GetIDByName(szValue);
	}
	else if (IsPropertyName(szProperty, _T("XMargin")))
		m_xMargin = _ttoi(szValue);
	else if (IsPropertyName(szProperty, _T("YMargin")))
		m_yMargin = _ttoi(szValue);
	else if (IsPropertyName(szProperty, _T("ExtendPos")))
	{
		Scan2IntX(szValue, m_xExtendStart, m_xExtendEnd);
	}
	else if (IsPropertyName(szProperty, _T("Tile")))
	{
		m_bTile = IsTRUE(szValue);
	}
	else if (IsPropertyName(szProperty, _T("BlendPixMode")))
	{
		m_bpm = BlendPixModeFromStr(szValue);
	}
	else if (IsPropertyName(szProperty, _T("Fadein")))
	{
		m_bFadein = IsTRUE(szValue);
	}
	else if (IsPropertyName(szProperty, _T("TabFocus")))
	{
		if (IsTRUE(szValue))
			m_nMsgWant |= UO_MSG_WANT_KEY;
		else
			m_nMsgWant &= ~UO_MSG_WANT_KEY;
	}
	else
		return FALSE;

	return TRUE;

PARSE_VALUE_FAILED:
	ERRLOG2(_T("Analysis Value: %s = %s FAILED."), szProperty, szValue);
	return TRUE;
}

#ifdef _SKIN_EDITOR_
void CSkinNStatusButton::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObject::EnumProperties(listProperties);

	listProperties.AddPropInt(_T("CurStatus"), m_nCurStatus);

	for (int i = 0; i < (int)m_vBtStatImg.size(); i++)
	{
		BtStatImg	*btimg = m_vBtStatImg[i];
		tstring		strName, strNameRect;
		tstring		strPrefix = _T("S0_");
		strPrefix[1] = '0' + i;

		strName = strPrefix + _T("Image");
		strNameRect = strPrefix + _T("ImageRect");
		listProperties.AddPropImage(strName.c_str(), strNameRect.c_str(), btimg->strBkFile.c_str(), btimg->imgBk);

		strName = strPrefix + _T("ImageSel");
		strNameRect = strPrefix + _T("ImageSelRect");
		listProperties.AddPropImage(strName.c_str(), strNameRect.c_str(), btimg->strSelFile.c_str(), btimg->imgSel, !btimg->strSelFile.empty());

		strName = strPrefix + _T("ImageFocus");
		strNameRect = strPrefix + _T("ImageFocusRect");
		listProperties.AddPropImage(strName.c_str(), strNameRect.c_str(), btimg->strHoverFile.c_str(), btimg->imgHover, !btimg->strHoverFile.empty());

		strName = strPrefix + _T("ImageDisabled");
		strNameRect = strPrefix + _T("ImageDisabledRect");
		listProperties.AddPropImage(strName.c_str(), strNameRect.c_str(), btimg->strDisabledFile.c_str(), btimg->imgDisabled, !btimg->strDisabledFile.empty());

		listProperties.AddPropID(_T("IDCmd"), m_pSkin->GetSkinFactory()->GetStringOfID(btimg->nIDCmd).c_str(), btimg->nIDCmd != SKIN_UIOBJ_ID_UNDEFINE);
	}

	listProperties.AddPropBoolStr(_T("Continuous"), m_bContinuousCmd, m_bContinuousCmd);

	listProperties.AddPropBlendPixMode(_T("BlendPixMode"), m_bpm, m_bpm != BPM_BLEND);
}
#endif // _SKIN_EDITOR_

int CSkinNStatusButton::GetStatus()
{
	return m_nCurStatus;
}

void CSkinNStatusButton::SetStatus(int nStatus)
{
	if (nStatus == m_nCurStatus)
		return;
	if (nStatus >= 0 && nStatus < (int)m_vBtStatImg.size())
	{
		m_nCurStatus = nStatus;
		Invalidate();
	}
}

void CSkinNStatusButton::SetMaxStat(int nMaxStat)
{
	if ((int)m_vBtStatImg.size() > nMaxStat)
	{
		// remove extra stat
		for (int i = m_vBtStatImg.size() - 1; i >= nMaxStat; i--)
		{
			delete m_vBtStatImg.back();
			m_vBtStatImg.pop_back();
		}
	}
	else if ((int)m_vBtStatImg.size() < nMaxStat)
	{
		// add extra stat
		for (int i = m_vBtStatImg.size(); i < nMaxStat; i++)
		{
			m_vBtStatImg.push_back(new BtStatImg);
		}
	}
}

void CSkinNStatusButton::ButtonDownAction()
{
	m_bLBtDown = TRUE;

	if (m_bFadein)
	{
		if (m_nTimerIdFadein != 0)
			m_pSkin->UnregisterTimerObject(this, m_nTimerIdFadein);
		m_nTimerIdFadein = m_pSkin->RegisterTimerObject(this, 50);
		m_dwBeginFadeinTime = GetTickCount();

		m_pLastImage = NULL;
		if (m_bEnableHover)
			m_pLastImage = &(m_vBtStatImg[m_nCurStatus]->imgHover);
		if (!m_pLastImage || !m_pLastImage->IsValid())
			m_pLastImage = &(m_vBtStatImg[m_nCurStatus]->imgBk);
	}
	else if (m_bContinuousCmd)
	{
		m_bContinuousBegin = true;
		m_nTimerIdContinuous = m_pSkin->RegisterTimerObject(this, 500);
	}

	Invalidate();
}

void CSkinNStatusButton::ButtonUpAction()
{
	m_bLBtDown = FALSE;

	if (m_bContinuousCmd)
	{
		m_pSkin->UnregisterTimerObject(this, m_nTimerIdContinuous);
		m_nTimerIdContinuous = 0;
	}

	if (m_bFadein)
	{
		if (m_nTimerIdFadein != 0)
			m_pSkin->UnregisterTimerObject(this, m_nTimerIdFadein);
		m_nTimerIdFadein = m_pSkin->RegisterTimerObject(this, 50);
		m_dwBeginFadeinTime = GetTickCount();

		m_pLastImage = NULL;
		m_pLastImage = &(m_vBtStatImg[m_nCurStatus]->imgSel);
		if (!m_pLastImage->IsValid())
			m_pLastImage = &(m_vBtStatImg[m_nCurStatus]->imgBk);
	}

	if (m_nCurStatus >= 0 && m_nCurStatus < (int)m_vBtStatImg.size() - 1)
		m_nCurStatus++;
	else
		m_nCurStatus = 0;

	if (m_nCurStatus < (int)m_vBtStatImg.size())
		m_bEnableHover = m_vBtStatImg[m_nCurStatus]->imgHover.IsValid();

	Invalidate();

	if (m_nId != SKIN_UIOBJ_ID_UNDEFINE)
		m_pSkin->PostCustomCommandMsg(m_nId);

	if (m_vBtStatImg[m_nCurStatus]->nIDCmd != SKIN_UIOBJ_ID_UNDEFINE)
		m_pSkin->PostCustomCommandMsg(m_vBtStatImg[m_nCurStatus]->nIDCmd);
}

void CSkinNStatusButton::Destroy()
{
	for (int i = 0; i < (int)m_vBtStatImg.size(); i++)
	{
		delete m_vBtStatImg[i];
	}
	m_vBtStatImg.clear();
}

bool CSkinNStatusButton::IsMouseHitBt(POINT &pt)
{
	if (m_xMargin == 0 && m_yMargin == 0)
		return Tobool(m_rcObj.PtInRect(pt));

	CRect	rc = m_rcObj;

	rc.DeflateRect(m_xMargin, m_yMargin);
	return Tobool(rc.PtInRect(pt));
}
