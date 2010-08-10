// RawGraph.cpp: implementation of the CRawGraph class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "../SDK/Agg/include/agg_scanline_p.h"
#include "../SDK/Agg/include/agg_renderer_scanline.h"
#include "../SDK/Agg/include/agg_pixfmt_rgba.h"
#include "../SDK/Agg/include/agg_pixfmt_rgb.h"
#include "../SDK/Agg/include/agg_rasterizer_scanline_aa.h"
#include "../SDK/Agg/include/agg_path_storage.h"
#include "../SDK/Agg/include/agg_scanline_u.h"
#include "../SDK/Agg/include/agg_conv_stroke.h"
#include "../SDK/Agg/include/agg_renderer_mclip.h"

#include "GfxRaw.h"
#include "RawGraph.h"
#include "ImageBuffBlt.h"

using namespace GfxRaw;

//////////////////////////////////////////////////////////////////////////

CRawGraph::CRawGraph()
{
	m_rawBmpFont = NULL;

	m_clrText.set(RGB(255, 255, 255));
	m_nOpacityPainting = 255;
	m_ptOrigin.x = m_ptOrigin.y = 0;

	memset(&m_rcClip, 0 , sizeof(m_rcClip));
	
	m_bAlphaChannelEnabled = true;
}

CRawGraph::~CRawGraph()
{
}

bool CRawGraph::Create(int cx, int cy, IMLGraphicsBase *pGraph, int nBitCount)
{
	if (!CRawGraphData::Create(cx, cy, pGraph, nBitCount))
		return false;

	m_rcClip.left = m_rcClip.top = 0;
	m_rcClip.right = cx;
	m_rcClip.bottom = cy;

	m_ptOrigin.x = m_ptOrigin.y = 0;

	m_bAlphaChannelEnabled = (nBitCount == 32);

	return true;
}

void CRawGraph::Line(int x1, int y1, int x2, int y2)
{
	typedef agg::scanline_p8 scanline_type;
	scanline_type sl;
	agg::rasterizer_scanline_aa<> ras;

	x1 = MapX(x1);
	x2 = MapX(x2);
	y1 = MapY(y1);
	y2 = MapY(y2);

	agg::path_storage ps;
	agg::conv_stroke<agg::path_storage> pg(ps);
	agg::rgba	clr(m_pen.m_clrPen.r() / (double)255, 
		m_pen.m_clrPen.g() / (double)255, 
		m_pen.m_clrPen.b() / (double)255,
		m_pen.m_clrPen.a() * m_nOpacityPainting / 255 / (double)255);

	pg.width(m_pen.m_nWidth);

	ps.remove_all();
	ps.move_to(x1, y1);
	ps.line_to(x2, y2);
	ras.add_path(pg);

	if (m_imageData.bitCount == 24)
	{
		typedef agg::pixfmt_rgb24	pixfmt;
		typedef agg::renderer_mclip<pixfmt> renderer_mclip;

		agg::rendering_buffer	buf(m_imageData.buff, m_imageData.width, m_imageData.height, m_imageData.stride);
		pixfmt	pixf(buf);
		renderer_mclip rb(pixf);
		rb.add_clip_box(m_rcClip.left, m_rcClip.top, m_rcClip.right, m_rcClip.bottom);

		agg::render_scanlines_aa_solid(ras, sl, rb, clr);
	}
	else if (m_imageData.bitCount == 32)
	{
		typedef agg::pixfmt_rgba32	pixfmt;
		typedef agg::renderer_mclip<pixfmt> renderer_mclip;

		agg::rendering_buffer	buf(m_imageData.buff, m_imageData.width, m_imageData.height, m_imageData.stride);
		pixfmt	pixf(buf);
		renderer_mclip rb(pixf);
		rb.add_clip_box(m_rcClip.left, m_rcClip.top, m_rcClip.right, m_rcClip.bottom);

		agg::render_scanlines_aa_solid(ras, sl, rb, clr);
	}
}

void CRawGraph::Rectangle(int x, int y, int width, int height)
{
	typedef agg::scanline_p8 scanline_type;

	x = MapX(x);
	y = MapY(y);

	scanline_type sl;
	agg::rasterizer_scanline_aa<> ras;

	agg::path_storage ps;
	agg::conv_stroke<agg::path_storage> pg(ps);
	agg::rgba	clr(m_pen.m_clrPen.r() / (double)255, 
		m_pen.m_clrPen.g() / (double)255, 
		m_pen.m_clrPen.b() / (double)255,
		m_pen.m_clrPen.a() * m_nOpacityPainting / 255 / (double)255);

	pg.width(m_pen.m_nWidth);

	ps.remove_all();
	ps.move_to(x, y);
	ps.line_to(x + width, y);
	ps.line_to(x + width, y + height);
	ps.line_to(x, y + height);
	ps.line_to(x, y);
	ras.add_path(pg);

	if (m_imageData.bitCount == 24)
	{
		typedef agg::pixfmt_rgb24	pixfmt;
		typedef agg::renderer_mclip<pixfmt> renderer_mclip;

		agg::rendering_buffer	buf(m_imageData.buff, m_imageData.width, m_imageData.height, m_imageData.stride);
		pixfmt	pixf(buf);
		renderer_mclip rb(pixf);
		rb.add_clip_box(m_rcClip.left, m_rcClip.top, m_rcClip.right, m_rcClip.bottom);

		agg::render_scanlines_aa_solid(ras, sl, rb, clr);
	}
	else if (m_imageData.bitCount == 32)
	{
		typedef agg::pixfmt_rgba32	pixfmt;
		typedef agg::renderer_mclip<pixfmt> renderer_mclip;

		agg::rendering_buffer	buf(m_imageData.buff, m_imageData.width, m_imageData.height, m_imageData.stride);
		pixfmt	pixf(buf);
		renderer_mclip rb(pixf);
		rb.add_clip_box(m_rcClip.left, m_rcClip.top, m_rcClip.right, m_rcClip.bottom);

		agg::render_scanlines_aa_solid(ras, sl, rb, clr);
	}
}

void CRawGraph::SetPen(CRawPen &pen)
{
	m_pen = pen;
}

void CRawGraph::GetClipBoundBox(CRect &rc)
{
	rc = m_rcClip;
}

void CRawGraph::SetClipBoundBox(const CRect &rc)
{
	if (rc.left > m_rcClip.left)
		m_rcClip.left = rc.left;
	if (rc.top > m_rcClip.top)
		m_rcClip.top = rc.top;

	if (rc.right < m_rcClip.right)
		m_rcClip.right = rc.right;
	if (rc.bottom < m_rcClip.bottom)
		m_rcClip.bottom = rc.bottom;


	if (m_rcClip.right < m_rcClip.left)
		m_rcClip.right = m_rcClip.left;
	if (m_rcClip.bottom < m_rcClip.top)
		m_rcClip.bottom = m_rcClip.top;
}

void CRawGraph::ResetClipBoundBox(const CRect &rc)
{
	m_rcClip = rc;
	if (MapX(m_rcClip.left) < 0)
		m_rcClip.left = RevertMapX(0);
	if (MapY(m_rcClip.top) < 0)
		m_rcClip.top = RevertMapY(0);
	if (MapX(m_rcClip.right) > Width())
		m_rcClip.right = RevertMapX(Width());
	if (MapY(m_rcClip.bottom) > Height())
		m_rcClip.bottom = RevertMapY(Height());

	if (m_rcClip.right < m_rcClip.left)
		m_rcClip.right = m_rcClip.left;
	if (m_rcClip.bottom < m_rcClip.top)
		m_rcClip.bottom = m_rcClip.top;
}

void CRawGraph::ClearClipBoundBox()
{
	m_rcClip.left = RevertMapX(0);
	m_rcClip.top = RevertMapY(0);
	m_rcClip.right = RevertMapX(Width());
	m_rcClip.bottom = RevertMapY(Height());
}

bool CRawGraph::TextOut(int x, int y, LPCTSTR szText, int nLen)
{
	if (m_rawBmpFont)
		return m_rawBmpFont->TextOut(this, MapX(x), MapY(y), m_clrText, szText, nLen, m_bAlphaChannelEnabled);
	return false;
}

bool CRawGraph::DrawTextClip(LPCTSTR szText, int nLen, LPRECT lpRect, int xLeftClipOffset)
{
	if (m_rawBmpFont)
	{
		return m_rawBmpFont->DrawText(this, MapX(lpRect->left), MapY(lpRect->top), MapX(lpRect->right - lpRect->left), MapX(xLeftClipOffset), m_clrText, szText, nLen, m_bAlphaChannelEnabled);
	}
	return false;
}

bool CRawGraph::TextOutOutlined(int x, int y, LPCTSTR szText, int nLen, const MLColor &clrText, const MLColor &clrBorder)
{
	if (m_rawBmpFont)
		return m_rawBmpFont->OutlinedTextOut(this, MapX(x), MapY(y), clrText, clrBorder, szText, nLen, m_bAlphaChannelEnabled);
	return false;
}

bool CRawGraph::DrawTextClipOutlined(LPCTSTR szText, int nLen, LPRECT lpRect, const MLColor &clrText, const MLColor &clrBorder, int xLeftClipOffset)
{
	if (m_rawBmpFont)
		return m_rawBmpFont->OutlinedDrawText(this, MapX(lpRect->left), MapY(lpRect->top), MapX(lpRect->right - lpRect->left), MapX(xLeftClipOffset), clrText, clrBorder, szText, nLen, m_bAlphaChannelEnabled);
	return false;
}


bool CRawGraph::DrawTextOutlined(LPCTSTR szText, int nLen, LPRECT lpRect, const MLColor &clrText, const MLColor &clrBorder, UINT uFormat)
{
	RECT	rc = *lpRect;
	MapRect(rc);

	if (m_rawBmpFont)
		return m_rawBmpFont->OutlinedDrawTextEx(this, &rc, clrText, clrBorder, szText, nLen, uFormat, m_bAlphaChannelEnabled);

	return false;
}


bool CRawGraph::DrawText(LPCTSTR szText, int nLen, LPRECT lpRect, UINT uFormat)
{
	RECT	rc = *lpRect;
	MapRect(rc);

	if (m_rawBmpFont)
		return m_rawBmpFont->DrawTextEx(this, &rc, m_clrText, szText, nLen, uFormat, m_bAlphaChannelEnabled);

	return false;
}


bool CRawGraph::GetTextExtentPoint32(LPCTSTR szText, int nLen, LPSIZE pSize)
{
	if (m_rawBmpFont)
		return m_rawBmpFont->GetTextExtentPoint32(szText, nLen, pSize);

	return false;
}

void CRawGraph::SetFont(CRawBmpFont *font)
{
	ASSERT(font);
	if (!font)
		return;

	m_rawBmpFont = font;
}

void CRawGraph::ResetAlphaChannel(BYTE byAlpha, const RECT *lpRect)
{
	if (!m_bAlphaChannelEnabled)
		return;
	
	ASSERT(m_imageData.bitCount == 32);

	int			x, y, xMax, yMax;
	byte		*p, *pEnd, *pRow;
	int			nStrideModify;

	ClipAndMapRect(lpRect, x, y, xMax, yMax);
	if (y >= yMax || x >= xMax)
		return;
	nStrideModify = (xMax - x) * PIX_SIZE;

	// Modify alpha channel directly
	pRow = m_imageData.PixPtr(x, y) + PIX_A;
	for (; y < yMax; y++)
	{
		pEnd = pRow + nStrideModify;
		for (p = pRow; p < pEnd; p += PIX_SIZE)
		{
			*p = byAlpha;
		}
		pRow += m_imageData.stride;
	}
}

void CRawGraph::MultiplyAlpha(BYTE byAlpha, const RECT *lpRect)
{
	if (!m_bAlphaChannelEnabled)
		return;

	ASSERT(m_imageData.bitCount == 32);

	int			x, y, xMax, yMax;
	byte		*p, *pEnd, *pRow;
	int			nStrideModify;

	ClipAndMapRect(lpRect, x, y, xMax, yMax);
	if (y >= yMax || x >= xMax)
		return;
	nStrideModify = (xMax - x) * PIX_SIZE;

	// PIX_SIZE is the pixel size(rgba).
	pRow = m_imageData.PixPtr(x, y);
	for (; y < yMax; y++)
	{
		pEnd = pRow + nStrideModify;
		for (p = pRow; p < pEnd; p ++)
		{
			*p = (byte)((*p * byAlpha) / 256);
		}
		pRow += m_imageData.stride;
	}
}

void CRawGraph::VertAlphaFadeOut(const RECT *lpRect, bool bTop)
{
	if (!m_bAlphaChannelEnabled)
		return;

	ASSERT(m_imageData.bitCount == 32);
	if (lpRect->bottom == lpRect->top)
		return;

	int			x, y, xMax, yMax;
	byte		*p, *pEnd, *pRow;
	float		fStep, fStart;
	int			nStrideModify;

	ClipAndMapRect(lpRect, x, y, xMax, yMax);
	if (y >= yMax || x >= xMax)
		return;
	nStrideModify = (xMax - x) * PIX_SIZE;

	fStep = (float)1 / (lpRect->bottom - lpRect->top);
	if (bTop)
	{
		fStart = 0;
	}
	else
	{
		fStep = -fStep;
		fStart = 1;
	}

	fStart += fStep * (y - MapY(lpRect->top));
	ASSERT(fStart <= 1.0 && fStart >= 0.0);

	// PIX_SIZE is the pixel size(rgba).
	pRow = m_imageData.PixPtr(x, y);
	for (; y < yMax; y++)
	{
		pEnd = pRow + nStrideModify;
		for (p = pRow; p < pEnd; p ++)
		{
			*p = (byte)(*p * fStart);
		}
		pRow += m_imageData.stride;
		fStart += fStep;
	}
}

void CRawGraph::VertFadeOut(const RECT *lpRect, const MLColor &clrBg, bool bTop)
{
	ASSERT(m_imageData.bitCount == 32);
	if (lpRect->bottom == lpRect->top)
		return;

	int			x, y, xMax, yMax;
	byte		*p, *pEnd, *pRow;
	// float		fStep, fStart;
	int			r, g, b, a;
	int			nStrideModify;

	a = clrBg.getAlpha() * m_nOpacityPainting / 255;
	r = clrBg.r() * a / 255;
	g = clrBg.g() * a / 255;
	b = clrBg.b() * a / 255;

	ClipAndMapRect(lpRect, x, y, xMax, yMax);
	if (y >= yMax || x >= xMax)
		return;

	nStrideModify = (xMax - x) * PIX_SIZE;

	int			nFadeRange = lpRect->bottom - lpRect->top;
	int			nFadeStart = y - MapY(lpRect->top);
	
	if (!bTop)
		nFadeStart = nFadeRange - nFadeStart;

/*	fStep = (float)1 / (lpRect->bottom - lpRect->top);
	if (bTop)
	{
		fStart = 0;
	}
	else
	{
		fStep = -fStep;
		fStart = 1;
	}

	fStart += fStep * (y - MapY(lpRect->top));
	ASSERT(fStart <= 1.0 && fStart >= 0.0);*/

	pRow = m_imageData.PixPtr(x, y);
	for (; y < yMax; y++)
	{
		pEnd = pRow + nStrideModify;
		for (p = pRow; p < pEnd; p += PIX_SIZE)
		{
			// a * f + b * (1 - f) = (a - b) * f + b
			p[PIX_B] = (byte)((p[PIX_B] - b) * nFadeStart / nFadeRange + b);
			p[PIX_G] = (byte)((p[PIX_G] - g) * nFadeStart / nFadeRange + g);
			p[PIX_R] = (byte)((p[PIX_R] - r) * nFadeStart / nFadeRange + r);
			p[PIX_A] = (byte)((p[PIX_A] - a) * nFadeStart / nFadeRange + a);
/*			p[PIX_B] = (byte)((p[PIX_B] - b) * fStart + b);
			p[PIX_G] = (byte)((p[PIX_G] - g) * fStart + g);
			p[PIX_R] = (byte)((p[PIX_R] - r) * fStart + r);
			p[PIX_A] = (byte)((p[PIX_A] - a) * fStart + a);*/
		}
		pRow += m_imageData.stride;
		// fStart += fStep;
		if (bTop)
			nFadeStart++;
		else
			nFadeStart--;
	}
}


void CRawGraph::HorzAlphaFadeOut(const RECT *lpRect, bool bLeft)
{
	if (!m_bAlphaChannelEnabled)
		return;

	ASSERT(m_imageData.bitCount == 32);
	if (lpRect->left == lpRect->right)
		return;

	int			x, y, xMax, yMax;
	byte		*p, *pEnd, *pRow;
	float		fStep, fStart, f;
	int			nStrideModify;

	ClipAndMapRect(lpRect, x, y, xMax, yMax);
	if (y >= yMax || x >= xMax)
		return;
	nStrideModify = (xMax - x) * PIX_SIZE;

	fStep = (float)1 / (lpRect->right - lpRect->left);
	if (bLeft)
	{
		fStart = 0;
	}
	else
	{
		fStep = -fStep;
		fStart = 1;
	}

	fStart += fStep * (x - MapX(lpRect->left));
	ASSERT(fStart <= 1.0 && fStart >= 0.0);

	pRow = m_imageData.PixPtr(x, y);
	for (; y < yMax; y++)
	{
		f = fStart;

		pEnd = pRow + nStrideModify;
		for (p = pRow; p < pEnd; p += PIX_SIZE)
		{
			p[PIX_B] = (byte)(p[PIX_B] * f);
			p[PIX_G] = (byte)(p[PIX_G] * f);
			p[PIX_R] = (byte)(p[PIX_R] * f);
			p[PIX_A] = (byte)(p[PIX_A] * f);
			f += fStep;
		}
		pRow += m_imageData.stride;
	}
}


void CRawGraph::HorzFadeOut(const RECT *lpRect, const MLColor &clrBg, bool bLeft)
{
	ASSERT(m_imageData.bitCount == 32);
	if (lpRect->left == lpRect->right)
		return;

	int			x, y, xMax, yMax;
	byte		*p, *pEnd, *pRow;
	float		fStep, fStart, f;
	BYTE		r, g, b, a;
	int			nStrideModify;

	a = clrBg.getAlpha();
	r = clrBg.r() * a / 255;
	g = clrBg.g() * a / 255;
	b = clrBg.b() * a / 255;

	ClipAndMapRect(lpRect, x, y, xMax, yMax);
	if (y >= yMax || x >= xMax)
		return;
	nStrideModify = (xMax - x) * PIX_SIZE;

	fStep = (float)1 / (lpRect->right - lpRect->left);
	if (bLeft)
	{
		fStart = 0;
	}
	else
	{
		fStep = -fStep;
		fStart = 1;
	}

	fStart += fStep * (x - MapX(lpRect->left));
	ASSERT(fStart <= 1.0 && fStart >= 0.0);

	pRow = m_imageData.PixPtr(x, y);
	for (; y < yMax; y++)
	{
		f = fStart;

		pEnd = pRow + nStrideModify;
		for (p = pRow; p < pEnd; p += PIX_SIZE)
		{
			p[PIX_B] = (byte)((p[PIX_B] - b) * f + b);
			p[PIX_G] = (byte)((p[PIX_G] - g) * f + g);
			p[PIX_R] = (byte)((p[PIX_R] - r) * f + r);
			p[PIX_A] = (byte)((p[PIX_A] - a) * f + a);
			f += fStep;
		}
		pRow += m_imageData.stride;
	}
}

void CRawGraph::FillRectXOR(const RECT *lpRect, const MLColor &clrFill)
{
	ASSERT(m_imageData.bitCount == 32);

	int			x, y, xMax, yMax;
	byte		*p, *pEnd, *pRow;
	int			nStrideModify;
	byte		r = clrFill.r(), g = clrFill.g(), b = clrFill.b();

	ClipAndMapRect(lpRect, x, y, xMax, yMax);
	if (y >= yMax || x >= xMax)
		return;
	nStrideModify = (xMax - x) * PIX_SIZE;

	pRow = m_imageData.PixPtr(x, y);
	for (; y < yMax; y++)
	{
		pEnd = pRow + nStrideModify;
		for (p = pRow; p < pEnd; p += PIX_SIZE)
		{
			p[PIX_B] = p[PIX_B] ^ b;
			p[PIX_G] = p[PIX_G] ^ g;
			p[PIX_R] = p[PIX_R] ^ r;
		}
		pRow += m_imageData.stride;
	}
}


template<class _PixRGBBlender, class _PixAlphaBlender>
void FillRectMask(PRAW_IMAGE_DATA pImage, int x, int y, int xMax, int yMax, PRAW_IMAGE_DATA pImageMask, int xMask, int yMask, uint8 *srcPixel)
{
	byte		*p, *pEnd, *pRow;
	byte		*pMask, *pRowMask;
	int			nStrideModify;
	int			nPixSize, nPixSizeMask;

	if (pImage->bitCount == 32)
		nPixSize = 4;
	else if (pImage->bitCount == 24)
		nPixSize = 3;
	else
	{
		ASSERT(0);
		return;
	}

	if (pImageMask->bitCount == 32)
		nPixSizeMask = 4;
	else if (pImageMask->bitCount == 24)
		nPixSizeMask = 3;
	else
	{
		ASSERT(0);
		return;
	}

	nStrideModify = (xMax - x) * nPixSize;

	pRow = pImage->PixPtr(x, y);
	pRowMask = pImageMask->PixPtr(xMask, yMask);
	for (; y < yMax; y++)
	{
		pEnd = pRow + nStrideModify;
		pMask = pRowMask;
		for (p = pRow; p < pEnd; p += nPixSize)
		{
			if (pMask[3] != 0)
			{
				_PixRGBBlender::blend(p, srcPixel, pMask);
				_PixAlphaBlender::blend(p, srcPixel, pMask);
			}
			pMask += nPixSizeMask;
		}
		pRow += pImage->stride;
		pRowMask += pImageMask->stride;
	}
}


void CRawGraph::FillRect(const RECT *lpRect, PRAW_IMAGE_DATA pImgMask, const RECT *rcMask, const MLColor &clrFill, BlendPixMode bpm)
{
	ASSERT(m_imageData.bitCount == 32 || m_imageData.bitCount == 24);

	int			x, y, xMax, yMax;
	uint8		srcPixel[4];

	srcPixel[PIX_R] = clrFill.r() * clrFill.getAlpha() / 255;
	srcPixel[PIX_G] = clrFill.g() * clrFill.getAlpha() / 255;
	srcPixel[PIX_B] = clrFill.b() * clrFill.getAlpha() / 255;
	srcPixel[PIX_A] = clrFill.getAlpha();

	ASSERT(pImgMask);
	if (!pImgMask)
		return;

	if (pImgMask->pixFormat != PF_RGBA32)
		return;

	ClipAndMapRect(lpRect, x, y, xMax, yMax);
	if (y >= yMax || x >= xMax)
		return;

	if (!m_bAlphaChannelEnabled)
		bpm &= ~BPM_CHANNEL_ALPHA;

	// Fill region must be within mask image
	if (xMax - x > rcMask->right - rcMask->left)
		xMax = rcMask->right - rcMask->left + x;
	if (yMax - y > rcMask->bottom - rcMask->top)
		yMax = rcMask->bottom - rcMask->top + y;

	if (IsFlagSet(bpm, BPM_OP_BLEND))
	{
		// blend
		if (IsFlagSet(bpm, BPM_CHANNEL_RGB))
		{
			if (IsFlagSet(bpm, BPM_CHANNEL_ALPHA))
				::FillRectMask<mask_blend_rgb, mask_blend_alpha>(&m_imageData, x, y, xMax, yMax, pImgMask, rcMask->left, rcMask->top, srcPixel);
			else
				::FillRectMask<mask_blend_rgb, mask_blend_copy_none>(&m_imageData, x, y, xMax, yMax, pImgMask, rcMask->left, rcMask->top, srcPixel);
		}
		else
		{
			if (IsFlagSet(bpm, BPM_CHANNEL_ALPHA))
				::FillRectMask<mask_blend_copy_none, mask_blend_alpha>(&m_imageData, x, y, xMax, yMax, pImgMask, rcMask->left, rcMask->top, srcPixel);
		}
	}
	else if (IsFlagSet(bpm, BPM_OP_MULTIPLY))
	{
		// Multiply
		if (IsFlagSet(bpm, BPM_CHANNEL_RGB))
		{
			if (IsFlagSet(bpm, BPM_CHANNEL_ALPHA))
				::FillRectMask<mask_multiply_rgb, mask_multiply_alpha>(&m_imageData, x, y, xMax, yMax, pImgMask, rcMask->left, rcMask->top, srcPixel);
			else
				::FillRectMask<mask_multiply_rgb, mask_blend_copy_none>(&m_imageData, x, y, xMax, yMax, pImgMask, rcMask->left, rcMask->top, srcPixel);
		}
		else
		{
			if (IsFlagSet(bpm, BPM_CHANNEL_ALPHA))
				::FillRectMask<mask_blend_copy_none, mask_multiply_alpha>(&m_imageData, x, y, xMax, yMax, pImgMask, rcMask->left, rcMask->top, srcPixel);
		}
	}
	else
	{
		// copy
		if (IsFlagSet(bpm, BPM_CHANNEL_RGB))
		{
			if (IsFlagSet(bpm, BPM_CHANNEL_ALPHA))
				::FillRectMask<mask_copy_rgb, mask_copy_alpha>(&m_imageData, x, y, xMax, yMax, pImgMask, rcMask->left, rcMask->top, srcPixel);
			else
				::FillRectMask<mask_copy_rgb, mask_blend_copy_none>(&m_imageData, x, y, xMax, yMax, pImgMask, rcMask->left, rcMask->top, srcPixel);
		}
		else
		{
			if (IsFlagSet(bpm, BPM_CHANNEL_ALPHA))
				::FillRectMask<mask_blend_copy_none, mask_copy_alpha>(&m_imageData, x, y, xMax, yMax, pImgMask, rcMask->left, rcMask->top, srcPixel);
		}
	}
}


template<class _PixRGBBlender, class _PixAlphaBlender, class _PixRGBBlender_Opacity, class _PixAlphaBlender_Opacity>
void FillRect(PRAW_IMAGE_DATA pImage, int x, int y, int xMax, int yMax, uint8 *srcPixel, uint8 alpha)
{
	byte		*p, *pEnd, *pRow;
	int			nStrideModify;
	int			nPixSize;

	if (pImage->bitCount == 32)
		nPixSize = 4;
	else if (pImage->bitCount == 24)
		nPixSize = 3;
	else
	{
		ASSERT(0);
		return;
	}

	nStrideModify = (xMax - x) * nPixSize;
	pRow = pImage->PixPtr(x, y);

	if (alpha == 255)
	{
		for (; y < yMax; y++)
		{
			pEnd = pRow + nStrideModify;
			for (p = pRow; p < pEnd; p += nPixSize)
			{
				_PixRGBBlender::blend(p, srcPixel);
				_PixAlphaBlender::blend(p, srcPixel);
			}
			pRow += pImage->stride;
		}
	}
	else
	{
		for (; y < yMax; y++)
		{
			pEnd = pRow + nStrideModify;
			for (p = pRow; p < pEnd; p += nPixSize)
			{
				_PixRGBBlender_Opacity::blend(p, srcPixel, alpha);
				_PixAlphaBlender_Opacity::blend(p, srcPixel, alpha);
			}
			pRow += pImage->stride;
		}
	}
}

/* Test CPU optimizing code for inline funcitions.
void FillRectCopyRGB(PRAW_IMAGE_DATA pImage, int x, int y, int xMax, int yMax, uint8 *srcPixel)
{
	byte		*p, *pEnd, *pRow;
	int			nStrideModify;
	int			nPixSize;
	
	if (pImage->bitCount == 32)
		nPixSize = 4;
	else if (pImage->bitCount == 24)
		nPixSize = 3;
	else
	{
		ASSERT(0);
		return;
	}
	
	nStrideModify = (xMax - x) * nPixSize;
	pRow = pImage->PixPtr(x, y);
	
	for (; y < yMax; y++)
	{
		pEnd = pRow + nStrideModify;
		for (p = pRow; p < pEnd; p += nPixSize)
		{
			p[PIX_B] = srcPixel[PIX_B];
			p[PIX_G] = srcPixel[PIX_G];
			p[PIX_R] = srcPixel[PIX_R];
		}
		pRow += pImage->stride;
	}
}*/

void CRawGraph::FillRect(const RECT *lpRect, const MLColor &clrFill, BlendPixMode bpm)
{
	ASSERT(m_imageData.bitCount == 32 || m_imageData.bitCount == 24);

	int			x, y, xMax, yMax;
	uint8		srcPixel[4];

	srcPixel[PIX_R] = clrFill.r() * clrFill.getAlpha() / 255;
	srcPixel[PIX_G] = clrFill.g() * clrFill.getAlpha() / 255;
	srcPixel[PIX_B] = clrFill.b() * clrFill.getAlpha() / 255;
	srcPixel[PIX_A] = clrFill.getAlpha();

	ClipAndMapRect(lpRect, x, y, xMax, yMax);
	if (y >= yMax || x >= xMax)
		return;

	if (!m_bAlphaChannelEnabled)
		bpm &= ~BPM_CHANNEL_ALPHA;

	if (IsFlagSet(bpm, BPM_OP_BLEND))
	{
		// blend
		if (IsFlagSet(bpm, BPM_CHANNEL_RGB))
		{
			if (IsFlagSet(bpm, BPM_CHANNEL_ALPHA))
				::FillRect<blend_rgb, blend_alpha, blend_rgb_opacity, blend_alpha_opacity>(&m_imageData, x, y, xMax, yMax, srcPixel, m_nOpacityPainting);
			else
				::FillRect<blend_rgb, blend_copy_none, blend_rgb_opacity, blend_copy_none_opacity>(&m_imageData, x, y, xMax, yMax, srcPixel, m_nOpacityPainting);
		}
		else
		{
			if (IsFlagSet(bpm, BPM_CHANNEL_ALPHA))
				::FillRect<blend_copy_none, blend_alpha, blend_copy_none_opacity, blend_alpha_opacity>(&m_imageData, x, y, xMax, yMax, srcPixel, m_nOpacityPainting);
		}
	}
	else if (IsFlagSet(bpm, BPM_OP_MULTIPLY))
	{
		// Multiply
		if (IsFlagSet(bpm, BPM_CHANNEL_RGB))
		{
			if (IsFlagSet(bpm, BPM_CHANNEL_ALPHA))
				::FillRect<multiply_rgb, multiply_alpha, multiply_rgb_opacity, multiply_alpha_opacity>(&m_imageData, x, y, xMax, yMax, srcPixel, m_nOpacityPainting);
			else
				::FillRect<multiply_rgb, blend_copy_none, multiply_rgb_opacity, blend_copy_none_opacity>(&m_imageData, x, y, xMax, yMax, srcPixel, m_nOpacityPainting);
		}
		else
		{
			if (IsFlagSet(bpm, BPM_CHANNEL_ALPHA))
				::FillRect<blend_copy_none, multiply_alpha, blend_copy_none_opacity, multiply_alpha_opacity>(&m_imageData, x, y, xMax, yMax, srcPixel, m_nOpacityPainting);
		}
	}
	else
	{
		// copy
		if (IsFlagSet(bpm, BPM_CHANNEL_RGB))
		{
			if (IsFlagSet(bpm, BPM_CHANNEL_ALPHA))
				::FillRect<copy_rgb, copy_alpha, copy_rgb_opacity, copy_alpha_opacity>(&m_imageData, x, y, xMax, yMax, srcPixel, m_nOpacityPainting);
			else
			{
				::FillRect<copy_rgb, blend_copy_none, copy_rgb_opacity, blend_copy_none_opacity>(&m_imageData, x, y, xMax, yMax, srcPixel, m_nOpacityPainting);
				// FillRectCopyRGB(&m_imageData, x, y, xMax, yMax, srcPixel);
			}
		}
		else
		{
			if (IsFlagSet(bpm, BPM_CHANNEL_ALPHA))
				::FillRect<blend_copy_none, copy_alpha, blend_copy_none_opacity, copy_alpha_opacity>(&m_imageData, x, y, xMax, yMax, srcPixel, m_nOpacityPainting);
		}
	}
}

POINT CRawGraph::SetOrigin(const POINT &ptOrg)
{
	POINT	ptOrgOld = m_ptOrigin;
	m_ptOrigin = ptOrg;

	m_rcClip.left += ptOrgOld.x - ptOrg.x;
	m_rcClip.right += ptOrgOld.x - ptOrg.x;

	m_rcClip.top += ptOrgOld.y - ptOrg.y;
	m_rcClip.bottom += ptOrgOld.y - ptOrg.y;

	return ptOrgOld;
}


void CRawGraph::ClipAndMapRect(const RECT *lpRect, int &x, int &y, int &xMax, int &yMax)
{
	if (!lpRect)
	{
		x = m_rcClip.left;
		y = m_rcClip.top;
		xMax = m_rcClip.right;
		yMax = m_rcClip.bottom;
		MapPoint(x, y);
		MapPoint(xMax, yMax);
		return;
	}

	if (lpRect->left < m_rcClip.left)
		x = m_rcClip.left;
	else
		x = lpRect->left;

	if (lpRect->top < m_rcClip.top)
		y = m_rcClip.top;
	else
		y = lpRect->top;

	if (lpRect->right > m_rcClip.right)
		xMax = m_rcClip.right;
	else
		xMax = lpRect->right;
	if (xMax < x)
		xMax = x;

	if (lpRect->bottom > m_rcClip.bottom)
		yMax = m_rcClip.bottom;
	else
		yMax = lpRect->bottom;
	if (yMax < y)
		yMax = y;

	MapPoint(x, y);
	MapPoint(xMax, yMax);
}
