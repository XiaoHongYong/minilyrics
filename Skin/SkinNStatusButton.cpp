// SkinNStatusButton.cpp: implementation of the CSkinNStatusButton class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MLSkin.h"
#include "SkinNStatusButton.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CSkinNStatusButton, _T("NStatusButton"))

CSkinNStatusButton::CSkinNStatusButton()
{
	m_nMsgWant = UO_MSG_WANT_MOUSEMOVE | UO_MSG_WANT_LBUTTON;

	m_bLBtDown = FALSE;
	m_nCurStatus = 0;

	m_bHover = false;
	m_bEnableHover = false;

	m_bContinuousBegin = false;
	m_bContinuousCmd = false;
	m_bpm = BPM_BLEND;
}

CSkinNStatusButton::~CSkinNStatusButton()
{
	Destroy();
}

bool CSkinNStatusButton::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (!m_bEnable)
		return false;

	assert(m_pSkin);

	if (!m_rcObj.PtInRect(point))
		return false;

	m_bLBtDown = TRUE;

	if (m_bHover)
		m_bHover = false;
	else
	{
		// 捕捉鼠标输入
		m_pSkin->SetCaptureMouse(this);
	}

	Invalidate();

	if (m_bContinuousCmd)
	{
		m_bContinuousBegin = true;
		m_pSkin->RegisterTimerObject(this, 500);
	}

	return true;
}

void CSkinNStatusButton::OnTimer(int nId)
{
	if (IsKeyPressed(VK_LBUTTON))
	{
		CPoint	pt;

		MLWidget::GetCursorPos(&pt);
		m_pSkin->ScreenToClient(pt);

		if (m_rcObj.PtInRect(pt))
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
			m_pSkin->UnregisterTimerObject(this);
			m_pSkin->RegisterTimerObject(this, 100);
			m_bContinuousBegin = false;
		}
	}
	else
		m_pSkin->UnregisterTimerObject(this);
}

bool CSkinNStatusButton::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (!m_bEnable)
		return false;

	// 释放鼠标输入
	m_pSkin->ReleaseCaptureMouse(this);

	if (!m_bLBtDown)
		return true;

	m_bLBtDown = FALSE;

	// 转换按钮的状态
	if (m_nCurStatus >= 0 && m_nCurStatus < (int)m_vBtStatImg.size() - 1)
		m_nCurStatus++;
	else
		m_nCurStatus = 0;

	if (m_nCurStatus < (int)m_vBtStatImg.size())
		m_bEnableHover = m_vBtStatImg[m_nCurStatus]->imgHover.IsValid();

	Invalidate();

	if (m_bContinuousCmd)
		m_pSkin->UnregisterTimerObject(this);

	if (m_nId != SKIN_UIOBJ_ID_UNDEFINE)
		m_pSkin->PostCustomCommandMsg(m_nId);

	if (m_vBtStatImg[m_nCurStatus]->nIDCmd != SKIN_UIOBJ_ID_UNDEFINE)
		m_pSkin->PostCustomCommandMsg(m_vBtStatImg[m_nCurStatus]->nIDCmd);

	return true;
}

bool CSkinNStatusButton::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!m_bEnable)
		return false;

	assert(m_pSkin);

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

	if (m_imgMask.IsValid())
		pImage->MaskBlt(pGraph, m_rcObj.left, m_rcObj.top, &m_imgMask, m_bpm);
	else
		pImage->Blt(pGraph, m_rcObj.left, m_rcObj.top, m_bpm);
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
		else if (_tcsicmp(szPropertyNew, _T("IDCmd")) == 0)
		{
			btimg->nIDCmd = m_pSkin->GetSkinFactory()->GetIDByName(szValue);
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
	else if (IsPropertyName(szProperty, _T("BlendPixMode")))
	{
		m_bpm = BlendPixModeFromStr(szValue);
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

void CSkinNStatusButton::Destroy()
{
	for (int i = 0; i < (int)m_vBtStatImg.size(); i++)
	{
		delete m_vBtStatImg[i];
	}
	m_vBtStatImg.clear();
}
