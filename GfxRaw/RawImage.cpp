// RawImage.cpp: implementation of the CRawImage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "../SDK/Agg/include/agg_scanline_p.h"
#include "../SDK/Agg/include/agg_renderer_scanline.h"
#include "../SDK/Agg/include/agg_pixfmt_rgba.h"
#include "../SDK/Agg/include/agg_pixfmt_rgb.h"
#include "../SDK/Agg/include/agg_rasterizer_scanline_aa.h"

#include "GfxRaw.h"
#include "RawImage.h"

#include "ImageBuffBlt.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void stretch_buff_bilinear_bgra32(agg::rendering_buffer &bufDest, agg::rendering_buffer &bufSrc, CRect &rcDest, agg::rect_f &rcSrc)
{
    typedef agg::pixfmt_bgra32                             pixfmt;
    typedef pixfmt::color_type                             color_type;
    typedef agg::renderer_base<pixfmt>                     renderer_base;
    typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_solid;

    typedef agg::pixfmt_bgra32_pre         pixfmt_pre;
    typedef agg::renderer_base<pixfmt_pre> renderer_base_pre;

	agg::scanline_u8  g_scanline;
	agg::rasterizer_scanline_aa<> g_rasterizer;
    pixfmt            pixf(bufDest);
    pixfmt_pre        pixf_pre(bufDest);
    renderer_base     rb(pixf);
    renderer_base_pre rb_pre(pixf_pre);

    renderer_solid r(rb);


    rb.clear(agg::rgba(1, 1, 1));

    g_rasterizer.clip_box(0, 0, bufDest.width(), bufDest.height());
    g_rasterizer.reset();
    g_rasterizer.move_to_d(rcDest.left, rcDest.top);
    g_rasterizer.line_to_d(rcDest.right, rcDest.top);
    g_rasterizer.line_to_d(rcDest.right, rcDest.bottom);
    g_rasterizer.line_to_d(rcDest.left, rcDest.bottom);

    agg::span_allocator<color_type> sa;
    agg::image_filter_bilinear filter_kernel;
    agg::image_filter_lut filter(filter_kernel, false);

    pixfmt pixf_img(bufSrc);

    typedef agg::image_accessor_clone<pixfmt> img_accessor_type;
    img_accessor_type ia(pixf_img);

	double		ploygon[8];
	int			g_x1, g_y1, g_x2, g_y2;

	g_x1 = (int)(rcSrc.x1);
	g_y1 = (int)(rcSrc.y1);
	g_x2 = (int)(rcSrc.x2);
	g_y2 = (int)(rcSrc.y2);

	ploygon[0] = rcDest.left;
	ploygon[1] = rcDest.top;
	ploygon[2] = rcDest.right;
	ploygon[3] = rcDest.top;
	ploygon[4] = rcDest.right;
	ploygon[5] = rcDest.bottom;
	ploygon[6] = rcDest.left;
	ploygon[7] = rcDest.bottom;

    agg::trans_bilinear tr(ploygon, g_x1, g_y1, g_x2, g_y2);
    if(tr.is_valid())
    {
        typedef agg::span_interpolator_linear<agg::trans_bilinear> interpolator_type;
        interpolator_type interpolator(tr);

        typedef agg::span_image_filter_rgba_2x2<img_accessor_type,
                                                interpolator_type> span_gen_type;
        span_gen_type sg(ia, interpolator, filter);
        agg::render_scanlines_aa(g_rasterizer, g_scanline, rb_pre, sa, sg);
    }
}

void stretch_buff_bilinear_bgr24(agg::rendering_buffer &bufDest, agg::rendering_buffer &bufSrc, CRect &rcDest, agg::rect_f &rcSrc)
{
    typedef agg::pixfmt_bgr24                              pixfmt;
    typedef pixfmt::color_type                             color_type;
    typedef agg::renderer_base<pixfmt>                     renderer_base;
    typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_solid;

    typedef agg::pixfmt_bgr24_pre         pixfmt_pre;
    typedef agg::renderer_base<pixfmt_pre> renderer_base_pre;

	agg::scanline_u8  g_scanline;
	agg::rasterizer_scanline_aa<> g_rasterizer;
    pixfmt            pixf(bufDest);
    pixfmt_pre        pixf_pre(bufDest);
    renderer_base     rb(pixf);
    renderer_base_pre rb_pre(pixf_pre);

    renderer_solid r(rb);


    rb.clear(agg::rgba(1, 1, 1));

    g_rasterizer.clip_box(0, 0, bufDest.width(), bufDest.height());
    g_rasterizer.reset();
    g_rasterizer.move_to_d(rcDest.left, rcDest.top);
    g_rasterizer.line_to_d(rcDest.right, rcDest.top);
    g_rasterizer.line_to_d(rcDest.right, rcDest.bottom);
    g_rasterizer.line_to_d(rcDest.left, rcDest.bottom);

    agg::span_allocator<color_type> sa;
    agg::image_filter_bilinear filter_kernel;
    agg::image_filter_lut filter(filter_kernel, false);

    pixfmt pixf_img(bufSrc);

    typedef agg::image_accessor_clone<pixfmt> img_accessor_type;
    img_accessor_type ia(pixf_img);

	double		ploygon[8];
	int			g_x1, g_y1, g_x2, g_y2;

	g_x1 = (int)(rcSrc.x1);
	g_y1 = (int)(rcSrc.y1);
	g_x2 = (int)(rcSrc.x2);
	g_y2 = (int)(rcSrc.y2);

	ploygon[0] = rcDest.left;
	ploygon[1] = rcDest.top;
	ploygon[2] = rcDest.right;
	ploygon[3] = rcDest.top;
	ploygon[4] = rcDest.right;
	ploygon[5] = rcDest.bottom;
	ploygon[6] = rcDest.left;
	ploygon[7] = rcDest.bottom;

    agg::trans_bilinear tr(ploygon, g_x1, g_y1, g_x2, g_y2);
    if(tr.is_valid())
    {
        typedef agg::span_interpolator_linear<agg::trans_bilinear> interpolator_type;
        interpolator_type interpolator(tr);

        typedef agg::span_image_filter_rgb_2x2<img_accessor_type,
                                                interpolator_type> span_gen_type;
        span_gen_type sg(ia, interpolator, filter);
        agg::render_scanlines_aa(g_rasterizer, g_scanline, rb_pre, sa, sg);
    }
}

bool StretchBltBilinear(PRAW_IMAGE_DATA pImageDst, CRect &rcDst, PRAW_IMAGE_DATA pImageSrc, agg::rect_f &rcSrc, BlendPixMode bpm = BPM_BLEND)
{
	if (pImageDst->bitCount == pImageSrc->bitCount)
	{
		agg::rendering_buffer bufDest(pImageDst->buff, pImageDst->width, pImageDst->height, pImageDst->stride);
		agg::rendering_buffer bufSrc(pImageSrc->buff, pImageSrc->width, pImageSrc->height, pImageSrc->stride);
		if (pImageDst->bitCount == 32)
		{
			stretch_buff_bilinear_bgra32(bufDest, bufSrc, rcDst, rcSrc);
		}
		else if (pImageDst->bitCount == 24)
		{
			stretch_buff_bilinear_bgr24(bufDest, bufSrc, rcDst, rcSrc);
		}
		else
			return false;

		return true;
	}

	return false;
}

bool BltRawImage(PRAW_IMAGE_DATA pImageDst, CRect &rcDst, PRAW_IMAGE_DATA pImageSrc, int xSrc, int ySrc, BlendPixMode bpm, int nOpacitySrc)
{
	if (nOpacitySrc == 255)
	{
		CBuffBitBlter	blter;

		blter.rcDest = rcDst;
		blter.xSrc = xSrc;
		blter.ySrc = ySrc;

		return XBltRawImage<blend_rgb, blend_alpha, copy_rgb, copy_alpha, copy_alpha_255, blend_copy_none, multiply_rgb, multiply_alpha>(blter, pImageDst, pImageSrc, bpm);
	}
	else
	{
		CBuffBitBlterOpacity	blter;

		blter.rcDest = rcDst;
		blter.xSrc = xSrc;
		blter.ySrc = ySrc;
		blter.nOpacitySrc = nOpacitySrc;

		return XBltRawImage<blend_rgb_opacity, blend_alpha_opacity, copy_rgb_opacity, copy_alpha_opacity, copy_alpha_255_opacity, blend_copy_none_opacity, multiply_rgb_opacity, multiply_alpha_opacity>(blter, pImageDst, pImageSrc, bpm);
	}
}


bool StretchBlt(PRAW_IMAGE_DATA pImageSrc, PRAW_IMAGE_DATA pImageDst, CRect &rcDst, agg::rect_f &rcSrc, BlendPixMode bpm, int nOpacitySrc)
{
	if (IsFlagSet(bpm, BPM_BILINEAR))
	{
		return StretchBltBilinear(pImageDst, rcDst, pImageSrc, rcSrc, bpm & ~BPM_BILINEAR);
	}

	if (nOpacitySrc == 255)
	{
		CBuffStretchBitBlter	blter;

		blter.rcDest = rcDst;
		blter.rcSrc = rcSrc;

		return XBltRawImage<blend_rgb, blend_alpha, copy_rgb, copy_alpha, copy_alpha_255, blend_copy_none, multiply_rgb, multiply_alpha>(blter, pImageDst, pImageSrc, bpm);
	}
	else
	{
		CBuffStretchBitBlterOpacity	blter;

		blter.rcDest = rcDst;
		blter.rcSrc = rcSrc;
		blter.nOpacitySrc = nOpacitySrc;

		return XBltRawImage<blend_rgb_opacity, blend_alpha_opacity, copy_rgb_opacity, copy_alpha_opacity, copy_alpha_255_opacity, blend_copy_none_opacity, multiply_rgb_opacity, multiply_alpha_opacity>(blter, pImageDst, pImageSrc, bpm);
	}
}


//////////////////////////////////////////////////////////////////////

CRawImage::CRawImage(void)
{
	m_hImage = NULL;
	m_x = 0;
	m_y = 0;
	m_cx = 0;
	m_cy = 0;
}

CRawImage::~CRawImage(void)
{
	Destroy();
}

void CRawImage::Attach(PRAW_IMAGE_DATA hImage)
{
	if (!hImage)
	{
		Destroy();
		return;
	}
	m_x = 0;
	m_y = 0;

	m_cx = hImage->width;
	m_cy = hImage->height;

	m_hImage = hImage;
}

void CRawImage::Detach()
{
	m_hImage = NULL;
}

void CRawImage::Destroy()
{
	if (m_hImage)
	{
		FreeRawImage(m_hImage);
		m_hImage = NULL;
	}
}

bool CRawImage::Load(LPCTSTR szFile)
{
	PRAW_IMAGE_DATA		hImage;

	Destroy();

	hImage = LoadRawImageDataFromFile(szFile);

	if (hImage)
		Attach(hImage);

	return hImage != NULL;
}

void CRawImage::Blt(CRawGraph *pGraphics, int xDest, int yDest, BlendPixMode bpm)
{
	Blt(pGraphics, xDest, yDest, m_cx, m_cy, m_x, m_y, bpm);
}

void CRawImage::Blt(CRawImage *pImage, int xDest, int yDest, BlendPixMode bpm, int nOpacitySrc)
{
	Blt(pImage, xDest, yDest, m_cx, m_cy, m_x, m_y, bpm, nOpacitySrc);
}

bool CRawImage::Blt(CRawGraph *pGraphics, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, BlendPixMode bpm)
{
	if (!m_hImage)
		return false;

	CRect			rcDst;
	PRAW_IMAGE_DATA	pImageDst;

	pGraphics->GetMappedClipBoundRect(rcDst);

	xDest = pGraphics->MapX(xDest);
	yDest = pGraphics->MapY(yDest);

	// get the clip box to copy
	if (xDest + widthDest < rcDst.right)
		rcDst.right = xDest + widthDest;
	if (yDest + heightDest < rcDst.bottom)
		rcDst.bottom = yDest + heightDest;

	if (xDest >= rcDst.left)
		rcDst.left = xDest;
	else
		xSrc += rcDst.left - xDest;
	if (yDest >= rcDst.top)
		rcDst.top = yDest;
	else
		ySrc += rcDst.top - yDest;

	if (xSrc + rcDst.Width() > m_x + m_cx)
		rcDst.right = rcDst.left + m_x + m_cx - xSrc;
	if (ySrc + rcDst.Height() > m_y + m_cy)
		rcDst.bottom = rcDst.top + m_y + m_cy - ySrc;

	if (rcDst.Width() <= 0 || rcDst.Height() <= 0)
		return true;

	pImageDst = pGraphics->GetRawBuff();

	return BltRawImage(pImageDst, rcDst, m_hImage, xSrc, ySrc, bpm, pGraphics->GetOpacityPainting());
}

void CRawImage::MaskBlt(CRawGraph *pGraphics, int xDest, int yDest, CRawImage *pImgMask, BlendPixMode bpm)
{
	MaskBlt(pGraphics, xDest, yDest, m_cx, m_cy, m_x, m_y, pImgMask, bpm);
}

bool CRawImage::MaskBlt(CRawGraph *pGraphics, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, CRawImage *pImgMask, BlendPixMode bpm)
{
	if (!m_hImage)
		return false;

	CRect			rcDst;
	PRAW_IMAGE_DATA	pImageDst;

	pGraphics->GetMappedClipBoundRect(rcDst);

	xDest = pGraphics->MapX(xDest);
	yDest = pGraphics->MapY(yDest);

	// get the clip box to copy
	if (xDest + widthDest < rcDst.right)
		rcDst.right = xDest + widthDest;
	if (yDest + heightDest < rcDst.bottom)
		rcDst.bottom = yDest + heightDest;

	// determine the start position of Graph, image, mask.
	if (xDest >= rcDst.left)
		rcDst.left = xDest;
	else
	{
		xSrc += rcDst.left - xDest;
	}
	if (yDest >= rcDst.top)
		rcDst.top = yDest;
	else
	{
		ySrc += rcDst.top - yDest;
	}

	// make sure the right is within the image region.
	int			nWidthMax = min_fun(m_cx, pImgMask->width());
	int			nHeightMax = min_fun(m_cy, pImgMask->height());

	if (xSrc + rcDst.Width() > m_x + nWidthMax)
		rcDst.right = rcDst.left + m_x + nWidthMax - xSrc;
	if (ySrc + rcDst.Height() > m_y + nHeightMax)
		rcDst.bottom = rcDst.top + m_y + nHeightMax - ySrc;

	if (rcDst.IsRectEmpty())
		return true;

	pImageDst = pGraphics->GetRawBuff();

	return MaskBlt(pImageDst, rcDst, xSrc, ySrc, pImgMask->GetHandle(), xSrc, ySrc, bpm, pGraphics->GetOpacityPainting());
}

bool CRawImage::Blt(CRawImage *pImage, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, BlendPixMode bpm, int nOpacitySrc)
{
	if (!m_hImage || !pImage->m_hImage)
		return false;

	CRect	rcDst(xDest, yDest, widthDest + xDest, heightDest + yDest);

	if (rcDst.left < pImage->m_x)
		rcDst.left = pImage->m_x;
	if (rcDst.top < pImage->m_y)
		rcDst.top = pImage->m_y;

	if (xSrc + widthDest > m_x + m_cx)
		rcDst.right = rcDst.left + m_x + m_cx - xSrc;
	if (ySrc + heightDest > m_y + m_cy)
		rcDst.bottom = rcDst.top + m_y + m_cy - ySrc;

	if (rcDst.Width() > pImage->m_cx)
		rcDst.right = rcDst.left + pImage->m_cx;
	if (rcDst.Height() > pImage->m_cy)
		rcDst.bottom = rcDst.top + pImage->m_cy;

	if (rcDst.top >= pImage->m_y + pImage->m_cy)
		return true;

	if (rcDst.left >= pImage->m_x + pImage->m_cx)
		return true;

	return BltRawImage(pImage->m_hImage, rcDst, m_hImage, xSrc, ySrc, bpm, nOpacitySrc);
}

void CRawImage::StretchBlt(CRawGraph *pGraphics, int xDest, int yDest, int widthDest, int heightDest, BlendPixMode bpm)
{
	StretchBlt(pGraphics, xDest, yDest, widthDest, heightDest, m_x, m_y, m_cx, m_cy, bpm);
}

bool CRawImage::StretchBlt(CRawGraph *pGraphics, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, int widthSrc, int heightSrc, BlendPixMode bpm)
{
	if (!m_hImage)
		return false;

	CRect			rcDst;
	PRAW_IMAGE_DATA	pImageDst;

	if (xSrc >= m_hImage->width || ySrc >= m_hImage->height
		|| widthDest == 0 || heightDest == 0)
		return true;

	if (xSrc + widthSrc > m_x + m_cx)
		widthSrc = m_x + m_cx - xSrc;
	if (ySrc + heightSrc > m_y + m_cy)
		heightSrc = m_y + m_cy - ySrc;

	float		dx, dy;
	agg::rect_f		rcSrc;

	dx = (float)widthSrc / widthDest;
	dy = (float)heightSrc / heightDest;

	pGraphics->GetMappedClipBoundRect(rcDst);

	xDest = pGraphics->MapX(xDest);
	yDest = pGraphics->MapY(yDest);

	if (xDest > rcDst.left)
	{
		rcDst.left = xDest;
		rcSrc.x1 = (float)xSrc;
	}
	else
		rcSrc.x1 = xSrc + dx * (rcDst.left - xDest);

	if (yDest > rcDst.top)
	{
		rcDst.top = yDest;
		rcSrc.y1 = (float)ySrc;
	}
	else
		rcSrc.y1 = ySrc + dy * (rcDst.top - yDest);

	// get the clip box to copy
	if (xDest + widthDest <= rcDst.right)
	{
		rcDst.right = xDest + widthDest;
		rcSrc.x2 = (float)(xSrc + widthSrc);
	}
	else
		rcSrc.x2 = xSrc + widthSrc - dx * (xDest + widthDest - rcDst.right);

	if (yDest + heightDest <= rcDst.bottom)
	{
		rcDst.bottom = yDest + heightDest;
		rcSrc.y2 = (float)(ySrc + heightSrc);
	}
	else
		rcSrc.y2 = ySrc + heightSrc - dy * (yDest + heightDest - rcDst.bottom);

	if (rcDst.Width() <= 0 || rcDst.Height() <= 0)
		return true;

	pImageDst = pGraphics->GetRawBuff();

	ASSERT(rcSrc.x1 >= xSrc);
	ASSERT(rcSrc.x2 <= xSrc + widthSrc);
	ASSERT(rcSrc.y1 >= ySrc);
	ASSERT(rcSrc.y2 <= ySrc + heightSrc);

	return ::StretchBlt(m_hImage, pImageDst, rcDst, rcSrc, bpm, pGraphics->GetOpacityPainting());
}

void CRawImage::StretchBlt(CRawImage *pImage, int xDest, int yDest, int widthDest, int heightDest, BlendPixMode bpm, int nOpacitySrc)
{
	StretchBlt(pImage, xDest, yDest, widthDest, heightDest, m_x, m_y, m_cx, m_cy, bpm, nOpacitySrc);
}


bool CRawImage::StretchBlt(CRawImage *pImage, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, int widthSrc, int heightSrc, BlendPixMode bpm, int nOpacitySrc)
{
	if (!m_hImage || !pImage->m_hImage)
		return false;

	CRect			rcDst;

	if (xSrc >= m_hImage->width || ySrc >= m_hImage->height
		|| widthDest == 0 || heightDest == 0)
		return true;

	if (xSrc + widthSrc > m_x + m_cx)
		widthSrc = m_x + m_cx - xSrc;
	if (ySrc + heightSrc > m_y + m_cy)
		heightSrc = m_y + m_cy - ySrc;

	float		dx, dy;
	agg::rect_f		rcSrc;

	dx = (float)widthSrc / widthDest;
	dy = (float)heightSrc / heightDest;

	rcDst.SetRect(pImage->m_x, pImage->m_y, pImage->m_x + pImage->m_cx, pImage->m_y + pImage->m_cy);

	if (xDest > rcDst.left)
	{
		rcDst.left = xDest;
		rcSrc.x1 = (float)xSrc;
	}
	else
		rcSrc.x1 = xSrc + dx * (xDest - rcDst.left);

	if (yDest > rcDst.top)
	{
		rcDst.top = yDest;
		rcSrc.y1 = (float)ySrc;
	}
	else
		rcSrc.y1 = ySrc + dy * (rcDst.top - yDest);

	// get the clip box to copy
	if (xDest + widthDest < rcDst.right)
	{
		rcDst.right = xDest + widthDest;
		rcSrc.x2 = (float)(xSrc + widthSrc);
	}
	else
		rcSrc.x2 = xSrc + widthSrc - dx * (xDest + widthDest - rcDst.right);

	if (yDest + heightDest < rcDst.bottom)
	{
		rcDst.bottom = yDest + heightDest;
		rcSrc.y2 = (float)(ySrc + heightSrc);
	}
	else
		rcSrc.y2 = ySrc + heightSrc - dy * (yDest + heightDest - rcDst.bottom);

	if (rcDst.Width() <= 0 || rcDst.Height() <= 0)
		return true;

	if (rcDst.top >= pImage->m_y + pImage->m_cy)
		return true;

	if (rcDst.left >= pImage->m_x + pImage->m_cx)
		return true;

	return ::StretchBlt(m_hImage, pImage->m_hImage, rcDst, rcSrc, bpm, nOpacitySrc);
}

// Mask Blt
bool CRawImage::MaskBlt(PRAW_IMAGE_DATA pImageDst, CRect &rcDst, int xSrc, int ySrc, PRAW_IMAGE_DATA pImageMask, int xMask, int yMask, BlendPixMode bpm, int nOpacitySrc)
{
	if (nOpacitySrc == 255)
	{
		CBuffMaskBlter	blter;

		blter.rcDest = rcDst;
		blter.xSrc = xSrc;
		blter.ySrc = ySrc;
		blter.xMask = xMask;
		blter.yMask = yMask;
		blter.pImageMask = pImageMask;

		return XBltRawImage<mask_blend_rgb, mask_blend_alpha, mask_copy_rgb, mask_copy_alpha, mask_copy_alpha_255, mask_blend_copy_none, mask_multiply_rgb, mask_multiply_alpha>(blter, pImageDst, m_hImage, bpm);
	}
	else
	{
		CBuffMaskBlterOpacity	blter;

		blter.rcDest = rcDst;
		blter.xSrc = xSrc;
		blter.ySrc = ySrc;
		blter.xMask = xMask;
		blter.yMask = yMask;
		blter.pImageMask = pImageMask;
		blter.nOpacitySrc = nOpacitySrc;

		return XBltRawImage<mask_blend_rgb_opacity, mask_blend_alpha_opacity, mask_copy_rgb_opacity, mask_copy_alpha_opacity, mask_copy_alpha_255_opacity, mask_blend_copy_none_opacity, mask_multiply_rgb_opacity, mask_multiply_alpha_opacity>(blter, pImageDst, m_hImage, bpm);
	}
}

bool CRawImage::GetOrginalSize(int &nWidth, int &nHeight)
{
	nWidth = m_cx = m_hImage->width;
	nHeight = m_cy = m_hImage->height;

	return true;
}
