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

	Glyph *BuildGlyph(tstring &ch);

protected:
	void CreateMemGraph(int nWidthGraph);
	int GetHeightBitmap() { return m_font.GetHeight() + MARGIN_BITMAP * 2; }

protected:
	CFontInfo				m_font;
	CMLGraphics				*m_pmemGraph;
	CRawGraph				m_memGraph;

};

};

#endif // _RAW_GLYPH_BUILDER_H_
