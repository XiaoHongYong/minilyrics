// MediaAlbumArtCtrl.cpp: implementation of the CMediaAlbumArtCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MediaAlbumArtCtrl.h"

PRAW_IMAGE_DATA ConvertTo32BppRawImage(PRAW_IMAGE_DATA src);

PRAW_IMAGE_DATA CreateScaleImage(PRAW_IMAGE_DATA pImgSrc, int wDst, int hDst)
{
	ASSERT(pImgSrc);
	CRect	rc;
	int		w, h;

	if (!pImgSrc)
		return NULL;

	w = pImgSrc->width * hDst / pImgSrc->height;
	if (w < wDst)
	{
		// fill left and right
		h = hDst;
	}
	else
	{
		// fill top and bottom
		h = pImgSrc->height * wDst / pImgSrc->width;
		w = wDst;
	}

	CRawImage	imgSrc;
	CRawImage	imgDst;
	PRAW_IMAGE_DATA		pImgDst;

	pImgDst = new RAW_IMAGE_DATA;
	if (!pImgDst->Create(w, h, pImgSrc->bitCount))
		return NULL;

	imgSrc.Attach(pImgSrc);
	imgDst.Attach(pImgDst);

	imgSrc.StretchBlt(&imgDst, 0, 0, w, h, BPM_COPY | BPM_BILINEAR);

	imgSrc.Detach();
	imgDst.Detach();

	return pImgDst;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CMediaAlbumArtCtrl, _T("AlbumArt"))

CMediaAlbumArtCtrl::CMediaAlbumArtCtrl()
{
	m_nMsgWant = UO_MSG_WANT_LBUTTON;

	m_clrBg.set(RGB(0, 0, 0));
	m_brBg.CreateSolidBrush(m_clrBg);
	m_rcImageMaskPos.SetRect(0, 0, 0, 0);
}

CMediaAlbumArtCtrl::~CMediaAlbumArtCtrl()
{
	m_pSkin->UnregisterTimerObject(this);
}

void CMediaAlbumArtCtrl::OnCreate()
{
	CUIObject::OnCreate();

	m_pSkin->RegisterTimerObject(this, 20 * 1000);

	UpdateAlbumArt();
}

bool CMediaAlbumArtCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_nId != SKIN_UIOBJ_ID_UNDEFINE)
	{
		m_pSkin->OnCustomCommand(m_nId);

		return true;
	}
	return false;
}

bool CMediaAlbumArtCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	return false;
}

void CMediaAlbumArtCtrl::OnTimer(int nId)
{
}

void CMediaAlbumArtCtrl::OnSize(int cx, int cy)
{
	if (!IsMaskImageUsed())
		ResizeAlbumArt();
}

void CMediaAlbumArtCtrl::Draw(CRawGraph *pGraph)
{
	if (!m_img.IsValid())
		return;

	if (IsMaskImageUsed())
	{
		m_img.Blt(pGraph, m_rcObj.left, m_rcObj.top, m_rcObj.Width(), m_rcObj.Height(), 0, 0, BPM_ALPHA_BLEND);
		return;
	}

	//
	// No album art mask image is used.
	//

	CRect	rc;
	int		w, h;

	rc = m_rcObj;

	if (m_img.m_cx == 0 || m_img.m_cy == 0 || rc.IsRectEmpty())
		return;

	w = m_img.m_cx * rc.Height() / m_img.m_cy;
	if (w < rc.Width())
	{
		// fill left and right
		h = rc.Height();
		if (!m_bTransparent)
		{
			pGraph->FillRect(rc.left, rc.top, (rc.Width() - w) / 2, h, &m_brBg);
			pGraph->FillRect(rc.right - (rc.Width() - w) / 2, rc.top, (rc.Width() - w) / 2, h, &m_brBg);
		}
	}
	else
	{
		// fill top and bottom
		h = m_img.m_cy * rc.Width() / m_img.m_cx;
		w = rc.Width();
		if (!m_bTransparent)
		{
			pGraph->FillRect(rc.left, rc.top, w, (rc.Height() - h) / 2, &m_brBg);
			pGraph->FillRect(rc.left, rc.bottom - (rc.Height() - h) / 2, w, (rc.Height() - h) / 2, &m_brBg);
		}
	}

	m_img.Blt(pGraph, rc.left + (rc.Width() - w) / 2, rc.top + (rc.Height() - h) / 2, BPM_ALPHA_BLEND);
}

BOOL CMediaAlbumArtCtrl::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CUIObject::SetProperty(szProperty, szValue))
		return TRUE;

	if (_tcsicmp(szProperty, SZ_PN_IMAGE) == 0)
	{
		m_strImgFile = szValue;
		m_img.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
	}
	if (_tcsicmp(szProperty, _T("ImageMask")) == 0)
	{
		m_strImgMask = szValue;
		m_imgMask.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
	}
	if (_tcsicmp(szProperty, _T("ImageMaskRectPos")) == 0)
	{
		GetRectValue(szValue, m_rcImageMaskPos.left, m_rcImageMaskPos.top, m_rcImageMaskPos.right, m_rcImageMaskPos.bottom);
		m_rcImageMaskPos.right += m_rcImageMaskPos.left;
		m_rcImageMaskPos.bottom += m_rcImageMaskPos.top;
	}
	else if (_tcsicmp(szProperty, _T("BgColor")) == 0)
	{
		GetColorValue(m_clrBg, szValue);

		if (m_brBg.IsValid())
			m_brBg.Destroy();
		m_brBg.CreateSolidBrush(m_clrBg);
	}
	else
		return FALSE;

	return TRUE;
}

void CMediaAlbumArtCtrl::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObject::EnumProperties(listProperties);

	listProperties.AddPropImageFile(SZ_PN_IMAGE, m_strImgFile.c_str(), true);
}

void CMediaAlbumArtCtrl::UpdateAlbumArt()
{
	m_img.Destroy();
	m_imgOrg.Destroy();

	if (IsMaskImageUsed())
		m_img.LoadFromSRM(m_pSkin->GetSkinFactory(), m_strImgFile.c_str());
	else
	{
		m_imgOrg.LoadFromSRM(m_pSkin->GetSkinFactory(), m_strImgFile.c_str());
		ResizeAlbumArt();
	}
}

void ClearRawImage(PRAW_IMAGE_DATA pImg, const MLColor &clr);

void CMediaAlbumArtCtrl::CreateAlbumArtImage(PRAW_IMAGE_DATA pImgSrc)
{
	if (!IsMaskImageUsed())
	{
		m_imgOrg.Destroy();
		m_imgOrg.Attach(pImgSrc);

		ResizeAlbumArt();
		return;
	}

	ASSERT(pImgSrc);
	if (!pImgSrc)
		return;

	//
	// Album art mask image is used.
	//
	PRAW_IMAGE_DATA		pImg, pImgSizedAlbumArt;
	int			w, h;
	CRawImage	imgSizedAlbumArt;
	CSFImage	imgAaFrame;

	// Create a scale sized image for album art
	pImgSizedAlbumArt = CreateScaleImage(pImgSrc, m_rcImageMaskPos.Width(), m_rcImageMaskPos.Height());
	if (pImgSizedAlbumArt)
		pImgSizedAlbumArt = ConvertTo32BppRawImage(pImgSizedAlbumArt);
	if (!pImgSizedAlbumArt)
	{
		FreeRawImage(pImgSrc);
		return;
	}
	imgSizedAlbumArt.Attach(pImgSizedAlbumArt);

	// Load album art frame
	imgAaFrame.LoadFromSRM(m_pSkin->GetSkinFactory(), m_strImgFile.c_str());
	if (!imgAaFrame.IsValid())
	{
		FreeRawImage(pImgSrc);
		return;
	}
	pImg = new RAW_IMAGE_DATA;
	if (!pImg->Create(imgAaFrame.width(), imgAaFrame.height(), imgAaFrame.GetHandle()->bitCount))
	{
		delete pImg;
		FreeRawImage(pImgSrc);
		return;
	}

	MLColor	clrBg;

	clrBg.set(0);
	clrBg.setAlpha(0);
	ClearRawImage(pImg, clrBg);
	m_img.Attach(pImg);

	//
	// Album Art --> Add mask --> Blt to Frame
	//
	w = m_rcImageMaskPos.Width();
	h = m_rcImageMaskPos.Height();
	if (w == pImgSizedAlbumArt->width)
	{
		int		hTop = pImgSizedAlbumArt->height / 2;
		int		hBottom = pImgSizedAlbumArt->height - hTop;

		// blt top mask
		m_imgMask.Blt(&imgSizedAlbumArt, 0, 0, w, hTop, 0, 0, BPM_ALPHA_MASK);

		// blt top to frame
		imgSizedAlbumArt.Blt(&imgAaFrame, m_rcImageMaskPos.left, m_rcImageMaskPos.top, w, hTop, 0, 0, BPM_ALPHA_BLEND);

		// blt top to Image
		imgAaFrame.Blt(&m_img, 0, m_img.height() / 2 - (hTop + m_rcImageMaskPos.top), m_img.width(), hTop + m_rcImageMaskPos.top, 0, 0, BPM_COPY);


		// blt bottom mask
		m_imgMask.Blt(&imgSizedAlbumArt, 0, hTop, w, hBottom, 0, m_imgMask.height() - hBottom, BPM_ALPHA_MASK);

		// blt bottom to frame
		imgSizedAlbumArt.Blt(&imgAaFrame, m_rcImageMaskPos.left, m_rcImageMaskPos.bottom - hBottom, w, hBottom, 0, hTop, BPM_ALPHA_BLEND);

		// blt bottom to Image
		imgAaFrame.Blt(&m_img, 0, m_img.height() / 2, m_img.width(), hBottom + (m_img.height() - m_rcImageMaskPos.bottom), 0, m_rcImageMaskPos.bottom - hBottom, BPM_COPY);
	}
	else
	{
		int		wLeft = pImgSizedAlbumArt->width / 2;
		int		wRight = pImgSizedAlbumArt->width - wLeft;

		// blt left mask
		m_imgMask.Blt(&imgSizedAlbumArt, 0, 0, wLeft, h, 0, 0, BPM_ALPHA_MASK);

		// blt left to frame
		imgSizedAlbumArt.Blt(&imgAaFrame, m_rcImageMaskPos.left, m_rcImageMaskPos.top, wLeft, h, 0, 0, BPM_ALPHA_BLEND);

		// blt left to Image
		imgAaFrame.Blt(&m_img, m_img.width() / 2 - (wLeft + m_rcImageMaskPos.left), 0, wLeft + m_rcImageMaskPos.left, m_img.height(), 0, 0, BPM_COPY);


		// blt right mask
		m_imgMask.Blt(&imgSizedAlbumArt, wLeft, 0, wRight, h, m_imgMask.width() - wRight, 0, BPM_ALPHA_MASK);

		// blt right to frame
		imgSizedAlbumArt.Blt(&imgAaFrame, m_rcImageMaskPos.right - wRight, m_rcImageMaskPos.top, wRight, h, wLeft, 0, BPM_ALPHA_BLEND);

		// blt right to Image
		imgAaFrame.Blt(&m_img, m_img.width() / 2, 0, wRight + (m_img.width() - m_rcImageMaskPos.right), m_img.height(), m_rcImageMaskPos.right - wRight, 0, BPM_COPY);
	}
}

void CMediaAlbumArtCtrl::ResizeAlbumArt()
{
	if (IsMaskImageUsed())
		return;
	if (!m_imgOrg.IsValid())
		return;

	m_img.Destroy();

	PRAW_IMAGE_DATA		pImgSizedAlbumArt;

	pImgSizedAlbumArt = CreateScaleImage(m_imgOrg.GetHandle(), m_rcObj.Width(), m_rcObj.Height());
	if (pImgSizedAlbumArt)
		m_img.Attach(pImgSizedAlbumArt);
}
