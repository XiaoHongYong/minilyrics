// GdiGraphics.h: interface for the CGdiGraphics class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GDIGRAPHICS_H__459134AA_966B_474C_B99D_61232BA41BDF__INCLUDED_)
#define AFX_GDIGRAPHICS_H__459134AA_966B_474C_B99D_61232BA41BDF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MLFont.h"

class CGdiFont
{
public:
	CGdiFont()
	{
		m_hFont = NULL;
	}
	~CGdiFont()
	{
		Destroy();
	}

	bool Create(IMLGraphicsBase *pGraph, LOGFONTA &logFont)
	{
		Destroy();

		m_hFont = CreateFontIndirectA(&logFont);

		return m_hFont != NULL;
	}

	bool Create(const CMLFont &font);

	void Destroy()
	{
		if (m_hFont)
		{
			DeleteObject(m_hFont);
			m_hFont = NULL;
		}
	}

	HFONT GetHandle() { return m_hFont; }

	bool IsValid() const { return m_hFont != NULL; }

protected:
	HFONT			m_hFont;

};

//
// 普通方式的画图
//
class CGdiGraphics : public IMLGraphicsBase
{
public:
	CGdiGraphics();
	~CGdiGraphics();

	virtual void Line(int x1, int y1, int x2, int y2);

	virtual void Rectangle(int x, int y, int width, int height);

	virtual void FillRect(int x, int y, int width, int height, CMLBrush *pBrush);

	virtual bool TextOut(int x, int y, LPCTSTR szText, int nLen);
	virtual bool DrawTextClip(LPCTSTR szText, int nLen, LPRECT lpRect);

	virtual bool TextOutOutlined(int x, int y, LPCTSTR szText, int nLen, const MLColor &clrText, const MLColor &clrBorder);
	virtual bool DrawTextClipOutlined(LPCTSTR szText, int nLen, LPRECT lpRect, const MLColor &clrText, const MLColor &clrBorder, UINT uFormat = DT_LEFT);

	virtual bool DrawText(LPCTSTR szText, int nLen, LPRECT lpRect, UINT uFormat = DT_SINGLELINE | DT_LEFT);
	virtual bool GetTextExtentPoint32(LPCTSTR szText, int nLen, LPSIZE pSize);

	virtual void SetTextColor(const MLColor &color)
	{
		::SetTextColor(m_hdc, color.get());
	}

	virtual void SetBkMode(bool bTransparent)
	{
		::SetBkMode(m_hdc, bTransparent ? TRANSPARENT : OPAQUE);
	}

	virtual void SetFgColor(const MLColor &color);
	virtual void SetBgColor(const MLColor &color);

	virtual void SetPen(CMLPen &pen);

	virtual bool GetClipBoundBox(CRect &rc);
	virtual bool SetClipBoundBox(const CRect &rc);

	virtual void RecoverClipBox();

	virtual void DrawGraphics(IMLGraphicsBase *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc);
	virtual void DrawGraphicsStretch(IMLGraphicsBase *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc, int widthsrc, int heightsrc);

	virtual void DrawImage(CMLImage *pImage, int xdest, int ydest, int width, int height, int xsrc, int ysrc);
	virtual void DrawImageStretch(CMLImage *pImage, int xdest, int ydest, int widthdest, int heightdest, int xsrc, int ysrc, int widthsrc, int heightsrc);

	virtual void DrawGraphicsAlphaBlend(IMLGraphicsBase *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc, byte nAlpha, bool bPerPixelAlpha);
	virtual void DrawGraphicsStretchAlphaBlend(IMLGraphicsBase *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc, int widthsrc, int heightsrc, byte nAlpha, bool bPerPixelAlpha);

	virtual void DrawImageAlphaBlend(CMLImage *pImage, int xdest, int ydest, int width, int height, int xsrc, int ysrc, byte nAlpha, bool bPerPixelAlpha);
	virtual void DrawImageStretchAlphaBlend(CMLImage *pImage, int xdest, int ydest, int widthdest, int heightdest, int xsrc, int ysrc, int widthsrc, int heightsrc, byte nAlpha, bool bPerPixelAlpha);

	virtual void SetFont(CMLFont *font);

	virtual void EnableResolveTextEncoding(bool bEnable);

public:
	bool Create(HDC hdc);

	virtual void Destroy();

protected:
	void SelectFont(HFONT hFont)
	{
		HFONT		hFontOld;

		hFontOld = (HFONT)SelectObject(m_hdc, hFont);
		if (!m_hFontOld)
			m_hFontOld = hFontOld;
	}

protected:
	CFontCollection<CGdiFont>		m_fontCollection;

	HFONT		m_hFontOld;
	HRGN		m_hRgnOld;
	int			m_nRgnOld;		// 0: no clip region, 1: have clip region, -1: unknow/error
	HPEN		m_hPenOld;
	HDC			m_hdcBk;

};

#endif // !defined(AFX_GDIGRAPHICS_H__459134AA_966B_474C_B99D_61232BA41BDF__INCLUDED_)
