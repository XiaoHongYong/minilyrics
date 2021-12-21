// RawBmpFont.cpp: implementation of the CRawBmpFont class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "../gfx/win32/RawGraph.h"
#include "RawBmpFontEx.h"
#include "agg_pixfmt_gray.h"
#include "agg_blur.h"

#define PATTERN_COLOR_RANGE		100

inline byte RGBToGray(int r, int g, int b)
{
	// 5 = 四舍五入
	return (r * 3 + g * 6 + b + 5) / 10;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRawBmpFont::CRawBmpFont()
{
	m_rcClip.SetRect(0, 0, 0, 0);
	m_bGdiplusAntialias = false;
	m_pmemGraph = NULL;
	m_bThickOutline = true;

	m_overlayMode = OM_COLOR;
	m_imgPattern1 = NULL;
	m_imgPattern2 = NULL;
	m_nAlphaPattern1 = 255;
	m_nAlphaPattern2 = 255;

	m_shadowMode = SM_SHADOW;
}

CRawBmpFont::~CRawBmpFont()
{
	Destroy();
}

bool CRawBmpFont::Create(LPCTSTR szFaceNameLatin9, LPCTSTR szFaceNameOthers, int nHeight, int nWeight, int nItalic, bool bUnderline)
{
	if (_tcsicmp(GetName(), szFaceNameLatin9) == 0 
		&& _tcsicmp(GetNameOthers(), szFaceNameOthers) == 0 
		&& GetHeight() == nHeight
		&& GetWeight() == nWeight
		&& GetItalic() == nItalic
		&& IsUnderline() == bUnderline)
	{
		// same font.
		return true;
	}

	Destroy();

	return CMLFont::Create(szFaceNameLatin9, szFaceNameOthers, nHeight, nWeight, nItalic, bUnderline);
}

bool CRawBmpFont::CreateFrom(const CMLFont &font)
{
	if (_tcsicmp(GetName(), font.GetName()) == 0 
		&& _tcsicmp(GetNameOthers(), font.GetNameOthers()) == 0 
		&& GetHeight() == font.GetHeight()
		&& GetWeight() == font.GetWeight()
		&& GetItalic() == font.GetItalic()
		&& IsUnderline() == font.IsUnderline())
	{
		// same font.
		return true;
	}

	Destroy();

	return CMLFont::CreateFrom(font);
}

void CRawBmpFont::Destroy()
{
	if (m_pmemGraph)
	{
		delete m_pmemGraph;
		m_pmemGraph = NULL;

		m_memGraph.Destroy();

		DestroyGlyphMap();
	}
}

void CRawBmpFont::EnableGdiplusAntialias(bool bGdiplusAntialias)
{
	if (m_bGdiplusAntialias == bGdiplusAntialias)
		return;

	m_bGdiplusAntialias = bGdiplusAntialias;

	if (m_pmemGraph)
	{
		delete m_pmemGraph;
		m_pmemGraph = NULL;

		if (m_bGdiplusAntialias)
		{
			CGdiplusGraphics	*pmemGraph;

			pmemGraph = new CGdiplusGraphics;
			pmemGraph->Create(m_memGraph.GetHandle());
			m_pmemGraph = pmemGraph;
		}
		else
		{
			CGdiGraphics	*pmemGraph;

			pmemGraph = new CGdiGraphics;
			pmemGraph->Create(m_memGraph.GetHandle());
			m_pmemGraph = pmemGraph;
		}

		m_pmemGraph->SetFont(this);
		m_pmemGraph->SetTextColor(MLColor(RGB(255, 255, 255)));

		DestroyGlyphMap();
	}
}

void CRawBmpFont::SetThickOutline(bool bThick)
{
	if (m_bThickOutline == bThick)
		return;

	m_bThickOutline = bThick;
	if (m_shadowMode == SM_OUTLINE)
		DestroyGlyphMap();
}

bool CRawBmpFont::TextOut(CRawGraph *pGraph, int x, int y, MLColor &clrText, LPCTSTR szText, int nLen)
{
	return DrawText(pGraph, x, y, pGraph->Width() - x, 0, clrText, szText, nLen);
}

// graph is 32 bpp, glyph is 32 bpp
static inline void DrawGlyphRGBA32(agg::rendering_buffer &graph, int xDst, int yDst, agg::rendering_buffer &glyph, int xSrc, int ySrc, int xSrcEnd, int ySrcEnd, 
					  int nAlphaGlyph, MLColor &clrGlyph)
{
	byte		*pDst, *pRowDst, *pSrc, *pRowSrc;
	int			x, y;
	byte		clr[3];

	clr[0] = clrGlyph.b();
	clr[1] = clrGlyph.g();
	clr[2] = clrGlyph.r();

	pRowDst = graph.row_ptr(yDst);
	pRowDst += xDst * 4;

	pRowSrc = glyph.row_ptr(ySrc);
	pRowSrc += xSrc * G_PIX_SIZE;

	// Fill with color
	for (y = ySrc; y < ySrcEnd; y++)
	{
		pDst = pRowDst;
		pSrc = pRowSrc;
		for (x = xSrc; x < xSrcEnd; x++)
		{
			if (pSrc[3] != 0)
			{
				int		a;
				for (int i = 0; i < 3; i++)
				{
					a = pSrc[i] * nAlphaGlyph / 255;
					pDst[i] = (uint8)(((clr[i] - pDst[i]) * a + (pDst[i] << 8)) >> 8);
//					pDst[i] = (uint8)(((clr[i] - pDst[i]) * a + (pDst[i] * 255)) / 255);
					// pDst[i] = (uint8)((clr[i] * (a) + pDst[i] * (255 - a)) / 255);
					// pDst[i] = 255 - pSrc[2-i];
				}
				a = pSrc[G_A] * nAlphaGlyph / 255;
				pDst[G_A] = (uint8)((a + pDst[G_A]) - ((a * pDst[G_A] + 0xFF) >> 8));
// 				int		a = pSrc[0] * nAlphaGlyph / 255;
// 				pDst[0] = (uint8)(((b - pDst[0]) * a + (pDst[0] << 8)) >> 8);
// 				pDst[1] = (uint8)(((g - pDst[1]) * a + (pDst[1] << 8)) >> 8);
// 				pDst[2] = (uint8)(((r - pDst[2]) * a + (pDst[2] << 8)) >> 8);
// 				pDst[3] = (uint8)((a + pDst[3]) - ((a * pDst[3] + 0xFF) >> 8));
			}
			pDst += 4;
			pSrc += G_PIX_SIZE;
		}
		pRowDst += graph.stride();
		pRowSrc += glyph.stride();
	}
}

// graph is 32 bpp, glyph is 32 bpp
// pattern is 24 bpp.
inline void DrawGlyphRGBA32(agg::rendering_buffer &graph, int xDst, int yDst, agg::rendering_buffer &glyph, int xSrc, int ySrc, int xSrcEnd, int ySrcEnd,
					  int nAlphaGlyph, CRawImage *pattern, int &xPtStart)
{
	byte		*pDst, *pRowDst, *pSrc, *pRowSrc;
	byte		*pPt, *pRowPt;
	PRAW_IMAGE_DATA	imgPattern;
	int			x, y;
	int			yPtStart = 0;
	int			xPtStartNew = xPtStart;

	pRowDst = graph.row_ptr(yDst);
	pRowDst += xDst * 4;

	pRowSrc = glyph.row_ptr(ySrc);
	pRowSrc += xSrc * G_PIX_SIZE;

	imgPattern = pattern->GetHandle();
	ASSERT(imgPattern->bitCount == 24);

	yPtStart += ySrc;
	if (yPtStart >= imgPattern->height)
		yPtStart = 0;
	// xPtStart += xSrc;
	if (xPtStart >= imgPattern->width)
		xPtStart = 0;

	pRowPt = imgPattern->RowPtr(yPtStart);
	pRowPt += xPtStart * 3;

	// Fill with color
	for (y = ySrc; y < ySrcEnd; y++)
	{
		pDst = pRowDst;
		pSrc = pRowSrc;
		pPt = pRowPt;
		xPtStartNew = xPtStart;
		for (x = xSrc; x < xSrcEnd; x++)
		{
			if (pSrc[3] != 0)
			{
				int		a;
				for (int i = 0; i < 3; i++)
				{
					a = pSrc[i] * nAlphaGlyph / 255;
					pDst[i] = (uint8)(((pPt[i] - pDst[i]) * a + (pDst[i] << 8)) >> 8);
				}
				a = pSrc[G_A] * nAlphaGlyph / 255;
				pDst[G_A] = (uint8)((a + pDst[G_A]) - ((a * pDst[G_A] + 0xFF) >> 8));
// 				int		a = pSrc[0] * nAlphaGlyph / 255;
// 				pDst[0] = (uint8)(((pPt[0] - pDst[0]) * a + (pDst[0] << 8)) >> 8);
// 				pDst[1] = (uint8)(((pPt[1] - pDst[1]) * a + (pDst[1] << 8)) >> 8);
// 				pDst[2] = (uint8)(((pPt[2] - pDst[2]) * a + (pDst[2] << 8)) >> 8);
// 				pDst[3] = (uint8)((a + pDst[3]) - ((a * pDst[3] + 0xFF) >> 8));
			}
			pDst += 4;
			pSrc += G_PIX_SIZE;

			xPtStartNew++;
			if (xPtStartNew >= imgPattern->width)
			{
				xPtStartNew = 0;
				pPt = imgPattern->RowPtr(yPtStart);
			}
			else
				pPt += 3;
		}
		pRowDst += graph.stride();
		pRowSrc += glyph.stride();
		yPtStart++;
		if (yPtStart >= imgPattern->height)
		{
			yPtStart = 0;
			pRowPt = imgPattern->RowPtr(0);
			pRowPt += xPtStart * 3;
		}
		else
			pRowPt += imgPattern->stride;
	}

	xPtStart = xPtStartNew;
}

// graph is 32 bpp, glyph is 32 bpp
// pattern is 24 bpp.
inline void DrawGlyphRGBA32(agg::rendering_buffer &graph, int xDst, int yDst, agg::rendering_buffer &glyph, int xSrc, int ySrc, int xSrcEnd, int ySrcEnd,
					  int nAlphaGlyph, CRawImage *pattern, int &xPtStart, int nAlphaPattern, MLColor &clrPt, int nAlphaClrPt)
{
	byte		*pDst, *pRowDst, *pSrc, *pRowSrc;
	byte		*pPt, *pRowPt;
	PRAW_IMAGE_DATA	imgPattern;
	int			x, y;
	int			yPtStart = 0;
	int			xPtStartNew = xPtStart;
	byte		pclrPt[3];

	pclrPt[0] = clrPt.b();
	pclrPt[1] = clrPt.g();
	pclrPt[2] = clrPt.r();

	pRowDst = graph.row_ptr(yDst);
	pRowDst += xDst * 4;

	pRowSrc = glyph.row_ptr(ySrc);
	pRowSrc += xSrc * G_PIX_SIZE;

	imgPattern = pattern->GetHandle();
	ASSERT(imgPattern->bitCount == 24);

	yPtStart += ySrc;
	if (yPtStart >= imgPattern->height)
		yPtStart = 0;
	// xPtStart += xSrc;
	if (xPtStart >= imgPattern->width)
		xPtStart = 0;

	pRowPt = imgPattern->RowPtr(yPtStart);
	pRowPt += xPtStart * 3;

	// Fill with color
	for (y = ySrc; y < ySrcEnd; y++)
	{
		pDst = pRowDst;
		pSrc = pRowSrc;
		pPt = pRowPt;
		xPtStartNew = xPtStart;
		for (x = xSrc; x < xSrcEnd; x++)
		{
			if (pSrc[3] != 0)
			{
				int		a, c;
				for (int i = 0; i < 3; i++)
				{
					a = pSrc[i] * nAlphaGlyph / 255;
					c = (pPt[i] * nAlphaPattern + pclrPt[i] * (255 - nAlphaPattern)) / 255;
					pDst[i] = (uint8)(((c - pDst[i]) * a + (pDst[i] << 8)) >> 8);
				}
				a = pSrc[3] * nAlphaGlyph / 255;
				pDst[3] = (uint8)((a + pDst[3]) - ((a * pDst[3] + 0xFF) >> 8));
// 
// 				int	r, g, b;
// 				b = (pPt[0] * nAlphaPattern + bClrPt) / 255;
// 				g = (pPt[1] * nAlphaPattern + gClrPt) / 255;
// 				r = (pPt[2] * nAlphaPattern + rClrPt) / 255;
// 				pDst[0] = (uint8)(((b - pDst[0]) * a + (pDst[0] << 8)) >> 8);
// 				pDst[1] = (uint8)(((g - pDst[1]) * a + (pDst[1] << 8)) >> 8);
// 				pDst[2] = (uint8)(((r - pDst[2]) * a + (pDst[2] << 8)) >> 8);
// 				pDst[3] = (uint8)((a + pDst[3]) - ((a * pDst[3] + 0xFF) >> 8));
			}
			pDst += 4;
			pSrc += G_PIX_SIZE;

			xPtStartNew++;
			if (xPtStartNew >= imgPattern->width)
			{
				xPtStartNew = 0;
				pPt = imgPattern->RowPtr(yPtStart);
			}
			else
				pPt += 3;
		}
		pRowDst += graph.stride();
		pRowSrc += glyph.stride();
		yPtStart++;
		if (yPtStart >= imgPattern->height)
		{
			yPtStart = 0;
			pRowPt = imgPattern->RowPtr(0);
		}
		else
			pRowPt += imgPattern->stride;
	}

	xPtStart = xPtStartNew;
}

// graph is 32 bpp, glyph is 32 bpp
// pattern is 24 bpp.
inline void DrawGlyphRGBA32(agg::rendering_buffer &graph, int xDst, int yDst, agg::rendering_buffer &glyph, int xSrc, int ySrc, int xSrcEnd, int ySrcEnd,
					  int nAlphaGlyph, CRawImage *pattern1, int &xPt1Start, int nAlphaPt1, CRawImage *pattern2, int &xPt2Start, int nAlphaPt2)
{
	byte		*pDst, *pRowDst, *pSrc, *pRowSrc;
	byte		*pPt1, *pRowPt1;
	byte		*pPt2, *pRowPt2;
	PRAW_IMAGE_DATA	imgPattern1, imgPattern2;
	int			x, y;
	int			yPt1Start = 0, yPt2Start = 0;
	int			xPt1StartNew = xPt1Start;
	int			xPt2StartNew = xPt2Start;

	pRowDst = graph.row_ptr(yDst);
	pRowDst += xDst * 4;

	pRowSrc = glyph.row_ptr(ySrc);
	pRowSrc += xSrc * G_PIX_SIZE;

	// pattern 1
	imgPattern1 = pattern1->GetHandle();
	ASSERT(imgPattern1->bitCount == 24);
	yPt1Start += ySrc;
	if (yPt1Start >= imgPattern1->height)
		yPt1Start = 0;
	// xPt1Start += xSrc;
	if (xPt1Start >= imgPattern1->width)
		xPt1Start = 0;

	pRowPt1 = imgPattern1->RowPtr(yPt1Start);
	pRowPt1 += xPt1Start * 3;

	// pattern 2
	imgPattern2 = pattern2->GetHandle();
	ASSERT(imgPattern2->bitCount == 24);
	yPt2Start += ySrc;
	if (yPt2Start >= imgPattern2->height)
		yPt2Start = 0;
	// xPt2Start += xSrc;
	if (xPt2Start >= imgPattern2->width)
		xPt2Start = 0;

	pRowPt2 = imgPattern2->RowPtr(yPt2Start);
	pRowPt2 += xPt2Start * 3;

	// Fill with color
	for (y = ySrc; y < ySrcEnd; y++)
	{
		pDst = pRowDst;
		pSrc = pRowSrc;
		pPt1 = pRowPt1;
		pPt2 = pRowPt2;
		xPt1StartNew = xPt1Start;
		xPt2StartNew = xPt2Start;
		for (x = xSrc; x < xSrcEnd; x++)
		{
			if (pSrc[3] != 0)
			{
				int		a, c;
				for (int i = 0; i < 3; i++)
				{
					a = pSrc[i] * nAlphaGlyph / 255;
					c = (pPt1[i] * nAlphaPt1 + pPt2[i] * nAlphaPt2) / 255;
					pDst[i] = (uint8)(((c - pDst[i]) * a + (pDst[i] << 8)) >> 8);
				}
				a = pSrc[G_A] * nAlphaGlyph / 255;
				pDst[G_A] = (uint8)((a + pDst[G_A]) - ((a * pDst[G_A] + 0xFF) >> 8));
// 				int		a = pSrc[0] * nAlphaGlyph / 255;
// 				int		r, g, b;
// 				b = (pPt1[0] * nAlphaPt1 + pPt2[0] * nAlphaPt2) / 255;
// 				g = (pPt1[1] * nAlphaPt1 + pPt2[1] * nAlphaPt2) / 255;
// 				r = (pPt1[2] * nAlphaPt1 + pPt2[2] * nAlphaPt2) / 255;
// 				pDst[0] = (uint8)(((b - pDst[0]) * a + (pDst[0] << 8)) >> 8);
// 				pDst[1] = (uint8)(((g - pDst[1]) * a + (pDst[1] << 8)) >> 8);
// 				pDst[2] = (uint8)(((r - pDst[2]) * a + (pDst[2] << 8)) >> 8);
// 				pDst[3] = (uint8)((a + pDst[3]) - ((a * pDst[3] + 0xFF) >> 8));
			}
			pDst += 4;
			pSrc += G_PIX_SIZE;

			xPt1StartNew++;
			if (xPt1StartNew >= imgPattern1->width)
			{
				xPt1StartNew = 0;
				pPt1 = imgPattern1->RowPtr(yPt1Start);
			}
			else
				pPt1 += 3;

			xPt2StartNew++;
			if (xPt2StartNew >= imgPattern2->width)
			{
				xPt2StartNew = 0;
				pPt2 = imgPattern2->RowPtr(yPt2Start);
			}
			else
				pPt2 += 3;
		}
		pRowDst += graph.stride();
		pRowSrc += glyph.stride();

		yPt1Start++;
		if (yPt1Start >= imgPattern1->height)
		{
			yPt1Start = 0;
			pRowPt1 = imgPattern1->RowPtr(0);
		}
		else
			pRowPt1 += imgPattern1->stride;

		yPt2Start++;
		if (yPt2Start >= imgPattern2->height)
		{
			yPt2Start = 0;
			pRowPt2 = imgPattern2->RowPtr(0);
		}
		else
			pRowPt2 += imgPattern2->stride;
	}

	xPt1Start = xPt1StartNew;
	xPt2Start = xPt2StartNew;
}

bool CRawBmpFont::DrawTextEx(CRawGraph *pGraph, int x, int y, int width, MLColor &clrText, LPCTSTR szText, int nLen, UINT uFormat)
{
	if (IsFlagSet(uFormat, DT_END_ELLIPSIS))
	{
		int		w = 0;
		Glyph	*glyph;
		int		i;

		for (i = 0; i < nLen; i++)
		{
			glyph = GetGlyph((unsigned short)szText[i]);
			if (!glyph)
				continue;

			w += glyph->nWidth;
			if (w > width)
				break;
		}

		if (i != nLen)
		{
			tstring		str;
			int			wMax;
			nLen = i;

			glyph = GetGlyph((unsigned short)'.');
			if (!glyph)
				return false;
			wMax = width - glyph->nWidth * 3;

			w = 0;
			for (i = 0; i < nLen; i++)
			{
				glyph = GetGlyph((unsigned short)szText[i]);
				if (!glyph)
					continue;

				w += glyph->nWidth;
				if (w > wMax)
					break;

				str += szText[i];
			}
			str += _T("...");
			return DrawText(pGraph, x, y, width, 0, clrText, str.c_str(), str.size());
		}
	}

	return DrawText(pGraph, x, y, width, 0, clrText, szText, nLen);
}

bool CRawBmpFont::DrawText(CRawGraph *pGraph, int x, int y, int width, int xLeftClipOffset, MLColor &clrText, LPCTSTR szText, int nLen)
{
	byte		r = clrText.r(), g = clrText.g(), b = clrText.b();
	byte		textAlpha = pGraph->GetTextAlpha();
	int			yyStart, yyEnd;
	CRect		rcClip;
	agg::rendering_buffer bufGraph(pGraph->GetBuff(), pGraph->Width(), pGraph->Height(), -pGraph->Width() * 4);
	int			xPattern1 = 0, xPattern2 = 0;

	ASSERT(m_pmemGraph);

	pGraph->GetClipBoundBox(rcClip);

	if (x + xLeftClipOffset > rcClip.right)
		return true;
	if (x + xLeftClipOffset > rcClip.left)
		rcClip.left = x + xLeftClipOffset;

	if (y >= rcClip.bottom)
		return true;
	if (y + GetHeightBitmap() <= rcClip.top)
		return true;

	if (x + width < rcClip.right)
		rcClip.right = x + width;

	if (y < rcClip.top)
		yyStart = rcClip.top - y;
	else
		yyStart = 0;
	if (y + GetHeightBitmap() > rcClip.bottom)
		yyEnd = rcClip.bottom - y;
	else
		yyEnd = GetHeightBitmap();

	for (int i = 0; i < nLen; i++)
	{
		int			xxStart, xxEnd;
		// byte		*p, *pRowGlyph, *graphData, *pRowGraph;
		// int		xx, yy;
		Glyph		*glyph = GetGlyph((unsigned short)szText[i]);
		if (!glyph)
			continue;

		if (x >= rcClip.right)
			return true;

		if (x + glyph->nWidthBitmap <= rcClip.left)
		{
			x += glyph->nWidthBitmap;
			if (m_imgPattern1)
				xPattern1 = (xPattern1 + glyph->nWidthBitmap) % m_imgPattern1->width();
			if (m_imgPattern2)
				xPattern2 = (xPattern2 + glyph->nWidthBitmap) % m_imgPattern2->width();
			continue;
		}

		if (x < rcClip.left)
			xxStart = rcClip.left - x;
		else
			xxStart = 0;
		if (x + glyph->nWidthBitmap > rcClip.right)
			xxEnd = rcClip.right - x;
		else
			xxEnd = glyph->nWidthBitmap;

		if (m_imgPattern1)
			xPattern1 = (xPattern1 + xxStart) % m_imgPattern1->width();
		if (m_imgPattern2)
			xPattern2 = (xPattern2 + xxStart) % m_imgPattern2->width();

		agg::rendering_buffer bufGlyph(glyph->bitmap, glyph->nWidthBitmap, GetHeightBitmap(), glyph->WidthBytes());

		if (m_overlayMode == OM_PATTERN)
			DrawGlyphRGBA32(bufGraph, x + xxStart, y + yyStart, bufGlyph, xxStart, yyStart, xxEnd, yyEnd, textAlpha, m_imgPattern1, xPattern1);
		else if (m_overlayMode == OM_COLOR)
			DrawGlyphRGBA32(bufGraph, x + xxStart, y + yyStart, bufGlyph, xxStart, yyStart, xxEnd, yyEnd, textAlpha, clrText);
		else if (m_overlayMode == OM_DUAL_PATTERN)
			DrawGlyphRGBA32(bufGraph, x + xxStart, y + yyStart, bufGlyph, xxStart, yyStart, xxEnd, yyEnd, textAlpha, m_imgPattern1, xPattern1, m_nAlphaPattern1, m_imgPattern2, xPattern2, m_nAlphaPattern2);
		else if (m_overlayMode == OM_PATTERN_COLOR)
			DrawGlyphRGBA32(bufGraph, x + xxStart, y + yyStart, bufGlyph, xxStart, yyStart, xxEnd, yyEnd, textAlpha, m_imgPattern1, xPattern1, m_nAlphaPattern1, m_clrPattern2, m_nAlphaPattern2);

/*		// pRowGraph is the target start pixel to draw.
		pRowGraph = pGraph->GetBuff() + nWidthBytesGraph * (pGraph->Height() - 1);
		pRowGraph -= nWidthBytesGraph * (y + yyStart);		// y offset
		pRowGraph += x * 4;						// x offset

		// pRowGlyph is the source start pixel
		pRowGlyph = glyph->bitmap + glyph->nWidthBitmap * (GetHeightBitmap() - 1 - yyStart);
		pRowGlyph += xxStart;
		if (!m_bPatternOverlay)
		{
			// Fill with color
			for (yy = yyStart; yy < yyEnd; yy++)
			{
				p = pRowGlyph;
				graphData = pRowGraph;
				for (xx = xxStart; xx < xxEnd; xx++)
				{
					if (p[0] != 0)
					{
						int		a = p[0] * textAlpha / 255;
						graphData[0] = (uint8)(((b - graphData[0]) * a + (graphData[0] << 8)) >> 8);
						graphData[1] = (uint8)(((g - graphData[1]) * a + (graphData[1] << 8)) >> 8);
						graphData[2] = (uint8)(((r - graphData[2]) * a + (graphData[2] << 8)) >> 8);
						graphData[3] = (uint8)((a + graphData[3]) - ((a * graphData[3] + 0xFF) >> 8));
					}
					graphData += 4;
					p++;
				}
				pRowGlyph -= glyph->nWidthBitmap;
				pRowGraph -= nWidthBytesGraph;
			}
		}
		else
		{
			// Fill with pattern image
			PRAW_IMAGE_DATA		imgPattern = m_pattern.GetHandle();
			byte				*pRowPT, *pixlPT;

			pRowPT = imgPattern->RowPtr(yyStart);
			pRowPT += xxStart * 3;

			for (yy = yyStart; yy < yyEnd; yy++)
			{
				p = pRowGlyph;
				pixlPT = pRowPT;
				graphData = pRowGraph;
				for (xx = xxStart; xx < xxEnd; xx++)
				{
					if (p[0] != 0)
					{
						int		a = p[0] * textAlpha / 255;
						graphData[0] = (uint8)(((pixlPT[0] - graphData[0]) * a + (graphData[0] << 8)) >> 8);
						graphData[1] = (uint8)(((pixlPT[1] - graphData[1]) * a + (graphData[1] << 8)) >> 8);
						graphData[2] = (uint8)(((pixlPT[2] - graphData[2]) * a + (graphData[2] << 8)) >> 8);
						graphData[3] = (uint8)((a + graphData[3]) - ((a * graphData[3] + 0xFF) >> 8));
					}
					graphData += 4;
					p++;
					pixlPT += 3;
				}
				pRowGlyph -= glyph->nWidthBitmap;
				pRowGraph -= nWidthBytesGraph;
				pRowPT += imgPattern->stride;
			}
		}*/
		x += glyph->nWidth;
	}

	return true;
}

bool CRawBmpFont::OutlinedTextOut(CRawGraph *pGraph, int x, int y, MLColor &clrText, MLColor &clrBorder, LPCTSTR szText, int nLen)
{
	return OutlinedDrawText(pGraph, x, y, pGraph->Width() - x, 0, clrText, clrBorder, szText, nLen);
}

bool CRawBmpFont::OutlinedDrawText(CRawGraph *pGraph, int x, int y, int width, int xLeftClipOffset, MLColor &clrText, MLColor &clrBorder, LPCTSTR szText, int nLen)
{
	byte		r, g, b;
	byte		textAlpha = pGraph->GetTextAlpha();
	int			yyStart, yyEnd;
	int			nOFHeight = GetHeightBitmap() + OUTLINED_BORDER;
	int			nOFWidth;	// OF = OutlinedFont
	CRect		rcClip;

	ASSERT(m_pmemGraph);

	agg::rendering_buffer bufGraph(pGraph->GetBuff(), pGraph->Width(), pGraph->Height(), -pGraph->Width() * 4);
	int			xPattern1 = 0, xPattern2 = 0;

	pGraph->GetClipBoundBox(rcClip);

	if (x + xLeftClipOffset > rcClip.right)
		return true;
	if (x + xLeftClipOffset > rcClip.left)
		rcClip.left = x + xLeftClipOffset;

	if (y >= rcClip.bottom)
		return true;
	if (y + nOFHeight <= rcClip.top)
		return true;

	if (x + width < rcClip.right)
		rcClip.right = x + width;

	if (x + OUTLINED_BORDER / 2 < rcClip.left)
	{
		if (m_imgPattern1)
			xPattern1 = (rcClip.left - x - OUTLINED_BORDER / 2) % m_imgPattern1->width();
		if (m_imgPattern2)
			xPattern2 = (rcClip.left - x - OUTLINED_BORDER / 2) % m_imgPattern2->width();
	}

	for (int i = 0; i < nLen; i++)
	{
		int			xxStart, xxEnd;
		// int			xx, yy;
		// byte		*p, *pRowGlyph, *graphData, *pRowGraph;
		Glyph		*glyph = GetGlyph((unsigned short)szText[i]);
		if (!glyph)
			continue;

		if (x >= rcClip.right)
			return true;

		nOFWidth = glyph->nWidthBitmap + OUTLINED_BORDER;

		if (x + nOFWidth <= rcClip.left)
		{
			x += glyph->nWidth;
			continue;
		}

		if (!glyph->bitmapOutlined)
			CreateOutlinedGlyph(glyph);

		if (glyph->bitmapOutlined)
		{
			//
			// Draw outlined border
			//
			r = clrBorder.r(), g = clrBorder.g(), b = clrBorder.b();

			if (y < rcClip.top)
				yyStart = rcClip.top - y;
			else
				yyStart = 0;
			if (y + nOFHeight > rcClip.bottom)
				yyEnd = rcClip.bottom - y;
			else
				yyEnd = nOFHeight;

			if (x < rcClip.left)
				xxStart = rcClip.left - x;
			else
				xxStart = 0;
			if (x + nOFWidth > rcClip.right)
				xxEnd = rcClip.right - x;
			else
				xxEnd = nOFWidth;

			agg::rendering_buffer bufGlyph(glyph->bitmapOutlined, nOFWidth, nOFHeight, glyph->WidthBytesOutlined());
			DrawGlyphRGBA32(bufGraph, x + xxStart, y + yyStart, bufGlyph, xxStart, yyStart, xxEnd, yyEnd, textAlpha, clrBorder);
		}

		if (glyph->bitmap)
		{
			//
			// Draw inner text
			//
			r = clrText.r(), g = clrText.g(), b = clrText.b();

			if (y + OUTLINED_BORDER / 2 < rcClip.top)
				yyStart = rcClip.top - (y + OUTLINED_BORDER / 2);
			else
				yyStart = 0;
			if (y + OUTLINED_BORDER / 2 + GetHeightBitmap() > rcClip.bottom)
				yyEnd = rcClip.bottom - (y + OUTLINED_BORDER / 2);
			else
				yyEnd = GetHeightBitmap();

			if (x + OUTLINED_BORDER / 2 < rcClip.left)
				xxStart = rcClip.left - (x + OUTLINED_BORDER / 2);
			else
				xxStart = 0;
			if (x + OUTLINED_BORDER / 2 + glyph->nWidthBitmap > rcClip.right)
				xxEnd = rcClip.right - (x + OUTLINED_BORDER / 2);
			else
				xxEnd = glyph->nWidthBitmap;

			agg::rendering_buffer bufGlyph(glyph->bitmap, glyph->nWidthBitmap, GetHeightBitmap(), glyph->WidthBytes());

			if (m_overlayMode == OM_PATTERN)
				DrawGlyphRGBA32(bufGraph, x + OUTLINED_BORDER / 2 + xxStart, y + OUTLINED_BORDER / 2 + yyStart, bufGlyph, xxStart, yyStart, xxEnd, yyEnd, textAlpha, m_imgPattern1, xPattern1);
			else if (m_overlayMode == OM_COLOR)
				DrawGlyphRGBA32(bufGraph, x + OUTLINED_BORDER / 2 + xxStart, y + OUTLINED_BORDER / 2 + yyStart, bufGlyph, xxStart, yyStart, xxEnd, yyEnd, textAlpha, clrText);
			else if (m_overlayMode == OM_DUAL_PATTERN)
				DrawGlyphRGBA32(bufGraph, x + OUTLINED_BORDER / 2 + xxStart, y + OUTLINED_BORDER / 2 + yyStart, bufGlyph, xxStart, yyStart, xxEnd, yyEnd, textAlpha, m_imgPattern1, xPattern1, m_nAlphaPattern1, m_imgPattern2, xPattern2, m_nAlphaPattern2);
			else if (m_overlayMode == OM_PATTERN_COLOR)
				DrawGlyphRGBA32(bufGraph, x + OUTLINED_BORDER / 2 + xxStart, y + OUTLINED_BORDER / 2 + yyStart, bufGlyph, xxStart, yyStart, xxEnd, yyEnd, textAlpha, m_imgPattern1, xPattern1, m_nAlphaPattern1, m_clrPattern2, m_nAlphaPattern2);
		}

		x += glyph->nWidth;
	}
	return true;
}


bool CRawBmpFont::GetTextExtentPoint32(LPCTSTR szText, int nLen, LPSIZE pSize)
{
	int		n = 0;

	ASSERT(m_pmemGraph);

	for (int i = 0; i < nLen; i++)
	{
		Glyph		*glyph = GetGlyph((unsigned short)szText[i]);
		if (!glyph)
			continue;

		n += glyph->nWidth;
	}

	pSize->cx = n;
	pSize->cy = m_nHeight;

	return true;
}

CRawBmpFont::Glyph *CRawBmpFont::GlyphFromGraph(int nChar)
{
	TCHAR			szText[2];
	SIZE			size;
	int				nWidth;
	int				x, y;
	byte			*pDst, *pRowDst, *pSrc, *pRowSrc;
	agg::rendering_buffer	bufGraph(m_memGraph.GetBuff(), m_memGraph.Width(), m_memGraph.Height(), -m_nWidthBytesGraph);

	ASSERT(m_pmemGraph);

	szText[0] = nChar;
	szText[1] = 0;

	if (!m_pmemGraph->GetTextExtentPoint32(szText, 1, &size))
		return NULL;
	nWidth = size.cx + 2;

	pRowSrc = bufGraph.row_ptr(0);
	for (y = 0; y < GetHeightBitmap(); y++)
	{
		memset(pRowSrc, 0, bufGraph.stride_abs());
		pRowSrc += bufGraph.stride();
	}

	if (!m_pmemGraph->TextOut(1, 1, szText, 1))
		return NULL;

	Glyph		*glyph = new Glyph;
	glyph->nWidth = (int16)size.cx;
	glyph->nWidthBitmap = nWidth;
	glyph->bitmap = new byte[glyph->WidthBytes() * GetHeightBitmap()];

	pRowSrc = bufGraph.row_ptr(0);
	pRowDst = glyph->bitmap;
	for (y = 0; y < GetHeightBitmap(); y++)
	{
		pDst = pRowDst;
		pSrc = pRowSrc;
		for (x = 0; x < glyph->nWidthBitmap; x++)
		{
// 			pDst[G_R] = 255;
// 			pDst[G_G] = 255;
// 			pDst[G_B] = 255;
// 			pDst[G_A] = 255;
			pDst[G_R] = pSrc[2];
			pDst[G_G] = pSrc[1];
			pDst[G_B] = pSrc[0];
			pDst[G_A] = RGBToGray(pSrc[G_R], pSrc[G_G], pSrc[G_B]);
			pDst += G_PIX_SIZE;
			pSrc += 3;
		}
		pRowSrc += bufGraph.stride();
		pRowDst += glyph->WidthBytes();
	}

//	memset(glyph->bitmap, 255, size.cx * GetHeightBitmap());

// 
// 	byte		*ptemp;
// 	ptemp = new byte[size.cx * GetHeightBitmap()];
// 
// 	memcpy(ptemp, glyph->bitmap, size.cx * GetHeightBitmap());
// 
// 	agg::rendering_buffer		rbufDest(glyph->bitmap, nWidth, GetHeightBitmap(), glyph->nWidth);
// 	agg::rendering_buffer		rbufSrc(ptemp, nWidth, GetHeightBitmap(), glyph->nWidth);
// 	// agg::rendering_buffer		rbufSrc(graphData, m_memGraph.Width(), GetHeightBitmap(), m_nWidthBytesGraph);
// 
// 	blur_text(100, 2, 3, rbufSrc, rbufDest, 1);

	return glyph;
}

CRawBmpFont::Glyph *CRawBmpFont::GetGlyph(int nChar)
{
	MAP_GLYPH::iterator		it;
	Glyph		*glyph = NULL;

	it = m_mapGlyph.find(nChar);
	if (it == m_mapGlyph.end())
	{
		glyph = GlyphFromGraph(nChar);
		ASSERT(glyph);
		if (!glyph)
			return NULL;
		m_mapGlyph[nChar] = glyph;
	}
	else
		glyph = (*it).second;

	return glyph;
}

static void CopyToOutlined(byte *pOut, byte *pIn, int nWidth, int nHeight, int nMarginShadow, int nOffset)
{
	byte		*p, *pOutlined;
	int			x, y;
	int			nWidthOutlined = nWidth + nMarginShadow * 2;

	if (nOffset > nMarginShadow)
		nOffset = nMarginShadow;

	int			nLeftMargin, nRightMargin;

	nLeftMargin = nMarginShadow + nOffset;
	nRightMargin = nMarginShadow - nOffset;

	p = pIn;
	pOutlined = pOut + nRightMargin * nWidthOutlined;
	for (y = 0; y < nHeight; y++)
	{
		for (x = 0; x < nLeftMargin; x++)
			pOutlined[x] = 0;
		for (x = 0; x < nRightMargin; x++)
			pOutlined[nWidthOutlined - 1 - x] = 0;
		memcpy(pOutlined + nLeftMargin, p, nWidth);
		p += nWidth;
		pOutlined += nWidthOutlined;
	}

	for (y = 0; y < nLeftMargin; y++)
	{
		pOutlined = pOut + (nHeight + nMarginShadow * 2 - y - 1) * nWidthOutlined;
		memset(pOutlined, 0, nWidthOutlined);
	}
	for (y = 0; y < nRightMargin; y++)
	{
		pOutlined = pOut + y * nWidthOutlined;
		memset(pOutlined, 0, nWidthOutlined);
	}
}

static void CopyAndOutlineBuffRGBA32(byte *pOut, byte *pIn, int nWidth, int nHeight, int nMargin)
{
	byte		*pIRow, *pORow;
	int			y, c;
	int			nWidthBytesOut = (nWidth + nMargin * 2) * G_PIX_SIZE;
	int			nWidthBytesIn = nWidth * G_PIX_SIZE;

	// CopyToOutlined(pOut, pIn, nWidth, nHeight, OUTLINED_BORDER / 2, 0);

	// outline
	pIRow = pIn;
	pORow = pOut + nMargin * nWidthBytesOut + nMargin * G_PIX_SIZE;
	for (y = 0; y < nHeight; y++)
	{
		byte		*pI = pIRow, *pO = pORow;

		for (int x = 0; x < nWidth; x++)
		{
			for (int i = 0; i < 4; i++, pI++, pO++)
			{
				c = *pI;
				if (c == 0)
					continue;
				if (c > *(pO))						*(pO) = c;
				if (c > *(pO + G_PIX_SIZE))			*(pO + G_PIX_SIZE) = c;
				if (c > *(pO - G_PIX_SIZE))			*(pO - G_PIX_SIZE) = c;
				if (c > *(pO + nWidthBytesOut))		*(pO + nWidthBytesOut) = c;
				if (c > *(pO - nWidthBytesOut))		*(pO - nWidthBytesOut) = c;
			}
		}
		pIRow += nWidthBytesIn;
		pORow += nWidthBytesOut;
	}
}

static void EraseBuffRGBA32(byte *pOut, byte *pIn, int nWidth, int nHeight, int nMargin)
{
	byte		*pIRow, *pORow;
	int			x, y;
	int			nWidthBytesOut = (nWidth + nMargin * 2) * G_PIX_SIZE;
	int			nWidthBytesIn = nWidth * G_PIX_SIZE;

	pIRow = pIn;
	pORow = pOut + nMargin * nWidthBytesOut + nMargin * G_PIX_SIZE;
	for (y = 0; y < nHeight; y++)
	{
		byte		*pI = pIRow, *pO = pORow;

		for (x = 0; x < nWidth; x++)
		{
			if (pI[G_A] == 0xFF)
			{
				pO[G_A] = pO[G_R] = pO[G_G] = pO[G_B] = 0;
			}
			pI += 4;
			pO += 4;
		}
		pIRow += nWidthBytesIn;
		pORow += nWidthBytesOut;
	}
}

static void CopyAndShadowBuffRGBA32(byte *pOut, byte *pIn, int nWidth, int nHeight, int nMarginShadow, int nAlphaShadow, int nRangeShadow)
{
// 	if (nRangeShadow > nMarginShadow)
// 		nRangeShadow = nMarginShadow;

	// CopyToOutlined(pOut, pIn, nWidth, nHeight, nMarginShadow, 0);
	CopyAndOutlineBuffRGBA32(pOut, pIn, nWidth, nHeight, nMarginShadow);

	agg::rendering_buffer	buff(pOut, nWidth + nMarginShadow * 2, nHeight + nMarginShadow * 2,  (nWidth + nMarginShadow * 2) * G_PIX_SIZE);
	agg::pixfmt_rgba32		pixf(buff);

	agg::stack_blur_rgba32(pixf, nRangeShadow, nRangeShadow);

	EraseBuffRGBA32(pOut, pIn, nWidth, nHeight, nMarginShadow);

/*	byte		*p, *pOutlined;
	int			y, c;

	// Shawdow
	p = pIn;
	pOutlined = pOut+ nMargin * nWidthOutlined;
	for (y = 0; y < nHeight; y++)
	{
		for (int x = 0; x < nWidth - 1; x++)
		{
			c = p[x];
			if (c == 0)
				continue;
			if (c > p[x + 1])
			{
				// x --> shadow
			}

			if (c > p[x + nWidth])
			{
				// y --> shadow

			}
		}
		p += nWidth;
		pOutlined += nWidthOutlined;
	}*/
}
// 
// void GlowGlyph(Glyph *glyph, int nHeight, int nMargin)
// {
// 	
// }

void CRawBmpFont::CreateOutlinedGlyph(Glyph *glyph)
{
	int			nWidthOutlined;

	ASSERT(glyph->bitmapOutlined == NULL);

	nWidthOutlined = glyph->nWidthBitmap + OUTLINED_BORDER;

	glyph->bitmapOutlined = new byte[(GetHeightBitmap() + OUTLINED_BORDER) * glyph->WidthBytesOutlined()];
	if (!glyph->bitmapOutlined)
		return;

	memset(glyph->bitmapOutlined, 0, (GetHeightBitmap() + OUTLINED_BORDER) * glyph->WidthBytesOutlined());

	if (m_shadowMode == SM_SHADOW)
		CopyAndShadowBuffRGBA32(glyph->bitmapOutlined, glyph->bitmap, glyph->nWidthBitmap, GetHeightBitmap(), OUTLINED_BORDER / 2, 255, GetHeightBitmap() / 8);
	else
	{
		// Outline glyph
		CopyAndOutlineBuffRGBA32(glyph->bitmapOutlined, glyph->bitmap, glyph->nWidthBitmap, GetHeightBitmap(), OUTLINED_BORDER / 2);

		EraseBuffRGBA32(glyph->bitmapOutlined, glyph->bitmap, glyph->nWidthBitmap, GetHeightBitmap(), OUTLINED_BORDER / 2);
	}

	return;
/*
	int			nWidthOutlined;
	byte		*p, *pOutlined;
	int			x, y;

	ASSERT(glyph->bitmapOutlined == NULL);

	nWidthOutlined = glyph->nWidth + OUTLINED_BORDER;

	glyph->bitmapOutlined = new byte[(GetHeightBitmap() + OUTLINED_BORDER) * nWidthOutlined];
	if (!glyph->bitmapOutlined)
		return;

	memset(glyph->bitmapOutlined, 0, (GetHeightBitmap() + OUTLINED_BORDER) * nWidthOutlined);


	int		k[9] = { 1, 2, 1, 1, 4, 1, 1, 2, 1 };
	int		f = 0;

	for (int i = 0; i < 9; i++)
		f += k[i];

	Filter(glyph, GetHeightBitmap(), k, 3, f);
	return;*/

// 
// 	if (m_bThickOutline)
// 	{
// 		byte		*pTemp;
// 
// 		pTemp = new byte[(GetHeightBitmap() + OUTLINED_BORDER) * nWidthOutlined];
// 		if (!pTemp)
// 		{
// 			delete[] glyph->bitmapOutlined;
// 			glyph->bitmapOutlined = NULL;
// 			return;
// 		}
// 
// 		OutlineBuff(pTemp, nWidthOutlined, glyph->bitmap, glyph->nWidth, GetHeightBitmap());
// 		OutlineBuff(glyph->bitmapOutlined, nWidthOutlined, pTemp + OUTLINED_BORDER / 2 + nWidthOutlined, nWidthOutlined, GetHeightBitmap());
// 
// 		delete[] pTemp;
// 	}
// 	else
// 	{
// 		OutlineBuff(glyph->bitmapOutlined, nWidthOutlined, glyph->bitmap, glyph->nWidth, GetHeightBitmap());
// 	}
// 
// 	EraseGlyph(pOut, pIn, nWidth, nHeight, OUTLINED_BORDER / 2, 0);

/*
	memset(glyph->bitmap, 0, GetHeightBitmap() * glyph->nWidth);
	for (x = glyph->nWidth / 2; x < glyph->nWidth; x++)
	{
		glyph->bitmap[glyph->nWidth * 3 - 1] = 255;
		// glyph->bitmap[(GetHeightBitmap() - 1) * glyph->nWidth + glyph->nWidth - 2] = 255;
		break;
	}
*/
//	memset(glyph->bitmapOutlined, 0, (GetHeightBitmap() + OUTLINED_BORDER) * nWidthOutlined);
//	memset(pTemp, 0, (GetHeightBitmap() + OUTLINED_BORDER) * nWidthOutlined);
// 
// 	p = glyph->bitmap;
// 	pOutlined = pTemp + OUTLINED_BORDER / 2 * nWidthOutlined;
// 	for (y = 0; y < GetHeightBitmap(); y++)
// 	{
// 		for (int x = 0; x < OUTLINED_BORDER / 2; x++)
// 		{
// 			pOutlined[x] = 0;
// 			pOutlined[nWidthOutlined - 1 - x] = 0;
// 		}
// 		memcpy(pOutlined + OUTLINED_BORDER / 2, p, glyph->nWidth);
// 		p += glyph->nWidth;
// 		pOutlined += nWidthOutlined;
// 	}
// 
// 	for (y = 0; y < OUTLINED_BORDER / 2; y++)
// 	{
// 		pOutlined = pTemp + y * nWidthOutlined;
// 		memset(pOutlined, 0, nWidthOutlined);
// 
// 		pOutlined = pTemp + (GetHeightBitmap() + OUTLINED_BORDER - y - 1) * nWidthOutlined;
// 		memset(pOutlined, 0, nWidthOutlined);
// 	}
// 	OutlineBuff(pTemp, nWidthOutlined, glyph->bitmap, glyph->nWidth, GetHeightBitmap());
// 	// memcpy(glyph->bitmapOutlined, pTemp, (GetHeightBitmap() + OUTLINED_BORDER) * nWidthOutlined);
// 	OutlineBuff(glyph->bitmapOutlined, nWidthOutlined, pTemp + OUTLINED_BORDER / 2 + nWidthOutlined, nWidthOutlined, GetHeightBitmap());

//	OutlineBuff(glyph->bitmapOutlined, nWidthOutlined, glyph->bitmap, glyph->nWidth, GetHeightBitmap());

	// copy data
/*	p = glyph->bitmap;
	pOutlined = glyph->bitmapOutlined + OUTLINED_BORDER / 2 * nWidthOutlined;
	for (y = 0; y < GetHeightBitmap(); y++)
	{
		for (int x = 0; x < OUTLINED_BORDER / 2; x++)
		{
			pOutlined[x] = 0;
			pOutlined[nWidthOutlined - 1 - x] = 0;
		}
		memcpy(pOutlined + OUTLINED_BORDER / 2, p, glyph->nWidth);
		p += glyph->nWidth;
		pOutlined += nWidthOutlined;
	}

	for (y = 0; y < OUTLINED_BORDER / 2; y++)
	{
		pOutlined = glyph->bitmapOutlined + y * nWidthOutlined;
		memset(pOutlined, 0, nWidthOutlined);

		pOutlined = glyph->bitmapOutlined + (GetHeightBitmap() + OUTLINED_BORDER - y - 1) * nWidthOutlined;
		memset(pOutlined, 0, nWidthOutlined);
	}

	// outline
	p = glyph->bitmap;
	pOutlined = glyph->bitmapOutlined + OUTLINED_BORDER / 2 * nWidthOutlined;
	for (y = 1; y < GetHeightBitmap() + OUTLINED_BORDER / 2; y++)
	{
		for (int x = 1; x < glyph->nWidth + OUTLINED_BORDER / 2; x++)
		{
			c = p[x - 1];
			if (c == 0)
				continue;
			if (c > pOutlined[x + 1])
				pOutlined[x + 1] = c;
			if (c > pOutlined[x - 1])
				pOutlined[x - 1] = c;
			if (c > pOutlined[x + nWidthOutlined])
				pOutlined[x + nWidthOutlined] = c;
			if (c > pOutlined[x - nWidthOutlined])
				pOutlined[x - nWidthOutlined] = c;
		}
		p += glyph->nWidth;
		pOutlined += nWidthOutlined;
	}

	/*
	memset(glyph->bitmapOutlined, 0, (GetHeightBitmap() + OUTLINED_BORDER) * nWidthOutlined);

	for (y = 0; y < GetHeightBitmap(); y++)
	{
		p = glyph->bitmap + y * glyph->nWidth;
		pOutlined = glyph->bitmapOutlined + (y + OUTLINED_BORDER / 2) * nWidthOutlined;
		for (x = 0; x < glyph->nWidth; x++)
		{
			c = p[x];
			if (c != 0)
			{
				if (pOutlined[x] < c)
					pOutlined[x] = c;
				if (pOutlined[x + 1] < c)
					pOutlined[x + 1] = c;
				if (pOutlined[x + 2] < c)
					pOutlined[x + 2] = c;
				if (pOutlined[x + 3] < c)
					pOutlined[x + 3] = c;
				if (pOutlined[x + 1 - nWidthOutlined] < c)
					pOutlined[x + 1 - nWidthOutlined] = c;
				if (pOutlined[x + 1 - nWidthOutlined * 2] < c)
					pOutlined[x + 1 - nWidthOutlined * 2] = c;
				pOutlined[x + 1 + nWidthOutlined] = c;
				pOutlined[x + 1 + nWidthOutlined * 2] = c;
			}
		}
	}
	*/
}

void CRawBmpFont::DestroyGlyphMap()
{
	MAP_GLYPH::iterator		it, itEnd;

	itEnd = m_mapGlyph.end();
	for (it = m_mapGlyph.begin(); it != itEnd; ++it)
	{
		Glyph		*p = (*it).second;
		delete p;
	}
	m_mapGlyph.clear();
}

bool CRawBmpFont::InitSet(IMLGraphicsBase *pGraph)
{
	// Destroy();
	if (m_pmemGraph)
		return true;

	m_nWidthGraph = GetHeightBitmap() * 2;
	m_nWidthBytesGraph = (m_nWidthGraph * 3 + 3) & ~ 3;

	m_memGraph.Create(m_nWidthGraph, GetHeightBitmap(), pGraph, 24);
	m_memGraph.SetBkMode(false);

	if (m_bGdiplusAntialias)
	{
		CGdiplusGraphics	*pmemGraph;

		pmemGraph = new CGdiplusGraphics;
		pmemGraph->Create(m_memGraph.GetHandle());
		m_pmemGraph = pmemGraph;
	}
	else
	{
		CGdiGraphics	*pmemGraph;

		pmemGraph = new CGdiGraphics;
		pmemGraph->Create(m_memGraph.GetHandle());
		m_pmemGraph = pmemGraph;
	}

	m_memGraph.SetBkMode(false);
	m_pmemGraph->SetFont(this);
	m_pmemGraph->SetTextColor(MLColor(RGB(255, 255, 255)));
	m_pmemGraph->SetBgColor(MLColor(RGB(0, 0, 0)));
	// ::SelectObject(pmemGraph->GetHandle(), CreateSolidBrush(RGB(0, 0, 0)));

	return true;
}

void CRawBmpFont::SetOverlayPattern(CRawImage *imgPattern)
{
	m_imgPattern1 = imgPattern;
	m_overlayMode = OM_PATTERN;
	m_imgPattern2 = NULL;
}

void CRawBmpFont::SetOverlayPattern(CRawImage *imgPattern1, CRawImage *imgPattern2, int nAlphaPattern1, int nAlphaPattern2)
{
	m_imgPattern1 = imgPattern1;
	m_imgPattern2 = imgPattern2;
	m_nAlphaPattern1 = nAlphaPattern1;
	m_nAlphaPattern2 = nAlphaPattern2;
	m_overlayMode = OM_DUAL_PATTERN;
}

void CRawBmpFont::SetOverlayPattern(CRawImage *imgPattern1, int nAlphaPattern1, MLColor &clrPattern2, int nAlphaPattern2)
{
	m_imgPattern1 = imgPattern1;
	m_clrPattern2 = clrPattern2;
	m_nAlphaPattern1 = nAlphaPattern1;
	m_nAlphaPattern2 = nAlphaPattern2;
	m_overlayMode = OM_PATTERN_COLOR;
	m_imgPattern2 = NULL;
}

void CRawBmpFont::UseColorOverlay()
{
	m_imgPattern1 = NULL;
	m_imgPattern2 = NULL;
	m_overlayMode = OM_COLOR;
}

void CRawBmpFont::SetShadowMode(ShadowMode shadowMode)
{
	if (m_shadowMode != shadowMode)
	{
		MAP_GLYPH::iterator		it, itEnd;

		itEnd = m_mapGlyph.end();
		for (it = m_mapGlyph.begin(); it != itEnd; ++it)
		{
			Glyph		*p = (*it).second;
			delete [] p->bitmapOutlined;
			p->bitmapOutlined = NULL;
		}

		m_shadowMode = shadowMode;
	}
}
