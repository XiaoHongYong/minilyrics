// MLGraphics.h: interface for the CMLGraphics class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GTKGRAPHICS_H__D13EC141_707E_4988_B61C_99F775A880A6__INCLUDED_)
#define AFX_GTKGRAPHICS_H__D13EC141_707E_4988_B61C_99F775A880A6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMLImage;
class CMLFont;
class CMLGraphics;
class CMLBrush;
class CMLPen;

class CBaseWnd;

#ifndef WIN32
#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

#define GetRValue(rgb)      ((BYTE)(rgb))
#define GetGValue(rgb)      ((BYTE)(((WORD)(rgb)) >> 8))
#define GetBValue(rgb)      ((BYTE)((rgb)>>16))
#endif

class CMLGraphics : public IMLGraphicsBase
{
public:
	CMLGraphics();
	virtual ~CMLGraphics();

	virtual void Line(int x1, int y1, int x2, int y2);

	virtual void Rectangle(int x, int y, int width, int height);

	virtual void FillRect(int x, int y, int width, int height, CMLBrush *pBrush);

	virtual bool TextOut(int x, int y, LPCTSTR szText, int nLen);
	virtual bool DrawTextClip(LPCTSTR szText, int nLen, LPRECT lpRect);

	virtual bool TextOutOutlined(int x, int y, LPCTSTR szText, int nLen, const MLColor &clrText, const MLColor &clrBorder);
	virtual bool DrawTextClipOutlined(LPCTSTR szText, int nLen, LPRECT lpRect, const MLColor &clrText, const MLColor &clrBorder, UINT uFormat = DT_LEFT);

	virtual bool DrawText(LPCTSTR szText, int nLen, LPRECT lpRect, UINT uFormat = DT_SINGLELINE | DT_LEFT);

	virtual bool GetTextExtentPoint32(LPCTSTR szText, int nLen, LPSIZE pSize);

	virtual void SetTextColor(const MLColor &color);

	virtual void SetBkMode(bool bTransparent);

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

public:
	// for linux gtk2
	void Init(GdkDrawable *pDrawable, GdkGC *pGC = NULL);

	// for memdc.
	bool CreateMemDC(GdkDrawable *pDrawable, int width, int height);
	int Width() const { return m_nWidth; }
	int Height() const { return m_nHeight; }

	void DeleteDC();

	GdkDrawable *GetHandle() { return m_pDrawable; }
	GdkDrawable *GetDrawable() { return m_pDrawable; }
	GdkGC *GetGC() { return m_pGc; }

protected:
	bool			m_bRecoverClipBox;
	GdkRectangle	m_rcClipBoxOrg;
	bool			m_bTransparent;

	GdkGC			*m_pGc;
	GdkDrawable		*m_pDrawable;
	short			m_nWidth;
	short			m_nHeight;

};

//////////////////////////////////////////////////////////////////////////
// private section for gtk+ implementation
class CGtkContextObj
{
public:
	CGtkContextObj();
	~CGtkContextObj();

	GtkWidget *GetGtkWindow();

	void SetGtkWindow(GtkWidget *window);

protected:
	GtkWidget		*m_window;

};

extern CGtkContextObj		g_GtkContextObj;

#endif // !defined(AFX_GTKGRAPHICS_H__D13EC141_707E_4988_B61C_99F775A880A6__INCLUDED_)
