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
	m_cr = NULL;
	m_surface = NULL;
	m_nWidthGraph = 0;
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

	m_nWidthGraph = GetHeightBitmap() * 2;
}

Glyph *CRawGlyphBuilder::BuildGlyph(StringIterator::CharType ch)
{
	PangoLayout *layout;
	PangoFontDescription *desc;
	int width, height;

	// Clear graph
	cairo_set_source_rgb(m_cr, 0, 0, 0);
	cairo_paint(m_cr);

	// Create font
	desc = pango_font_description_new();
	pango_font_description_set_family(desc, m_font.GetName());
	pango_font_description_set_size(desc, m_font.GetHeight());
	pango_font_description_set_weight(desc, (PangoWeight)m_font.GetWeight());
	pango_font_description_set_style(desc, (PangoStyle)m_font.GetWeight());

	// Create a PangoLayout, set the font and text
	layout = pango_cairo_create_layout (m_cr);

	pango_layout_set_text (layout, ch, -1);
	pango_layout_set_font_description (layout, desc);
	pango_font_description_free (desc);

	// Draw text
	cairo_set_source_rgb (m_cr, 1.0, 1.0, 1.0);
	pango_cairo_update_layout (m_cr, layout);
	pango_layout_get_size (layout, &width, &height);
	pango_cairo_show_layout (m_cr, layout);

	g_object_unref (layout);

	int				nWidth;
	int				x, y;
	byte			*pDst, *pRowDst, *pSrc, *pRowSrc;

	DBGLOG3("glyph: %c, w: %d, h: %d", (LPCSTR)ch, width, height);
	DBGLOG1("Font height: %d", m_font.GetHeight());
	// assert(height == m_font.GetHeight());

	agg::rendering_buffer	bufGraph(cairo_image_surface_get_data(m_surface), cairo_image_surface_get_width(m_surface), cairo_image_surface_get_height(m_surface), cairo_image_surface_get_stride(m_surface));

	nWidth = width + MARGIN_BITMAP * 2;

	Glyph		*glyph = new Glyph;
	glyph->nWidth = (int16)width;
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

	return glyph;
}

};
