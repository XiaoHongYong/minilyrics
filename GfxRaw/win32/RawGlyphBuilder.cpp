#include "stdafx.h"
#include "../../GfxLite/win32/GdiGraphicsLite.h"
#include "../../GfxLite/win32/GdiplusGraphicsLite.h"
#include "../SDK/Agg/include/agg_scanline_p.h"
#include "../SDK/Agg/include/agg_renderer_scanline.h"
#include "../SDK/Agg/include/agg_pixfmt_rgba.h"
#include "../SDK/Agg/include/agg_pixfmt_rgb.h"
#include "../SDK/Agg/include/agg_rasterizer_scanline_aa.h"
#include "GfxRaw.h"
#include "RawBmpFont.h"
#include "RawGlyphBuilder.h"

namespace GfxRaw
{

#define MEM_GRAPH_BITS	24

extern bool	g_bAntialiasFontEnabled;

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
	if (m_pmemGraph && m_font.IsSame(font))
		return;

	m_font.Create(font);

	CreateMemGraph(GetHeightBitmap() * 2);
}


int CRawGlyphBuilder::GetHeight() const
{
	return m_font.GetHeight();
}

void CRawGlyphBuilder::CreateMemGraph(int nWidthGraph)
{
	nWidthGraph = (nWidthGraph + 3 % 4) * 4;
	m_memGraph.Destroy();
	if (m_pmemGraph)
	{
		delete m_pmemGraph;
		m_pmemGraph = NULL;
	}

	CMLGraphics		graphScreen;
	HDC				hdcScreen;
	hdcScreen = GetDC(NULL);
	graphScreen.Attach(hdcScreen);

	m_memGraph.Create(nWidthGraph, GetHeightBitmap(), &graphScreen, MEM_GRAPH_BITS);

	if (g_bAntialiasFontEnabled)
	{
		CGdiplusGraphicsLite	*pmemGraph;

		pmemGraph = new CGdiplusGraphicsLite;
		pmemGraph->Attach(m_memGraph.GetHandle());
		m_pmemGraph = pmemGraph;
	}
	else
	{
		CGdiGraphicsLite	*pmemGraph;

		pmemGraph = new CGdiGraphicsLite;
		pmemGraph->Attach(m_memGraph.GetHandle());
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
}

void CopyBitmpDataFromGraphBuffer(Glyph *pGlyph, agg::pixfmt_rgb24 &bufGraph);

Glyph *CRawGlyphBuilder::BuildGlyph(tstring &ch)
{
	SIZE			size;

	ASSERT(m_pmemGraph);

	if (!m_pmemGraph->GetTextExtentPoint32(ch.c_str(), ch.size(), &size))
		return NULL;

	if (size.cx > m_memGraph.Width() + MARGIN_BITMAP * 2)
		CreateMemGraph(size.cx + GetHeightBitmap());

	PRAW_IMAGE_DATA			rawBuff = m_memGraph.GetRawBuff();
	agg::rendering_buffer	bufGraph(rawBuff->buff, rawBuff->width, rawBuff->height, rawBuff->stride);

	int nHeightBitmap = GetHeightBitmap();

	uint8 *pRowSrc = bufGraph.row_ptr(0);
	for (int y = 0; y < nHeightBitmap; y++)
	{
		memset(pRowSrc, 0, bufGraph.stride_abs());
		pRowSrc += bufGraph.stride();
	}

	if (!m_pmemGraph->TextOut(MARGIN_BITMAP, MARGIN_BITMAP, ch.c_str(), ch.size()))
		return NULL;

	Glyph		*glyph = new Glyph;
	glyph->ch = ch;
	glyph->nWidth = (uint16)size.cx;

	agg::pixfmt_rgb24		pixf(bufGraph);
	CopyBitmpDataFromGraphBuffer(glyph, pixf);

	return glyph;
}

};
