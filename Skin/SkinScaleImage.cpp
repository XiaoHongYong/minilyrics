/********************************************************************
	Created  :	2002/03/27	0:45
	FileName :	SkinScaleImage.cpp
	Author   :	xhy
	
	Purpose  :	适合于界面中可以伸缩的 水平 或 垂直 的条
*********************************************************************/

#include "StdAfx.h"
#include "Skin.h"
#include "SkinScaleImage.h"

UIOBJECT_CLASS_NAME_IMP(CSkinXScaleImage, _T("HStretchImage"))

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSkinXScaleImage::CSkinXScaleImage()
{
	m_nScaleXStart = 0;
	m_nScaleXEnd = 0;
	m_bpm = BPM_BLEND;
}

CSkinXScaleImage::~CSkinXScaleImage()
{

}

void CSkinXScaleImage::Draw(CRawGraph *pGraph)
{
	int			nWidthLeft;
	int			nWidthRight;

	nWidthLeft = m_nScaleXStart - m_imgBk.m_x;
	nWidthRight = m_imgBk.m_x + m_imgBk.m_cx - m_nScaleXEnd;

	// 画出第一部分
	// X 方向上的映射关系
	// 1.
	// 从图片的           m_imgBk.m_x ----> m_nScaleXStart
	// 映射到 UIObject的  m_formleft  ----> m_formLeft + m_nScaleXStart - m_imgBk.m_x
	m_imgBk.Blt(pGraph, 
		m_rcObj.left, m_rcObj.top,
		nWidthLeft, m_imgBk.m_cy,
		m_imgBk.m_x, m_imgBk.m_y, m_bpm
	);


	// 2.
	// 从图片的           m_nScaleXStart                             ----> m_nScaleXEnd
	// 映射到 UIObject的  m_formLeft + m_nScaleXStart - m_imgBk.m_x  ----> m_formRight - (m_imgBk.m_x + m_imgBk.m_cx - m_nScaleXEnd)
	m_imgBk.StretchBlt(pGraph, 
		m_rcObj.left + nWidthLeft, m_rcObj.top,
			m_rcObj.Width() - nWidthLeft - nWidthRight, m_rcObj.Height(),
			m_nScaleXStart, m_imgBk.m_y,
			m_nScaleXEnd - m_nScaleXStart, m_imgBk.m_cy, m_bpm
	);

	// 3.
	// 从图片的           m_nScaleXEnd                             ----> m_imgBk.m_x + m_imgBk.m_cx
	// 映射到 UIObject的  m_formRight - (m_imgBk.m_x + m_imgBk.m_cx - m_nScaleXEnd)  ----> m_formRight
	m_imgBk.Blt(pGraph, 
		m_rcObj.right - nWidthRight, m_rcObj.top,
		nWidthRight, m_imgBk.m_cy,
		m_nScaleXEnd, m_imgBk.m_y, m_bpm
	);
}

BOOL CSkinXScaleImage::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
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
			ERRLOG2(_T("Analysis Value: %s = %s FAILED."), szProperty, szValue);
	}
	else if (_tcsicmp(szProperty, _T("StretchStartX")) == 0 ||
		_tcsicmp(szProperty, _T("ImageScaleXStart")) == 0)
		m_nScaleXStart = _ttoi(szValue);
	else if (_tcsicmp(szProperty, _T("StretchEndX")) == 0 || 
		_tcsicmp(szProperty, _T("ImageScaleXEnd")) == 0)
		m_nScaleXEnd = _ttoi(szValue);
	else if (IsPropertyName(szProperty, _T("BlendPixMode")))
	{
		m_bpm = BlendPixModeFromStr(szValue);
	}
	else
		return FALSE;

	return TRUE;
}

#ifdef _SKIN_EDITOR_
void CSkinXScaleImage::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObject::EnumProperties(listProperties);

	vector<tstring>		vExtra;
	TCHAR				szBuff[64];

	_itot_s(m_nScaleXStart, szBuff, CountOf(szBuff), 10);
	vExtra.push_back(_T("StretchStartX"));
	vExtra.push_back(szBuff);

	_itot_s(m_nScaleXEnd, szBuff, CountOf(szBuff), 10);
	vExtra.push_back(_T("StretchEndX"));
	vExtra.push_back(szBuff);

	listProperties.AddPropImageEx(SZ_PN_IMAGE, SZ_PN_IMAGERECT, m_strBmpBkFile.c_str(), m_imgBk, vExtra);
	listProperties.AddPropBlendPixMode(_T("BlendPixMode"), m_bpm, m_bpm != BPM_BLEND);
}
#endif // _SKIN_EDITOR_

//////////////////////////////////////////////////////////////////////
// CSkinYScaleImage

UIOBJECT_CLASS_NAME_IMP(CSkinYScaleImage, _T("VStretchImage"))

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSkinYScaleImage::CSkinYScaleImage()
{
	m_nScaleYStart = 0;
	m_nScaleYEnd = 0;
	m_bpm = BPM_BLEND;
}

CSkinYScaleImage::~CSkinYScaleImage()
{

}

void CSkinYScaleImage::Draw(CRawGraph *pGraph)
{
	int			nHeightTop;
	int			nHeightBottom;


	nHeightTop = m_nScaleYStart - m_imgBk.m_y;
	nHeightBottom = m_imgBk.m_y + m_imgBk.m_cy - m_nScaleYEnd;

	// 画出第一部分
	// Y 方向上的映射关系
	// 1.
	// 从图片的           m_imgBk.m_y ----> m_nScaleYStart
	// 映射到 UIObject的  m_formtop  ----> m_formTop + m_nScaleYStart - m_imgBk.m_y
	m_imgBk.Blt(pGraph, 
		m_rcObj.left, m_rcObj.top,
		m_imgBk.m_cx, nHeightTop,
		m_imgBk.m_x, m_imgBk.m_y, m_bpm
	);


	// 2.
	// 从图片的           m_nScaleYStart                             ----> m_nScaleYEnd
	// 映射到 UIObject的  m_formTop + m_nScaleYStart - m_imgBk.m_y  ----> m_formBottom - (m_imgBk.m_y + m_imgBk.m_cy - m_nScaleYEnd)
	m_imgBk.StretchBlt(pGraph, 
		m_rcObj.left, m_rcObj.top + nHeightTop,
		m_rcObj.Width(), m_rcObj.Height() - nHeightTop - nHeightBottom,
		m_imgBk.m_x, m_nScaleYStart,
		m_imgBk.m_cx, m_nScaleYEnd - m_nScaleYStart, m_bpm
	);

	// 3.
	// 从图片的           m_nScaleYEnd                             ----> m_imgBk.m_y + m_imgBk.m_cy
	// 映射到 UIObject的  m_formBottom - (m_imgBk.m_y + m_imgBk.m_cy - m_nScaleYEnd)  ----> m_formBottom
	m_imgBk.Blt(pGraph, 
		m_rcObj.left, m_rcObj.bottom  - nHeightBottom,
		m_imgBk.m_cx, nHeightBottom,
		m_imgBk.m_x, m_nScaleYEnd, m_bpm
	);
}

BOOL CSkinYScaleImage::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	assert(m_pSkin);

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
			ERRLOG2(_T("Analysis Value: %s = %s FAILED."), szProperty, szValue);
	}
	else if (_tcsicmp(szProperty, _T("StretchStartY")) == 0 ||
		_tcsicmp(szProperty, _T("ImageScaleYStart")) == 0)
		m_nScaleYStart = _ttoi(szValue);
	else if (_tcsicmp(szProperty, _T("StretchEndY")) == 0 || 
		_tcsicmp(szProperty, _T("ImageScaleYEnd")) == 0)
		m_nScaleYEnd = _ttoi(szValue);
	else if (IsPropertyName(szProperty, _T("BlendPixMode")))
	{
		m_bpm = BlendPixModeFromStr(szValue);
	}
	else
		return FALSE;

	return TRUE;
}

#ifdef _SKIN_EDITOR_
void CSkinYScaleImage::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObject::EnumProperties(listProperties);

	vector<tstring>		vExtra;
	TCHAR				szBuff[64];

	_itot_s(m_nScaleYStart, szBuff, CountOf(szBuff), 10);
	vExtra.push_back(_T("StretchStartY"));
	vExtra.push_back(szBuff);

	_itot_s(m_nScaleYEnd, szBuff, CountOf(szBuff), 10);
	vExtra.push_back(_T("StretchEndY"));
	vExtra.push_back(szBuff);

	listProperties.AddPropImageEx(SZ_PN_IMAGE, SZ_PN_IMAGERECT, m_strBmpBkFile.c_str(), m_imgBk, vExtra);
	listProperties.AddPropBlendPixMode(_T("BlendPixMode"), m_bpm, m_bpm != BPM_BLEND);
}
#endif // _SKIN_EDITOR_
