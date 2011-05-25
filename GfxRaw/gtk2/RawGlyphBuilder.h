#ifndef _RAW_GLYPH_BUILDER_H_
#define _RAW_GLYPH_BUILDER_H_

#pragma once

namespace GfxRaw
{

class CRawGlyphBuilder
{
public:
	CRawGlyphBuilder();
	virtual ~CRawGlyphBuilder();

	void Init(const CFontInfo &font);

	Glyph *BuildGlyph(tstring &ch);

protected:
	int GetHeightBitmap() { return m_font.GetHeight() + MARGIN_BITMAP * 2; }

protected:
	cairo_t					*m_cr;
	cairo_surface_t			*m_surface;

	CMLFont					m_font;
	int						m_nWidthGraph;

};

};

#endif // _RAW_GLYPH_BUILDER_H_
