// SkinMenuBar.cpp: implementation of the CSkinMenuBar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MLSkin.h"
#include "SkinMenuBar.h"


UIOBJECT_CLASS_NAME_IMP(CSkinMenuBar, _T("MenuBar"))

CSkinMenuBar::CSkinMenuBar()
{
	m_bpm = BPM_ALPHA_BLEND;
}

CSkinMenuBar::~CSkinMenuBar()
{

}


void CSkinMenuBar::Draw(CRawGraph *pGraph)
{
}

bool CSkinMenuBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	return false;
}

bool CSkinMenuBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	return false;
}

bool CSkinMenuBar::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (m_pSkin->IsZoomed())
		m_pSkin->ShowWindow(SW_RESTORE);
	else
		m_pSkin->ShowWindow(SW_MAXIMIZE);

	return true;
}

BOOL CSkinMenuBar::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
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
}

void CSkinMenuBar::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObjProperty		property;

	CUIObject::EnumProperties(listProperties);

	listProperties.AddPropBoolStr(_T("EnableDblClick"), m_bEnableMaximize, m_bEnableMaximize);

	vector<tstring>		vExtra;
	TCHAR				szBuff[64];

	_itot(m_nCutPos1, szBuff, 10);
	vExtra.push_back(_T("StretchStartX"));
	vExtra.push_back(szBuff);

	_itot(m_nCutPos2, szBuff, 10);
	vExtra.push_back(_T("StretchEndX"));
	vExtra.push_back(szBuff);

	_itot(m_nCutPos3, szBuff, 10);
	vExtra.push_back(_T("StretchStartX2"));
	vExtra.push_back(szBuff);

	_itot(m_nCutPos4, szBuff, 10);
	vExtra.push_back(_T("StretchEndX2"));
	vExtra.push_back(szBuff);

	listProperties.AddPropImageEx(SZ_PN_IMAGE, SZ_PN_IMAGERECT, m_strBmpBkFile.c_str(), m_imgBk, vExtra);

	listProperties.AddPropImage(_T("ImageFocus"), _T("ImageFocusRect"), m_strBmpFocusFile.c_str(), m_imgFocus, !m_strBmpFocusFile.empty());

	listProperties.AddPropBlendPixMode(_T("BlendPixMode"), m_bpm, m_bpm != BPM_ALPHA_BLEND);
}
