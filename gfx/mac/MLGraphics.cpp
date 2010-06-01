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
}

void CMLGraphics::Rectangle(int x, int y, int width, int height)
{
}

void CMLGraphics::FillRect(int x, int y, int width, int height, CMLBrush *pBrush)
{
}

bool CMLGraphics::TextOut(int x, int y, LPCTSTR szText, int nLen)
{
	return TRUE;
}

bool CMLGraphics::DrawTextClip(LPCTSTR szText, int nLen, LPRECT lpRect)
{
	return false;
}

bool CMLGraphics::TextOutOutlined(int x, int y, LPCTSTR szText, int nLen, const MLColor &clrText, const MLColor &clrBorder)
{
	return false;
}

bool CMLGraphics::DrawTextClipOutlined(LPCTSTR szText, int nLen, LPRECT lpRect, const MLColor &clrText, const MLColor &clrBorder, UINT uFormat)
{
	return false;
}

bool CMLGraphics::DrawText(LPCTSTR szText, int nLen, LPRECT lpRect, UINT uFormat)
{
	return false;
}

bool CMLGraphics::GetTextExtentPoint32(LPCTSTR szText, int nLen, LPSIZE pSize)
{
	return false;
}

void CMLGraphics::SetTextColor(const MLColor &color)
{
}

void CMLGraphics::SetBkMode(bool bTransparent)
{
}

void CMLGraphics::SetFgColor(const MLColor &color)
{
}

void CMLGraphics::SetBgColor(const MLColor &color)
{
}

void CMLGraphics::SetPen(CMLPen &pen)
{
}

bool CMLGraphics::GetClipBoundBox(CRect &rc)
{
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
	ERRLOG0("Unimplemented fun CMLGraphics::");
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

