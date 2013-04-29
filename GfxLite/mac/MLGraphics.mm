  // MLGraphics.cpp: implementation of the CMLGraphics class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GfxLiteMac.h"
#include "GfxLite.h"
#include "MLGraphics.h"
#include "MLColor.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMLGraphics::CMLGraphics()
{
}

CMLGraphics::~CMLGraphics()
{
}

bool CMLGraphics::TextOut(int x, int y, LPCTSTR szText, int nLen)
{
	CGContextShowTextAtPoint(m_context, x, y + m_fontInfo.GetHeight(), szText, nLen);

	return TRUE;
}

bool CMLGraphics::GetTextExtentPoint32(LPCTSTR szText, int nLen, LPSIZE pSize)
{
	CGContextSetTextDrawingMode(m_context, kCGTextInvisible);

	CGPoint ptStart = CGContextGetTextPosition(m_context);
	CGContextShowText(m_context, szText, nLen);
	CGPoint ptEnd = CGContextGetTextPosition(m_context);

	pSize->cx = ptEnd.x - ptStart.x;
	pSize->cy = m_fontInfo.GetHeight();

	CGContextSetTextDrawingMode(m_context, kCGTextFill);
	
	return true;
}

void CMLGraphics::SetTextColor(const MLColor &color)
{
	CGContextSetRGBFillColor(m_context, color.r() / 255.0, color.g() / 255.0, color.b() / 255.0, color.getAlpha() / 255.0);
}

void CMLGraphics::SetBgColor(const MLColor &color)
{
	ERRLOG0("Unimplemented fun CMLGraphics::");
}

void CMLGraphics::SetFont(CFontInfo *font)
{
	m_fontInfo = *font;

	CGContextSelectFont(m_context, "Helvetica", font->GetSize(), kCGEncodingMacRoman);
	// Next we set the text matrix to flip our text upside down. We do this because the context itself
	// is flipped upside down relative to the expected orientation for drawing text (much like the case for drawing Images & PDF).
	CGContextSetTextMatrix(m_context, CGAffineTransformMakeScale(1.0, -1.0));

	// CGContextSetFont(m_context, m_font->GetHandle());
	// CGContextSetFontSize(m_context, m_font->GetSize());
	CGContextSetTextDrawingMode(m_context, kCGTextFill);
}

void CMLGraphics::Attach(CGContextRef context)
{
	Detach();
	
	m_context = context;
}

CGContextRef CMLGraphics::Detach()
{
	CGContextRef old = m_context;
	
	if (m_context != NULL)
		m_context = NULL;

	return old;
}
