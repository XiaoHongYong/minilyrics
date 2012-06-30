  // MLGraphics.cpp: implementation of the CMLGraphics class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "gfx.h"
#include "MLGraphics.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMLGraphics::CMLGraphics()
{
	m_GraphicsType = MGT_MACOSX;

	// m_nWidth = m_nHeight = 0;
}

CMLGraphics::~CMLGraphics()
{
}

void CMLGraphics::Line(int x1, int y1, int x2, int y2)
{
	ERRLOG0("Unimplemented fun CMLGraphics::");
}

void CMLGraphics::Rectangle(int x, int y, int width, int height)
{
	ERRLOG0("Unimplemented fun CMLGraphics::");
}

void CMLGraphics::FillRect(int x, int y, int width, int height, CMLBrush *pBrush)
{
	ERRLOG0("Unimplemented fun CMLGraphics::");
}

bool CMLGraphics::TextOut(int x, int y, LPCTSTR szText, int nLen)
{
	CGContextShowTextAtPoint(m_context, x, y + m_font->GetHeight(), szText, nLen);

	return TRUE;
}

bool CMLGraphics::DrawTextClip(LPCTSTR szText, int nLen, LPRECT lpRect)
{
	ERRLOG0("Unimplemented fun CMLGraphics::");
	return false;
}

bool CMLGraphics::TextOutOutlined(int x, int y, LPCTSTR szText, int nLen, const MLColor &clrText, const MLColor &clrBorder)
{
	ERRLOG0("Unimplemented fun CMLGraphics::");
	return false;
}

bool CMLGraphics::DrawTextClipOutlined(LPCTSTR szText, int nLen, LPRECT lpRect, const MLColor &clrText, const MLColor &clrBorder, UINT uFormat)
{
	ERRLOG0("Unimplemented fun CMLGraphics::");
	return false;
}

bool CMLGraphics::DrawText(LPCTSTR szText, int nLen, LPRECT lpRect, UINT uFormat)
{
	ERRLOG0("Unimplemented fun CMLGraphics::");
	return false;
}

bool CMLGraphics::GetTextExtentPoint32(LPCTSTR szText, int nLen, LPSIZE pSize)
{
	CGContextSetTextDrawingMode(m_context, kCGTextInvisible);

	CGPoint ptStart = CGContextGetTextPosition(m_context);
	CGContextShowText(m_context, szText, nLen);
	CGPoint ptEnd = CGContextGetTextPosition(m_context);

	pSize->cx = ptEnd.x - ptStart.x;
	pSize->cy = m_font->GetHeight();

	CGContextSetTextDrawingMode(m_context, kCGTextFill);
	
	return true;
}

void CMLGraphics::SetTextColor(const MLColor &color)
{
	CGContextSetRGBFillColor(m_context, color.r() / 255.0, color.g() / 255.0, color.b() / 255.0, color.getAlpha() / 255.0);
}

void CMLGraphics::SetBkMode(bool bTransparent)
{
	ERRLOG0("Unimplemented fun CMLGraphics::");
}

void CMLGraphics::SetFgColor(const MLColor &color)
{
	ERRLOG0("Unimplemented fun CMLGraphics::");
}

void CMLGraphics::SetBgColor(const MLColor &color)
{
	ERRLOG0("Unimplemented fun CMLGraphics::");
}

void CMLGraphics::SetPen(CMLPen &pen)
{
	ERRLOG0("Unimplemented fun CMLGraphics::");
}

bool CMLGraphics::GetClipBoundBox(CRect &rc)
{
	ERRLOG0("Unimplemented fun CMLGraphics::");
	return false;
}

bool CMLGraphics::SetClipBoundBox(const CRect &rc)
{
	ERRLOG0("Unimplemented fun CMLGraphics::");
	return false;
}

void CMLGraphics::RecoverClipBox()
{
	ERRLOG0("Unimplemented fun CMLGraphics::");
}

void CMLGraphics::DrawGraphics(IMLGraphicsBase *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc)
{
	ERRLOG0("Unimplemented fun CMLGraphics::");
}

void CMLGraphics::DrawGraphicsStretch(IMLGraphicsBase *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc, int widthsrc, int heightsrc)
{
	ERRLOG0("Unimplemented fun CMLGraphics::");
}

void CMLGraphics::DrawImage(CMLImage *pImage, int xdest, int ydest, int width, int height, int xsrc, int ysrc)
{
	ERRLOG0("Unimplemented fun CMLGraphics::");
}

void CMLGraphics::DrawImageStretch(CMLImage *pImage, int xdest, int ydest, int widthdest, int heightdest, int xsrc, int ysrc, int widthsrc, int heightsrc)
{
	ERRLOG0("Unimplemented fun CMLGraphics::");
}


void CMLGraphics::DrawGraphicsAlphaBlend(IMLGraphicsBase *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc, byte nAlpha, bool bPerPixelAlpha)
{
	ERRLOG0("Unimplemented fun CMLGraphics::");
}

void CMLGraphics::DrawGraphicsStretchAlphaBlend(IMLGraphicsBase *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc, int widthsrc, int heightsrc, byte nAlpha, bool bPerPixelAlpha)
{
	ERRLOG0("Unimplemented fun CMLGraphics::");
}

void CMLGraphics::DrawImageAlphaBlend(CMLImage *pImage, int xdest, int ydest, int width, int height, int xsrc, int ysrc, byte nAlpha, bool bPerPixelAlpha)
{
	ERRLOG0("Unimplemented fun CMLGraphics::");
}

void CMLGraphics::DrawImageStretchAlphaBlend(CMLImage *pImage, int xdest, int ydest, int widthdest, int heightdest, int xsrc, int ysrc, int widthsrc, int heightsrc, byte nAlpha, bool bPerPixelAlpha)
{
	ERRLOG0("Unimplemented fun CMLGraphics::");
}

void CMLGraphics::SetFont(CMLFont *font)
{
	IMLGraphicsBase::SetFont(font);

	CGContextSelectFont(m_context, "Helvetica", m_font->GetSize(), kCGEncodingMacRoman);
	// Next we set the text matrix to flip our text upside down. We do this because the context itself
	// is flipped upside down relative to the expected orientation for drawing text (much like the case for drawing Images & PDF).
	CGContextSetTextMatrix(m_context, CGAffineTransformMakeScale(1.0, -1.0));

	// CGContextSetFont(m_context, m_font->GetHandle());
	// CGContextSetFontSize(m_context, m_font->GetSize());
	CGContextSetTextDrawingMode(m_context, kCGTextFill);
}

/*
bool CMLGraphics::CreateMemDC(GdkDrawable *pDrawable, int width, int height)
{
	DeleteDC();

	m_nWidth = width;
	m_nHeight = height;

	m_pDrawable = gdk_pixmap_new(pDrawable, 
		width, height, gdk_drawable_get_depth(pDrawable));
	m_pGc = gdk_gc_new(pDrawable);

	return true;
}

void CMLGraphics::DeleteDC()
{
	if (m_pDrawable)
	{
		gdk_drawable_unref(m_pDrawable);
		m_pDrawable = NULL;
	}

	if (m_pGc)
	{
		gdk_gc_unref(m_pGc);
		m_pGc = NULL;
	}
}*/

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
