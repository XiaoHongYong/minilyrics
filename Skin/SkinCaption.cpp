// SkinCaption.cpp: implementation of the CSkinCaption class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MLSkin.h"
#include "SkinCaption.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CSkinCaption, _T("CaptionImage"))

CSkinCaption::CSkinCaption()
{
	m_bEnableMaximize = false;
	m_bpm = BPM_BLEND;
	m_nCutPos1 = m_nCutPos2 = m_nCutPos3 = m_nCutPos4 = 0;
}

CSkinCaption::~CSkinCaption()
{

}


void CSkinCaption::Draw(CRawGraph *pGraph)
{
	int			nCenter;
	int			nW1, nW2;
	CSFImage	*pImg;

	// 窗口为激活状态
	if (m_pSkin->IsWndActive() && m_imgFocus.IsValid())
		pImg = &m_imgFocus;
	else
		pImg = &m_imgBk;

	if (!pImg->IsValid())
		return;

	nCenter = pImg->m_cx / 2;
	nW1 = m_rcObj.Width() / 2 - (m_nCutPos1 - pImg->m_x) - (pImg->m_x + nCenter - m_nCutPos2);
	nW2 = m_rcObj.Width() - (m_nCutPos1 - pImg->m_x) - nW1 - (m_nCutPos3 - m_nCutPos2) - (pImg->m_x + pImg->m_cx - m_nCutPos4);

	//////////////////////////////////////////////////////////////////////////
	// *****1____2***3____4*****
	//        w1       w2
	//////////////////////////////////////////////////////////////////////////

	//
	// 左边的
	int		x = m_rcObj.left;
	pImg->Blt(pGraph, x, m_rcObj.top,
			m_nCutPos1 - pImg->m_x, pImg->m_cy,
			pImg->m_x, pImg->m_y, m_bpm
	);

	//
	// 左中，拉伸绘画
	x += m_nCutPos1 - pImg->m_x;
	pImg->StretchBlt(pGraph, x, m_rcObj.top,
				nW1, m_rcObj.Height(),
				m_nCutPos1, pImg->m_y,
				m_nCutPos2 - m_nCutPos1, pImg->m_cy, m_bpm
	);

	//
	// 中间
	x += nW1;
	pImg->Blt(pGraph, x, m_rcObj.top,
			m_nCutPos3 - m_nCutPos2, pImg->m_cy,
			m_nCutPos2, pImg->m_y, m_bpm
	);

	//
	// 右中，拉伸绘画
	x += m_nCutPos3 - m_nCutPos2;
	pImg->StretchBlt(pGraph, x, m_rcObj.top,
				nW2, m_rcObj.Height(),
				m_nCutPos3, pImg->m_y,
				m_nCutPos4 - m_nCutPos3, pImg->m_cy, m_bpm
	);

	//
	// 右边的
	x += nW2;
	pImg->Blt(pGraph, x, m_rcObj.top,
			pImg->m_x + pImg->m_cx - m_nCutPos4, pImg->m_cy,
			m_nCutPos4, pImg->m_y, m_bpm
	);
}

bool CSkinCaption::OnLButtonDown(UINT nFlags, CPoint point)
{
	return false;
}

bool CSkinCaption::OnLButtonUp(UINT nFlags, CPoint point)
{
	return false;
}

bool CSkinCaption::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (m_pSkin->IsZoomed())
		m_pSkin->ShowWindow(SW_RESTORE);
	else
		m_pSkin->ShowWindow(SW_MAXIMIZE);

	return true;
}

BOOL CSkinCaption::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CUIObject::SetProperty(szProperty, szValue))
		return TRUE;

	if (_tcsicmp(szProperty, SZ_PN_IMAGE) == 0)
	{
		m_strBmpBkFile = szValue;
		m_imgBk.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
	}
	else if (_tcsicmp(szProperty, SZ_PN_IMAGERECT) == 0)
	{
		if (!GetRectValue(szValue, m_imgBk))
			goto PARSE_VALUE_FAILED;
	}
	else if (_tcsicmp(szProperty, _T("ImageFocus")) == 0)
	{
		m_strBmpFocusFile = szValue;
		m_imgFocus.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
	}
	else if (_tcsicmp(szProperty, _T("ImageFocusRect")) == 0)
	{
		if (!GetRectValue(szValue, m_imgFocus))
			goto PARSE_VALUE_FAILED;
	}
	else if (_tcsicmp(szProperty, _T("EnableDblClick")) == 0)
	{
		m_bEnableMaximize = IsTRUE(szValue);
		if (m_bEnableMaximize)
			m_nMsgWant = UO_MSG_WANT_LBUTTON;
		else
			m_nMsgWant = 0;
	}
	else if (_tcsicmp(szProperty, _T("StretchStartX")) == 0)
		m_nCutPos1 = _ttoi(szValue);
	else if (_tcsicmp(szProperty, _T("StretchEndX")) == 0)
		m_nCutPos2 = _ttoi(szValue);
	else if (_tcsicmp(szProperty, _T("StretchStartX2")) == 0)
		m_nCutPos3 = _ttoi(szValue);
	else if (_tcsicmp(szProperty, _T("StretchEndX2")) == 0)
		m_nCutPos4 = _ttoi(szValue);
	else if (_tcsicmp(szProperty, _T("CutPos")) == 0)
	{
		if (!GetRectValue(szValue, m_nCutPos1, m_nCutPos2, m_nCutPos3, m_nCutPos4))
			goto PARSE_VALUE_FAILED;
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
void CSkinCaption::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObjProperty		property;

	CUIObject::EnumProperties(listProperties);

	listProperties.AddPropBoolStr(_T("EnableDblClick"), m_bEnableMaximize, m_bEnableMaximize);

	vector<tstring>		vExtra;
	TCHAR				szBuff[64];

	_itot_s(m_nCutPos1, szBuff, CountOf(szBuff), 10);
	vExtra.push_back(_T("StretchStartX"));
	vExtra.push_back(szBuff);

	_itot_s(m_nCutPos2, szBuff, CountOf(szBuff), 10);
	vExtra.push_back(_T("StretchEndX"));
	vExtra.push_back(szBuff);

	_itot_s(m_nCutPos3, szBuff, CountOf(szBuff), 10);
	vExtra.push_back(_T("StretchStartX2"));
	vExtra.push_back(szBuff);

	_itot_s(m_nCutPos4, szBuff, CountOf(szBuff), 10);
	vExtra.push_back(_T("StretchEndX2"));
	vExtra.push_back(szBuff);

	listProperties.AddPropImageEx(SZ_PN_IMAGE, SZ_PN_IMAGERECT, m_strBmpBkFile.c_str(), m_imgBk, vExtra);

	listProperties.AddPropImage(_T("ImageFocus"), _T("ImageFocusRect"), m_strBmpFocusFile.c_str(), m_imgFocus, !m_strBmpFocusFile.empty());

	listProperties.AddPropBlendPixMode(_T("BlendPixMode"), m_bpm, m_bpm != BPM_BLEND);
}
#endif // _SKIN_EDITOR_
