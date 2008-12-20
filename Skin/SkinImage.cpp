/********************************************************************
	Created  :	2002/03/03	13:46
	FileName :	SkinImage.cpp
	Author   :	xhy
	
	Purpose  :	SKIN 中的可伸缩图片控件类
*********************************************************************/

#include "stdafx.h"
#include "MLSkin.h"

UIOBJECT_CLASS_NAME_IMP(CSkinNStatusImage, _T("NStatusImage"))

CSkinNStatusImage::CSkinNStatusImage()
{
	m_BltMode = BLT_COPY;
	m_nCurStatus = 0;
	m_bpm = BPM_ALPHA_BLEND;
}

CSkinNStatusImage::~CSkinNStatusImage()
{
	Destroy();
}

void CSkinNStatusImage::Draw(CRawGraph *pGraph)
{
	if (m_vStImages.empty())
		return;
	if (m_nCurStatus < 0 || m_nCurStatus >= (int)m_vStImages.size())
		m_nCurStatus = 0;

	StatImg		*si = m_vStImages[m_nCurStatus];

	if (m_bpm == BPM_ALPHA_MASK)
		m_pSkin->RedrawBackground(&m_rcObj);

	if (m_BltMode == BLT_STRETCH)
	{
		si->img.StretchBlt(pGraph, m_rcObj.left, m_rcObj.top,
					m_rcObj.Width(), m_rcObj.Height(), m_bpm);
	}
	else if (m_BltMode == BLT_TILE)
	{
		si->img.TileBlt(pGraph, m_rcObj.left, m_rcObj.top,
			m_rcObj.Width(), m_rcObj.Height(), m_bpm);
	}
	else
	{
		if (m_imgMask.IsValid())
			si->img.Blt(pGraph, m_rcObj.left, m_rcObj.top, &m_imgMask, m_bpm);
		else
			si->img.Blt(pGraph, m_rcObj.left, m_rcObj.top, m_bpm);
	}
}

BOOL CSkinNStatusImage::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CUIObject::SetProperty(szProperty, szValue))
		return TRUE;

	if (_tcsicmp(szProperty, _T("CurStatus")) == 0)
		m_nCurStatus = _ttoi(szValue);
	else if (szProperty[0] == 'S' && isdigit(szProperty[1]) && szProperty[2] == '_')
	{
		LPCTSTR		szPropertyNew = szProperty + 3;
		int		n = szProperty[1] - '0';
		if (n >= (int)m_vStImages.size())
			SetMaxStat(n + 1);
		StatImg	*si = m_vStImages[n];

		if (_tcsicmp(szPropertyNew, SZ_PN_IMAGE) == 0)
		{
			si->strImgFile = szValue;
			si->img.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
		}
		else if (_tcsicmp(szPropertyNew, SZ_PN_IMAGERECT) == 0)
		{
			if (!GetRectValue(szValue, si->img))
				ERRLOG2(_T("Analysis Value: %s = %s FAILED."), szProperty, szValue);
		}
		else
			return FALSE;
	}
	else if (_tcsicmp(szProperty, _T("BltMode")) == 0)
	{
		m_BltMode = BltModeFromStr(szValue);
	}
	else if (IsPropertyName(szProperty, _T("BlendPixMode")))
	{
		m_bpm = BlendPixModeFromStr(szValue);
	}
	else
		return FALSE;

	return TRUE;
}

void CSkinNStatusImage::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObject::EnumProperties(listProperties);

	listProperties.AddPropInt(_T("CurStatus"), m_nCurStatus);

	for (int i = 0; i < (int)m_vStImages.size(); i++)
	{
		StatImg		*si = m_vStImages[i];
		tstring		strName, strNameRect;
		tstring		strPrefix = _T("S0_");
		strPrefix[1] = '0' + i;

		strName = strPrefix + SZ_PN_IMAGE;
		strNameRect = strPrefix + SZ_PN_IMAGERECT;
		listProperties.AddPropImage(strName.c_str(), strNameRect.c_str(), si->strImgFile.c_str(), si->img);
	}

	listProperties.AddPropBltMode(_T("BltMode"), m_BltMode);
	listProperties.AddPropBlendPixMode(_T("BlendPixMode"), m_bpm, m_bpm != BPM_ALPHA_BLEND);
}

void CSkinNStatusImage::SetMaxStat(int nMaxStat)
{
	if ((int)m_vStImages.size() > nMaxStat)
	{
		// remove extra stat
		for (int i = m_vStImages.size() - 1; i >= nMaxStat; i--)
		{
			delete m_vStImages.back();
			m_vStImages.pop_back();
		}
	}
	else if ((int)m_vStImages.size() < nMaxStat)
	{
		// add extra stat
		for (int i = m_vStImages.size(); i < nMaxStat; i++)
		{
			m_vStImages.push_back(new StatImg);
		}
	}
}

void CSkinNStatusImage::Destroy()
{
	for (int i = 0; i < (int)m_vStImages.size(); i++)
	{
		delete m_vStImages[i];
	}
	m_vStImages.clear();
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CSkinImage, _T("Image"))

CSkinImage::CSkinImage()
{
	m_BltMode = BLT_COPY;
}

CSkinImage::~CSkinImage()
{
}

BOOL CSkinImage::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CSkinNStatusImage::SetProperty(szProperty, szValue))
		return TRUE;

	if (_tcsicmp(szProperty, SZ_PN_IMAGE) == 0 || _tcsicmp(szProperty, SZ_PN_IMAGERECT) == 0)
	{
		tstring		strNewProperty = _T("S0_");
		strNewProperty += szProperty;
		return CSkinNStatusImage::SetProperty(strNewProperty.c_str(), szValue);
	}
	else
		return FALSE;

	return TRUE;
}

void CSkinImage::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObject::EnumProperties(listProperties);

	if (m_vStImages.size() >= 1)
	{
		StatImg		*si = m_vStImages[0];
		listProperties.AddPropImage(SZ_PN_IMAGE, SZ_PN_IMAGERECT, si->strImgFile.c_str(), si->img);
	}
	else
	{
		CSFImage	img;
		listProperties.AddPropImage(SZ_PN_IMAGE, SZ_PN_IMAGERECT, _T(""), img);
	}

	listProperties.AddPropBltMode(_T("BltMode"), m_BltMode);
}

///////////////////////////////////////////////////////////////////////////////
// class CSkinActiveImage

UIOBJECT_CLASS_NAME_IMP(CSkinActiveImage, _T("ActiveImage"))

CSkinActiveImage::CSkinActiveImage()
{
}

CSkinActiveImage::~CSkinActiveImage()
{
}

void CSkinActiveImage::Draw(CRawGraph *pGraph)
{
	// 1 : active status
	if (m_pSkin->IsWndActive())
		m_nCurStatus = 1;
	else
		m_nCurStatus = 0;

	CSkinNStatusImage::Draw(pGraph);
}

BOOL CSkinActiveImage::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CSkinImage::SetProperty(szProperty, szValue))
		return TRUE;

	if (_tcsicmp(szProperty, _T("ImageFocus")) == 0 ||
		_tcsicmp(szProperty, _T("ImageActive")) == 0)
	{
		tstring		strNewProperty = _T("S1_");
		strNewProperty += SZ_PN_IMAGE;
		return CSkinNStatusImage::SetProperty(strNewProperty.c_str(), szValue);
	}
	else if (_tcsicmp(szProperty, _T("ImageFocusRect")) == 0 ||
		_tcsicmp(szProperty, _T("ImageActiveRect")))
	{
		tstring		strNewProperty = _T("S1_");
		strNewProperty += SZ_PN_IMAGERECT;
		return CSkinNStatusImage::SetProperty(strNewProperty.c_str(), szValue);
	}
	else
		return FALSE;

	return TRUE;
}

void CSkinActiveImage::EnumProperties(CUIObjProperties &listProperties)
{
	CSkinImage::EnumProperties(listProperties);

	if (m_vStImages.size() >= 2)
	{
		StatImg		*si = m_vStImages[1];
		listProperties.AddPropImage(_T("ImageActive"), _T("ImageActiveRect"), si->strImgFile.c_str(), si->img);
	}
	else
	{
		CSFImage	img;
		listProperties.AddPropImage(_T("ImageActive"), _T("ImageActiveRect"), _T(""), img);
	}
}
