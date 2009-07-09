#include "stdafx.h"
#include <pango/pangocairo.h>
#include "GfxRaw.h"
#include "RawBmpFont.h"
#include "RawGlyphBuilder.h"

namespace GfxRaw
{

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
	cairo_destroy(m_cr);
	cairo_surface_destroy(m_surface);
}

void CRawGlyphBuilder::Init(const CFontInfo &font)
{
	if (m_cr && m_font.IsSame(font))
		return;

	m_font.Create(font);

	cairo_status_t status;

	m_surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24,
		GetHeightBitmap() * 2, GetHeightBitmap());
	m_cr = cairo_create(m_surface);

	cairo_set_source_rgb(m_cr, 0, 0, 0);
	cairo_paint(m_cr);

	m_nWidthGraph = GetHeightBitmap() * 2;
}

Glyph *CRawGlyphBuilder::BuildGlyph(wchar_t nChar)
{
	PangoLayout *layout;
	PangoFontDescription *desc;

	/* Create a PangoLayout, set the font and text */
	layout = pango_cairo_create_layout (m_cr);

	pango_layout_set_text (layout, "Text", -1);
	desc = pango_font_description_from_string (FONT);
	pango_layout_set_font_description (layout, desc);
	pango_font_description_free (desc);

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
