#include "stdafx.h"
#include "SkinFilterCtrl.h"

UIOBJECT_CLASS_NAME_IMP(CSkinFilterCtrl, _T("FilterCtrl"));


CSkinFilterCtrl::CSkinFilterCtrl() : m_fStartPercent(1.0)
{
}


CSkinFilterCtrl::~CSkinFilterCtrl()
{
}


BOOL CSkinFilterCtrl::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CUIObject::SetProperty(szProperty, szValue))
		return true;

	if (IsPropertyName(szProperty, _T("StartPercent")))
	{
		m_fStartPercent = (float)_tstof(szValue);
		if (m_fStartPercent < 0.0001 || m_fStartPercent > 1.0)
			m_fStartPercent = 1.0;
		return true;
	}

	return false;
}

void CSkinFilterCtrl::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObject::EnumProperties(listProperties);
}


void CSkinFilterCtrl::Draw(CRawGraph *pGraph)
{
	const int PIX_R = GfxRaw::PIX_R;
	const int PIX_G = GfxRaw::PIX_G;
	const int PIX_B = GfxRaw::PIX_B;
	const int PIX_A = GfxRaw::PIX_A;
	const int PIX_SIZE = GfxRaw::PIX_SIZE;

	CRect rcClip;
	pGraph->GetClipBoundBox(rcClip);
	rcClip.IntersectRect(&rcClip, &m_rcObj);
	if (rcClip.IsRectEmpty())
		return;

	byte			*pSrc, *pDst;
	RAW_IMAGE_DATA	*pMemData		= pGraph->GetRawBuff();
	uint16			nByteStride		= rcClip.Width() * PIX_SIZE;
	int				nByteLeftOffset = rcClip.left * PIX_SIZE;
	float			fPercent		= m_fStartPercent;
	float			fStep			= fPercent / m_rcObj.Height();

	// Fade out and copy the graph above.
	for (int i = rcClip.top - m_rcObj.top;
		 i < rcClip.bottom - m_rcObj.top
			&& fPercent > 0.0000001
			&& m_rcObj.top - i - 1 >= 0;
		 ++i, fPercent -= fStep)
	{
		pSrc = pMemData->RowPtr(m_rcObj.top - i - 1) + nByteLeftOffset;
		pDst = pMemData->RowPtr(rcClip.top + i) + nByteLeftOffset;
		byte * pEnd = pSrc + nByteStride;

		for (; pSrc < pEnd; pSrc += PIX_SIZE, pDst += PIX_SIZE)
		{
			// a * f + b * (1 - f) = (a - b) * f + b
			pDst[PIX_R] = (byte)((pSrc[PIX_R] - pDst[PIX_R]) * fPercent + pDst[PIX_R]);
			pDst[PIX_G] = (byte)((pSrc[PIX_G] - pDst[PIX_G]) * fPercent + pDst[PIX_G]);
			pDst[PIX_B] = (byte)((pSrc[PIX_B] - pDst[PIX_B]) * fPercent + pDst[PIX_B]);
		}
	}
}