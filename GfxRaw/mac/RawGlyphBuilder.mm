#include "stdafx.h"
#import "GfxRawMac.h"
#include "../../SDK/Agg/include/agg_scanline_p.h"
#include "../../SDK/Agg/include/agg_renderer_scanline.h"
#include "../../SDK/Agg/include/agg_pixfmt_rgba.h"
#include "../../SDK/Agg/include/agg_pixfmt_rgb.h"
#include "../../SDK/Agg/include/agg_rasterizer_scanline_aa.h"
#include "GfxRaw.h"
#include "RawBmpFont.h"
#include "../../GfxLite/GfxLite.h"
#include "RawGlyphBuilder.h"

namespace GfxRaw
{

#define MEM_GRAPH_BITS	32

extern bool	g_bAntialiasFontEnabled;

CRawGlyphBuilder::CRawGlyphBuilder()
{
	m_attributes = NULL;
}

CRawGlyphBuilder::~CRawGlyphBuilder()
{
	if (m_attributes)
		CFRelease(m_attributes);
}

CGFloat GetLineHeightForFont(CTFontRef iFont)
{
	CGFloat lineHeight = 0.0;
	
	// Get the ascent from the font, already scaled for the font's size
	lineHeight += CTFontGetAscent(iFont);
	
	// Get the descent from the font, already scaled for the font's size
	lineHeight += CTFontGetDescent(iFont);
	
	// Get the leading from the font, already scaled for the font's size
	lineHeight += CTFontGetLeading(iFont);
	
	return lineHeight;
}

void CRawGlyphBuilder::Init(const CFontInfo &font)
{
	if (m_fontMemGraph.IsValid())
	{
		if (m_font.IsSame(font))
			return;
	}

	m_fontMemGraph.Destroy();

	m_font.Create(font);

	{
		CGFloat lineHeight = GetLineHeightForFont(m_font.GetHandle());
		m_nLineHeight = (int)(lineHeight + 0.99);
		
		m_fontMemGraph.Create(GetHeightBitmap() * 2, GetHeightBitmap(), NULL, MEM_GRAPH_BITS);

		CGContextSetTextMatrix(m_fontMemGraph.GetHandle(), CGAffineTransformMakeScale(1.0, -1.0));
		
		CGColorSpaceRef rgbColorSpace = CGColorSpaceCreateDeviceRGB();
		CGFloat components[] = { 1.0, 1.0, 1.0, 1.0 };
		CGColorRef red = CGColorCreate(rgbColorSpace, components);
		CGColorSpaceRelease(rgbColorSpace);
		
		// Initialize string, font, and context
		CFStringRef keys[] = { kCTFontAttributeName, kCTForegroundColorAttributeName };
		CFTypeRef values[] = { m_font.GetHandle(), red };
		
		m_attributes = CFDictionaryCreate(kCFAllocatorDefault, (const void**)&keys,
						   (const void**)&values, sizeof(keys) / sizeof(keys[0]),
						   &kCFTypeDictionaryKeyCallBacks,
						   &kCFTypeDictionaryValueCallBacks);
		
		
	}

/*	m_graph.Attach(m_fontMemGraph.GetHandle());
	m_graph.SetFont(&m_font);
	m_graph.SetTextColor(MLColor(RGB(255, 255, 255)));
	m_graph.SetBgColor(MLColor(RGB(0, 0, 0)));
	CGContextSetRGBFillColor(m_fontMemGraph.GetHandle(), 1.0, 1.0, 1.0, 1.0);*/
}

void CRawGlyphBuilder::Destroy()
{
	// m_graph.Detach();
	m_fontMemGraph.Destroy();
	m_font.Destroy();
}

void CopyBitmpDataFromGraphBuffer(Glyph *pGlyph, agg::pixfmt_rgba32 &bufGraph);

int CRawGlyphBuilder::GetHeightBitmap()
{
	return m_nLineHeight + MARGIN_BITMAP * 2;
}

Glyph *CRawGlyphBuilder::BuildGlyph(tstring &ch)
{
	SIZE			size;
	PRAW_IMAGE_DATA	rawData = m_fontMemGraph.GetRawBuff();

	agg::rendering_buffer	bufGraph(rawData->buff, rawData->width, rawData->height, rawData->stride);

	//if (!m_graph.GetTextExtentPoint32(ch, ch.size(), &size))
	//	return NULL;

	int nHeightBitmap = GetHeightBitmap();

	uint8 *pRowSrc = bufGraph.row_ptr(0);
	for (int y = 0; y < nHeightBitmap; y++)
	{
		memset(pRowSrc, 0, bufGraph.stride_abs());
		pRowSrc += bufGraph.stride();
	}
	
	{
		CFStringRef	string = CFStringCreateWithBytes(NULL, (UInt8*)(const char*)ch.c_str(), ch.size(), kCFStringEncodingUTF8, false);
		if (string)
		{
			CFAttributedStringRef attrString = CFAttributedStringCreate(kCFAllocatorDefault, string, m_attributes);
			CFRelease(string);
			
			CTLineRef line = CTLineCreateWithAttributedString(attrString);
			
			// Set text position and draw the line into the graphics context
			CGContextSetTextPosition(m_fontMemGraph.GetHandle(), MARGIN_BITMAP, MARGIN_BITMAP + m_nLineHeight - m_nLineHeight / 6);
			CGPoint ptStart = CGContextGetTextPosition(m_fontMemGraph.GetHandle());
			
			CTLineDraw(line, m_fontMemGraph.GetHandle());

			CGPoint ptEnd = CGContextGetTextPosition(m_fontMemGraph.GetHandle());
			
			size.cx = ptEnd.x - ptStart.x;
			
			CFRelease(line);
		}
		else
		{
			size.cx = m_font.GetHeight() / 2;
			// CRect rc(MARGIN_BITMAP, MARGIN_BITMAP, size.cx, m_font.GetHeight());
			// m_fontMemGraph.FillRect(&rc, MLColor(RGB(255, 255, 255)), BPM_COPY);
		}
	}
	

	//if (!m_graph.TextOut(MARGIN_BITMAP, MARGIN_BITMAP, ch, ch.size()))
	//	return NULL;

	Glyph		*glyph = new Glyph;
	glyph->ch = ch;
	glyph->nWidth = (byte)size.cx;

	agg::pixfmt_rgba32		pixf(bufGraph);
	CopyBitmpDataFromGraphBuffer(glyph, pixf);

	return glyph;
}

};
