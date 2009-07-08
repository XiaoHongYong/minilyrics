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

	Glyph *BuildGlyph(wchar_t nChar);

protected:
	int GetHeightBitmap() { return m_font.GetHeight() + MARGIN_BITMAP * 2; }

protected:
	CMLFont					m_font;
	IMLGraphicsBase			*m_pmemGraph;
	CRawGraph				m_memGraph;
	int						m_nWidthGraph;
	int						m_nWidthBytesGraph;

};

};

#endif // _RAW_GLYPH_BUILDER_H_
