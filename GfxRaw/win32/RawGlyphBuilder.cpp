#include "stdafx.h"
#include "GfxRaw.h"
#include "RawBmpFont.h"
#include "RawGlyphBuilder.h"

namespace GfxRaw
{

extern bool	g_bAntialiasFontEnabled;

inline byte RGBToGray(int r, int g, int b)
{
	// 5 = ËÄÉáÎåÈë
	return (r * 3 + g * 6 + b + 5) / 10;
}

CRawGlyphBuilder::CRawGlyphBuilder()
{
	m_pmemGraph = NULL;
}

CRawGlyphBuilder::~CRawGlyphBuilder()
{
	Destroy();
}

void CRawGlyphBuilder::Init(const CFontInfo &font)
{
	if (m_pmemGraph)
	{
		if (m_font.IsSame(font))
			return;
		delete m_pmemGraph;
		m_pmemGraph = NULL;
	}

	m_memGraph.Destroy();

	m_font.Create(font);

	CGdiGraphics	graphScreen;
	HDC				hdcScreen;

	hdcScreen = GetDC(NULL);
	graphScreen.Create(hdcScreen);

	m_nWidthGraph = GetHeightBitmap() * 2;
	m_nWidthBytesGraph = (m_nWidthGraph * 3 + 3) & ~ 3;

	m_memGraph.Create(m_nWidthGraph, GetHeightBitmap(), &graphScreen, 24);

	if (g_bAntialiasFontEnabled)
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

	m_pmemGraph->SetFont(&m_font);
	m_pmemGraph->SetTextColor(MLColor(RGB(255, 255, 255)));
	m_pmemGraph->SetBgColor(MLColor(RGB(0, 0, 0)));

	ReleaseDC(NULL, hdcScreen);
}

void CRawGlyphBuilder::Destroy()
{
	if (m_pmemGraph)
	{
		delete m_pmemGraph;
		m_pmemGraph = NULL;
	}

	m_memGraph.Destroy();
	m_font.Destroy();
}

Glyph *CRawGlyphBuilder::BuildGlyph(wchar_t nChar)
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

	nWidth = size.cx + MARGIN_BITMAP * 2;

	pRowSrc = bufGraph.row_ptr(0);
	for (y = 0; y < GetHeightBitmap(); y++)
	{
		memset(pRowSrc, 0, bufGraph.stride_abs());
		pRowSrc += bufGraph.stride();
	}

	if (!m_pmemGraph->TextOut(MARGIN_BITMAP, MARGIN_BITMAP, szText, 1))
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

};
