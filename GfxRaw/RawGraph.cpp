// RawGraph.cpp: implementation of the CRawGraph class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GfxRaw.h"
#include "RawGraph.h"
#include "ImageBuffBlt.h"

#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_pixfmt_rgb.h"
#include "agg_path_storage.h"
#include "agg_conv_stroke.h"

using namespace GfxRaw;

//////////////////////////////////////////////////////////////////////////

CRawGraph::CRawGraph()
{
	m_rawBmpFont = NULL;

	m_clrText.set(RGB(255, 255, 255));
	m_nOpacityPainting = 255;
	m_ptOrigin.x = m_ptOrigin.y = 0;

	memset(&m_rcClip, 0 , sizeof(m_rcClip));
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

	return true;
}

void CRawGraph::Line(int x1, int y1, int x2, int y2)
{
	ASSERT(m_imageData.bitCount == 32);

	typedef agg::pixfmt_rgba32	pixfmt;
	typedef agg::renderer_base<pixfmt> renderer_base;
	typedef agg::scanline_p8 scanline_type;

	agg::rendering_buffer	buf(m_imageData.buff, m_imageData.width, m_imageData.height, m_imageData.stride);
	pixfmt	pixf(buf);
	renderer_base rb(pixf);

	scanline_type sl;
	agg::rasterizer_scanline_aa<> ras;

	agg::path_storage ps;
	agg::conv_stroke<agg::path_storage> pg(ps);
	pg.width(m_pen.m_nWidth);

	ps.remove_all();
	ps.move_to(x1, y1);
	ps.line_to(x2, y2);
	ras.add_path(pg);

	agg::render_scanlines_aa_solid(ras, sl, rb, 
		agg::rgba(m_pen.m_clrPen.r() / (double)255, m_pen.m_clrPen.g() / (double)255, m_pen.m_clrPen.b() / (double)255, m_pen.m_clrPen.a() / (double)255));
}

void CRawGraph::Rectangle(int x, int y, int width, int height)
{
	ASSERT(m_imageData.bitCount == 32);

	typedef agg::pixfmt_rgba32	pixfmt;
	typedef agg::renderer_base<pixfmt> renderer_base;
	typedef agg::scanline_p8 scanline_type;

	agg::rendering_buffer	buf(m_imageData.buff, m_imageData.width, m_imageData.height, m_imageData.stride);
	pixfmt	pixf(buf);
	renderer_base rb(pixf);

	scanline_type sl;
	agg::rasterizer_scanline_aa<> ras;

	agg::path_storage ps;
	agg::conv_stroke<agg::path_storage> pg(ps);
	pg.width(m_pen.m_nWidth);

	ps.remove_all();
	ps.move_to(x, y);
	ps.line_to(x + width, y);
	ps.line_to(x + width, y + height);
	ps.line_to(x, y + height);
	ps.move_to(x, y);
	ras.add_path(pg);

	agg::render_scanlines_aa_solid(ras, sl, rb, 
		agg::rgba(m_pen.m_clrPen.r() / (double)255, m_pen.m_clrPen.g() / (double)255, m_pen.m_clrPen.b() / (double)255, m_pen.m_clrPen.a() / (double)255));
}

void CRawGraph::SetPen(CRawPen &pen)
{
	m_pen = pen;
}

bool CRawGraph::GetClipBoundBox(CRect &rc)
{
	rc = m_rcClip;
	return true;
}

bool CRawGraph::SetClipBoundBox(const CRect &rc)
{
	m_vClipOrg.push_back(m_rcClip);

	m_rcClip = rc;
	if (MapX(m_rcClip.left) < 0)
		m_rcClip.left = RevertMapX(0);
	if (MapY(m_rcClip.top) < 0)
		m_rcClip.top = RevertMapY(0);
	if (MapX(m_rcClip.right) > Width())
		m_rcClip.right = RevertMapX(Width());
	if (MapY(m_rcClip.bottom) > Height())
		m_rcClip.bottom = RevertMapY(Height());

	return true;
}

void CRawGraph::RecoverClipBox()
{
	if (!m_vClipOrg.empty())
	{
		m_rcClip = m_vClipOrg.back();

		if (MapX(m_rcClip.left) < 0)
			m_rcClip.left = RevertMapX(0);
		if (MapY(m_rcClip.top) < 0)
			m_rcClip.top = RevertMapY(0);
		if (MapX(m_rcClip.right) > Width())
			m_rcClip.right = RevertMapX(Width());
		if (MapY(m_rcClip.bottom) > Height())
			m_rcClip.bottom = RevertMapY(Height());

		m_vClipOrg.pop_back();
	}
	else
		ASSERT(0 && "m_vClipOrg.empty()");
}

bool CRawGraph::TextOut(int x, int y, LPCTSTR szText, int nLen)
{
	if (m_rawBmpFont)
		return m_rawBmpFont->TextOut(this, MapX(x), MapY(y), m_clrText, szText, nLen);
	return false;
}

bool CRawGraph::DrawTextClip(LPCTSTR szText, int nLen, LPRECT lpRect, int xLeftClipOffset)
{
	if (m_rawBmpFont)
	{
		return m_rawBmpFont->DrawText(this, MapX(lpRect->left), MapY(lpRect->top), MapX(lpRect->right - lpRect->left), MapX(xLeftClipOffset), m_clrText, szText, nLen);
	}
	return false;
}

bool CRawGraph::TextOutOutlined(int x, int y, LPCTSTR szText, int nLen, const MLColor &clrText, const MLColor &clrBorder)
{
	if (m_rawBmpFont)
		return m_rawBmpFont->OutlinedTextOut(this, MapX(x), MapY(y), clrText, clrBorder, szText, nLen);
	return false;
}

bool CRawGraph::DrawTextClipOutlined(LPCTSTR szText, int nLen, LPRECT lpRect, const MLColor &clrText, const MLColor &clrBorder, int xLeftClipOffset)
{
	if (m_rawBmpFont)
		return m_rawBmpFont->OutlinedDrawText(this, MapX(lpRect->left), MapY(lpRect->top), MapX(lpRect->right - lpRect->left), MapX(xLeftClipOffset), clrText, clrBorder, szText, nLen);
	return false;
}


bool CRawGraph::DrawTextOutlined(LPCTSTR szText, int nLen, LPRECT lpRect, const MLColor &clrText, const MLColor &clrBorder, UINT uFormat)
{
	RECT	rc = *lpRect;
	MapRect(rc);

	if (m_rawBmpFont)
		return m_rawBmpFont->OutlinedDrawTextEx(this, &rc, clrText, clrBorder, szText, nLen, uFormat);

	return false;
}


bool CRawGraph::DrawText(LPCTSTR szText, int nLen, LPRECT lpRect, UINT uFormat)
{
	RECT	rc = *lpRect;
	MapRect(rc);

	if (m_rawBmpFont)
		return m_rawBmpFont->DrawTextEx(this, &rc, m_clrText, szText, nLen, uFormat);

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
	ASSERT(m_imageData.bitCount == 32);

	int			x, y, xMax, yMax;
	byte		*p, *pEnd, *pRow;
	int			nStrideModify;

	ClipAndMapRect(lpRect, x, y, xMax, yMax);
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
	ASSERT(m_imageData.bitCount == 32);

	int			x, y, xMax, yMax;
	byte		*p, *pEnd, *pRow;
	int			nStrideModify;

	ClipAndMapRect(lpRect, x, y, xMax, yMax);
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
	ASSERT(m_imageData.bitCount == 32);

	int			x, y, xMax, yMax;
	byte		*p, *pEnd, *pRow;
	float		fStep, fStart;
	int			nStrideModify;

	ClipAndMapRect(lpRect, x, y, xMax, yMax);
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

	fStart += fStep * (y - lpRect->top);

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

	int			x, y, xMax, yMax;
	byte		*p, *pEnd, *pRow;
	float		fStep, fStart;
	int			r, g, b, a;
	int			nStrideModify;

	r = clrBg.r();
	g = clrBg.g();
	b = clrBg.b();
	a = clrBg.getAlpha();

	ClipAndMapRect(lpRect, x, y, xMax, yMax);
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

	fStart += fStep * (y - lpRect->top);

	pRow = m_imageData.PixPtr(x, y);
	for (; y < yMax; y++)
	{
		pEnd = pRow + nStrideModify;
		for (p = pRow; p < pEnd; p += PIX_SIZE)
		{
			// a * f + b * (1 - f) = (a - b) * f + b
			p[PIX_B] = (byte)((p[PIX_B] - b) * fStart + b);
			p[PIX_G] = (byte)((p[PIX_G] - g) * fStart + g);
			p[PIX_R] = (byte)((p[PIX_R] - r) * fStart + r);
			p[PIX_A] = (byte)((p[PIX_A] - a) * fStart + a);
		}
		pRow += m_imageData.stride;
		fStart += fStep;
	}
}


void CRawGraph::HorzAlphaFadeOut(const RECT *lpRect, bool bLeft)
{
	ASSERT(m_imageData.bitCount == 32);

	int			x, y, xMax, yMax;
	byte		*p, *pEnd, *pRow;
	float		fStep, fStart, f;
	int			nStrideModify;

	ClipAndMapRect(lpRect, x, y, xMax, yMax);
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

	fStart += fStep * (x - lpRect->left);

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

	int			x, y, xMax, yMax;
	byte		*p, *pEnd, *pRow;
	float		fStep, fStart, f;
	BYTE		r, g, b, a;
	int			nStrideModify;

	r = clrBg.r();
	g = clrBg.g();
	b = clrBg.b();
	a = clrBg.getAlpha();

	ClipAndMapRect(lpRect, x, y, xMax, yMax);
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

	fStart += fStep * (x - lpRect->left);

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


void CRawGraph::Clear(byte r, byte g, byte b, byte alpha, const RECT *lpRect)
{
	ASSERT(m_imageData.bitCount == 32);

	int			x, y, xMax, yMax;
	byte		*p, *pEnd, *pRow;
	int			nStrideModify;

	ClipAndMapRect(lpRect, x, y, xMax, yMax);
	nStrideModify = (xMax - x) * PIX_SIZE;

	pRow = m_imageData.PixPtr(x, y);
	for (; y < yMax; y++)
	{
		pEnd = pRow + nStrideModify;
		for (p = pRow; p < pEnd; p += PIX_SIZE)
		{
			p[PIX_B] = b;
			p[PIX_G] = g;
			p[PIX_R] = r;
			p[PIX_A] = alpha;
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

	nStrideModify = (xMax - x) * PIX_SIZE;

	pRow = pImage->PixPtr(x, y);
	pRowMask = pImageMask->PixPtr(xMask, yMask);
	for (; y < yMax; y++)
	{
		pEnd = pRow + nStrideModify;
		pMask = pRowMask;
		for (p = pRow; p < pEnd; p += PIX_SIZE)
		{
			if (pMask[3] != 0)
			{
				_PixRGBBlender::blend(p, srcPixel, pMask);
				_PixAlphaBlender::blend(p, srcPixel, pMask);
			}
			pMask += PIX_SIZE;
		}
		pRow += pImage->stride;
		pRowMask += pImageMask->stride;
	}
}


void CRawGraph::FillRect(const RECT *lpRect, PRAW_IMAGE_DATA pImgMask, const RECT *rcMask, const MLColor &clrFill, BlendPixMode bpm)
{
	int			x, y, xMax, yMax;
	uint8		srcPixel[4];

	srcPixel[PIX_R] = clrFill.r();
	srcPixel[PIX_G] = clrFill.g();
	srcPixel[PIX_B] = clrFill.b();
	srcPixel[PIX_A] = clrFill.getAlpha();

	ASSERT(pImgMask);
	if (!pImgMask)
		return;

	if (pImgMask->pixFormat != PF_RGBA32)
		return;

	ClipAndMapRect(lpRect, x, y, xMax, yMax);

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


template<class _PixRGBBlender, class _PixAlphaBlender>
void FillRect(PRAW_IMAGE_DATA pImage, int x, int y, int xMax, int yMax, uint8 *srcPixel)
{
	byte		*p, *pEnd, *pRow;
	int			nStrideModify;

	nStrideModify = (xMax - x) * PIX_SIZE;

	pRow = pImage->PixPtr(x, y);
	for (; y < yMax; y++)
	{
		pEnd = pRow + nStrideModify;
		for (p = pRow; p < pEnd; p += PIX_SIZE)
		{
			_PixRGBBlender::blend(p, srcPixel);
			_PixAlphaBlender::blend(p, srcPixel);
		}
		pRow += pImage->stride;
	}
}


void CRawGraph::FillRect(const RECT *lpRect, const MLColor &clrFill, BlendPixMode bpm)
{
	ASSERT(m_imageData.bitCount == 32);

	int			x, y, xMax, yMax;
	uint8		srcPixel[4];

	srcPixel[PIX_R] = clrFill.r();
	srcPixel[PIX_G] = clrFill.g();
	srcPixel[PIX_B] = clrFill.b();
	srcPixel[PIX_A] = clrFill.getAlpha();

	ClipAndMapRect(lpRect, x, y, xMax, yMax);

	if (IsFlagSet(bpm, BPM_OP_BLEND))
	{
		// blend
		if (IsFlagSet(bpm, BPM_CHANNEL_RGB))
		{
			if (IsFlagSet(bpm, BPM_CHANNEL_ALPHA))
				::FillRect<blend_rgb, blend_alpha>(&m_imageData, x, y, xMax, yMax, srcPixel);
			else
				::FillRect<blend_rgb, blend_copy_none>(&m_imageData, x, y, xMax, yMax, srcPixel);
		}
		else
		{
			if (IsFlagSet(bpm, BPM_CHANNEL_ALPHA))
				::FillRect<blend_copy_none, blend_alpha>(&m_imageData, x, y, xMax, yMax, srcPixel);
		}
	}
	else if (IsFlagSet(bpm, BPM_OP_MULTIPLY))
	{
		// Multiply
		if (IsFlagSet(bpm, BPM_CHANNEL_RGB))
		{
			if (IsFlagSet(bpm, BPM_CHANNEL_ALPHA))
				::FillRect<multiply_rgb, multiply_alpha>(&m_imageData, x, y, xMax, yMax, srcPixel);
			else
				::FillRect<multiply_rgb, blend_copy_none>(&m_imageData, x, y, xMax, yMax, srcPixel);
		}
		else
		{
			if (IsFlagSet(bpm, BPM_CHANNEL_ALPHA))
				::FillRect<blend_copy_none, multiply_alpha>(&m_imageData, x, y, xMax, yMax, srcPixel);
		}
	}
	else
	{
		// copy
		if (IsFlagSet(bpm, BPM_CHANNEL_RGB))
		{
			if (IsFlagSet(bpm, BPM_CHANNEL_ALPHA))
				::FillRect<copy_rgb, copy_alpha>(&m_imageData, x, y, xMax, yMax, srcPixel);
			else
				::FillRect<copy_rgb, blend_copy_none>(&m_imageData, x, y, xMax, yMax, srcPixel);
		}
		else
		{
			if (IsFlagSet(bpm, BPM_CHANNEL_ALPHA))
				::FillRect<blend_copy_none, copy_alpha>(&m_imageData, x, y, xMax, yMax, srcPixel);
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

	if (lpRect->bottom > m_rcClip.bottom)
		yMax = m_rcClip.bottom;
	else
		yMax = lpRect->bottom;

	MapPoint(x, y);
	MapPoint(xMax, yMax);
}
