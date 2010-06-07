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

	void Destroy();
	
	Glyph *BuildGlyph(StringIterator::CharType ch);

protected:
	int GetHeightBitmap() { return m_font.GetHeight() + MARGIN_BITMAP * 2; }

protected:
	CMLFont					m_font;
	CRawGraphData			m_fontMemGraph;
	CMLGraphics				m_graph;

};

};

#endif // _RAW_GLYPH_BUILDER_H_
