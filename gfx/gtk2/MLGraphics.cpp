  // MLGraphics.cpp: implementation of the CMLGraphics class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "gfx.h"
#include "MLGraphics.h"

inline guint pixel_from_rgb(GdkVisual *visual, guchar r, guchar b, guchar g)
{
 	return ((r )   << visual->red_shift) |
 		((g ) << visual->green_shift) |
 		((b )  << visual->blue_shift);
// 	return ((r >> (16 - visual->red_prec))   << visual->red_shift) |
// 		((g >> (16 - visual->green_prec)) << visual->green_shift) |
// 		((b >> (16 - visual->blue_prec))  << visual->blue_shift);
}

inline void MLColorToGdkColor(GdkColor &gclr, const MLColor &clr, GdkDrawable *pDrawable)
{
	// GdkVisual	*visual = gdk_drawable_get_visual(pDrawable);
	GdkColormap	*clrMap;

	gclr.blue = clr.blue;
	gclr.red = clr.red;
	gclr.green = clr.green;
	// gclr.pixel = pixel_from_rgb(visual, clr.r(), clr.b(), clr.g());
	// gclr.pixel = pixel_from_rgb(visual, gclr.red, gclr.blue, gclr.green);
	gclr.pixel = 0;
	clrMap = gdk_drawable_get_colormap(pDrawable);
	gdk_colormap_alloc_color(clrMap, &gclr, true, true);
	// printf("r:%x, g: %x, b: %x, pixel: %X", gclr.red, gclr.green, gclr.blue, gclr.pixel);
}

inline void SetGdkColorPixel(GdkColor &clr, GdkDrawable *pDrawable)
{
	// GdkVisual	*visual = gdk_drawable_get_visual(pDrawable);
	GdkColormap	*clrMap;

	clr.pixel = 0;
	clrMap = gdk_drawable_get_colormap(pDrawable);
	gdk_colormap_alloc_color(clrMap, &clr, true, true);
	// clr.pixel = pixel_from_rgb(visual, clr.red / 257, clr.blue / 257, clr.green / 257);
	// clr.pixel = pixel_from_rgb(visual, clr.red, clr.blue, clr.green);
	// printf("r:%x, g: %x, b: %x, pixel: %X", clr.red, clr.green, clr.blue, clr.pixel);
}


/* Xlib image scaling... */

#define sign(x) ((x)>0 ? 1:-1)

static void XlibStretchHorizontal(long x1,long x2,long y1,long y2,
                                  long ymin,long ymax,
                                  long startColumn, long endColumn,
                                  long offsetX, long offsetY,
                                  GdkPixmap *aSrcImage, GdkPixmap *aDstImage, GdkGC *gc);

/**********************************************************
 XlibRectStretch enlarges or diminishes a source rectangle of a bitmap to
 a destination rectangle. The source rectangle is selected by the two
 points (xs1,ys1) and (xs2,ys2), and the destination rectangle by
 (xd1,yd1) and (xd2,yd2).

 Entry:
	xs1,ys1 - first point of source rectangle
	xs2,ys2 - second point of source rectangle
	xd1,yd1 - first point of destination rectangle
	xd2,yd2 - second point of destination rectangle
  offx, offy - offset to target
**********************************************************/
void
XlibRectStretch(uint32 srcWidth, uint32 srcHeight,
                uint32 dstWidth, uint32 dstHeight,
                uint32 dstOrigX, uint32 dstOrigY,
                uint32 aDX, uint32 aDY,
                uint32 aDWidth, uint32 aDHeight,
                GdkPixmap *aSrcImage, GdkPixmap *aDstImage,
                GdkGC *gc, GdkGC *copygc, uint32 aDepth)
{
  long dx,dy,e,d,dx2;
  short sx,sy;
  GdkPixmap *aTmpImage = 0;
  bool skipHorizontal=false, skipVertical=false;
  long startColumn, startRow, endColumn, endRow;
  long xs1, ys1, xs2, ys2, xd1, yd1, xd2, yd2;

  xs1 = ys1 = xd1 = yd1 = 0;
  xs2 = srcWidth-1;
  ys2 = srcHeight-1;
  xd2 = dstWidth-1;
  yd2 = dstHeight-1;

//  fprintf(stderr, "XRS %p (%ld %ld)-(%ld %ld) (%ld %ld)-(%ld %ld)\n",
//          (void *)aDstImage, xs1, ys1, xs2, ys2, xd1, yd1, xd2, yd2);
  
  startColumn = aDX-dstOrigX;
  startRow    = aDY-dstOrigY;
  endColumn   = aDX+aDWidth-dstOrigX;
  endRow      = aDY+aDHeight-dstOrigY;

//  fprintf(stderr, "startXY = %d %d  endXY = %d %d   %d x %d\n",
//          startColumn, startRow, endColumn, endRow,
//          endColumn-startColumn, endRow-startRow);

  long scaleStartY, scaleEndY;
  scaleStartY = startRow * (ys2-ys1+1) / (yd2-yd1+1);
  scaleEndY   = 1 + endRow * (ys2-ys1+1) / (yd2-yd1+1);

  if (xd2-xd1 == xs2-xs1) {
//    fprintf(stderr, "skipping horizontal\n");
    skipHorizontal = true;
    aTmpImage = aSrcImage;
    scaleStartY = 0;
    scaleEndY = ys2;
  }

  if (yd2-yd1 == ys2-ys1) {
//    fprintf(stderr, "skipping vertical\n");
    skipVertical = true;
    aTmpImage = aDstImage;
  }

  if (skipVertical && skipHorizontal) {
    gdk_draw_pixmap(aDstImage, gc, aSrcImage,
                    0, 0, srcWidth, srcHeight,
                    dstOrigX, dstOrigY);
    return;
  }

//  fprintf(stderr, "scaleY Start/End = %d %d\n", scaleStartY, scaleEndY);

  if (!skipHorizontal && !skipVertical) {
    aTmpImage = gdk_pixmap_new(NULL,
                               endColumn-startColumn,
                               scaleEndY-scaleStartY,
                               aDepth);
#ifdef MOZ_WIDGET_GTK2
    if (aDepth != 1)
      gdk_drawable_set_colormap(GDK_DRAWABLE(aTmpImage),
                                gdk_rgb_get_colormap());
#endif
  }
 
  dx = abs((int)(yd2-yd1));
  dy = abs((int)(ys2-ys1));
  sx = sign(yd2-yd1);
  sy = sign(ys2-ys1);
  e = dy-dx;
  dx2 = dx;
  dy += 1;
  if (!dx2) dx2=1;

  if (!skipHorizontal)
    XlibStretchHorizontal(xd1, xd2, xs1, xs2, scaleStartY, scaleEndY,
                          startColumn, endColumn,
                          (skipVertical?MAX(dstOrigX,0):0),(skipVertical?MAX(dstOrigY,0):0),
                          aSrcImage, aTmpImage, (skipVertical?gc:copygc));
  
  if (!skipVertical) {
    for (d=0; d<=dx; d++) {
      if ((yd1 >= startRow) && (yd1 <= endRow)) {
        gdk_draw_pixmap(aDstImage, gc, aTmpImage,
                        (skipHorizontal?startColumn:0), ys1-scaleStartY,
                        (dstOrigX>0?dstOrigX:0), dstOrigY+yd1,
                        endColumn-startColumn, 1);
      }
      while (e>=0) {
	      ys1 += sy;
	      e -= dx2;
      }
      yd1 += sx;
      e += dy;
    }
  }

  if (!skipHorizontal && !skipVertical)
    gdk_pixmap_unref(aTmpImage);
}

/**********************************************************
 Stretches a image horizontally by column replication/deletion.
 Used by XlibRectStretch.

 Entry:
	x1,x2 - x-coordinates of the destination line
	y1,y2 - x-coordinates of the source line
	ymin  - y-coordinate of top of stretch region
	ymax  - y-coordinate of bottom of stretch region
**********************************************************/
static void
XlibStretchHorizontal(long x1, long x2, long y1, long y2,
                      long ymin, long ymax,
                      long startColumn, long endColumn,
                      long offsetX, long offsetY,
                      GdkPixmap *aSrcImage, GdkPixmap *aDstImage, GdkGC *gc)
{
  long dx,dy,e,d,dx2;
  short sx,sy;

  dx = abs((int)(x2-x1));
  dy = abs((int)(y2-y1));
  sx = sign(x2-x1);
  sy = sign(y2-y1);
  e = dy-dx;
  dx2 = dx;
  dy += 1;
  if (!dx2) dx2=1;
  for (d = 0; d <= dx; d++) {
    if ((x1 >= startColumn) && (x1 <= endColumn))
	{
		gdk_draw_pixmap(aDstImage, gc, aSrcImage,
                      y1, ymin, x1-startColumn+offsetX, 0+offsetY,
                      1, ymax-ymin);
    }
    while (e>=0) {
      y1 += sy;
      e -= dx2;
    }
    x1 += sx;
    e += dy;
  }
}

#undef sign

// condition: widthdest > widthsrc
void Gen1DimensionMap(int xsrc, int xdest, int widthsrc, int widthdest,vector<int> &vSrc, vector<int> &vDest)
{
	// s: source
	// d: dest
	float		sx, dx;		// source x, dest x;
	float		sx0, dx0;
	float		cxdiv2;
	int			sxlast, dxlast;
	int			isx;

	sx = sx0 = xsrc;
	dx = dx0 = xdest;

	sxlast = xsrc;
	dxlast = xdest;

	vSrc.push_back(sxlast);
	vDest.push_back(dxlast);

	cxdiv2 = (float)widthsrc / widthdest / 2;
	while (dx < xdest + widthdest)
	{
		// combine area that could be map together
		// sx0 --> sx
		// dx0 --> dx
		dx++;
		sx += cxdiv2;
		isx = (int)sx;
		if (sx > isx)
			isx++;
		sx += cxdiv2;

		if (sxlast + 1 == isx)
		{
			sxlast = isx;
			dxlast = (int)dx;
		}
		else
		{
			vSrc.push_back(sxlast);
			vDest.push_back(dxlast);

			sxlast = (int)isx;
			dxlast = (int)dx;
		}
	}
	vSrc.push_back(sxlast);
	vDest.push_back(dxlast);
}

void XlibStretchBlt(int xdest, int ydest,
					int widthdest, int heightdest,
					int xsrc, int ysrc,
					int widthsrc, int heightsrc,
					GdkPixmap *srcImage, GdkPixmap *dstImage, GdkGC *gc)
{
	assert(srcImage);
	assert(dstImage);
	if (widthdest == widthsrc && heightdest == heightsrc)
	{
		gdk_draw_pixmap(dstImage, gc, srcImage,
						xsrc, ysrc, xdest, ydest, widthsrc, heightsrc);
		return;
	}

	// vxSrc, vxDest, vySrc, vyDest, saves the small rect that to copy...
	vector<int>	vxSrc, vxDest, vySrc, vyDest;

	Gen1DimensionMap(xsrc, xdest, widthsrc, widthdest, vxSrc, vxDest);

	Gen1DimensionMap(ysrc, ydest, heightsrc, heightdest, vySrc, vyDest);

	assert(vxSrc.size() == vxDest.size());
	assert(vySrc.size() == vyDest.size());
	assert(vxSrc.size() >= 1);
	assert(vySrc.size() >= 1);

// 	bool	bxEnlarge, byEnlarge;
// 
// 	if (widthdest > widthsrc)
// 		bxEnlarge = true;
// 	else
// 		bxEnlarge = false;
// 
// 	if (heightdest > heightsrc)
// 		byEnlarge = true;
// 	else
// 		byEnlarge = false;

	int		i, k;
	int		sx0, sx, dx0, dx;
	sx0 = vxSrc[0];
	dx0 = vxDest[0];
	for (i = 1; i < vxDest.size(); i++)
	{
		int		sy0, sy, dy0, dy;

		sx = vxSrc[i];
		dx = vxDest[i];

		sy0 = vySrc[0];
		dy0 = vyDest[0];
		for (k = 1; k < vyDest.size(); k++)
		{
			sy = vySrc[k];
			dy = vyDest[k];

			gdk_draw_pixmap(dstImage, gc, srcImage,
						sx0, sy0, dx0, dy0, dx - dx0, dy - dy0);

			sy0 = sy;
			dy0 = dy;
		}
		sx0 = sx;
		dx0 = dx;
	}
}



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMLGraphics::CMLGraphics()
{
	m_GraphicsType = MGT_GTK2;

	m_pGc = NULL;
	m_pDrawable = NULL;
	m_bRecoverClipBox = false;
	m_bTransparent = false;

	m_nWidth = m_nHeight = 0;
}

CMLGraphics::~CMLGraphics()
{
	DeleteDC();
}

void CMLGraphics::Line(int x1, int y1, int x2, int y2)
{
	gdk_draw_line(m_pDrawable, m_pGc, x1, y1, x2, y2);
}

void CMLGraphics::Rectangle(int x, int y, int width, int height)
{
	gdk_draw_rectangle(m_pDrawable, m_pGc, false, x, y, width, height);
}

void CMLGraphics::FillRect(int x, int y, int width, int height, CMLBrush *pBrush)
{
	GdkGCValues		oldValue;

	SetGdkColorPixel(pBrush->m_color, m_pDrawable);

	gdk_gc_get_values(m_pGc, &oldValue);

	gdk_gc_set_foreground(m_pGc, &pBrush->m_color);

	gdk_draw_rectangle(m_pDrawable, m_pGc, true, x, y, width, height);

	gdk_gc_set_foreground(m_pGc, &oldValue.foreground);
}

bool CMLGraphics::TextOut(int x, int y, LPCTSTR szText, int nLen)
{
	// linux 中，必须给出长度
	if (nLen == -1)
		nLen = strlen(szText);

	assert(m_font->GetHandle());
	gdk_draw_text(m_pDrawable,
		m_font->GetHandle(),
		m_pGc,
		x, y + m_font->GetHeight(), szText, nLen);

	// MLDebug("gdk_draw_text, x: %d, y: %d, height: %d\n", x, y + m_font->GetHeight(), m_font->GetHeight());

	return TRUE;
}

bool CMLGraphics::DrawTextClip(LPCTSTR szText, int nLen, LPRECT lpRect)
{
	return TextOut(lpRect->left, lpRect->top, szText, nLen);
}

bool CMLGraphics::TextOutOutlined(int x, int y, LPCTSTR szText, int nLen, const MLColor &clrText, const MLColor &clrBorder)
{
	// linux 中，必须给出长度
	if (nLen == -1)
		nLen = strlen(szText);

	assert(m_font->GetHandle());

	// draw border
	SetFgColor(clrBorder);
	gdk_draw_text(m_pDrawable, m_font->GetHandle(), m_pGc, x + 1, y + m_font->GetHeight(), szText, nLen);
	gdk_draw_text(m_pDrawable, m_font->GetHandle(), m_pGc, x - 1, y + m_font->GetHeight(), szText, nLen);
	gdk_draw_text(m_pDrawable, m_font->GetHandle(), m_pGc, x, y + 1 + m_font->GetHeight(), szText, nLen);
	gdk_draw_text(m_pDrawable, m_font->GetHandle(), m_pGc, x, y - 1 + m_font->GetHeight(), szText, nLen);

	// draw inner
	SetFgColor(clrText);
	gdk_draw_text(m_pDrawable, m_font->GetHandle(), m_pGc, x, y + m_font->GetHeight(), szText, nLen);

	return TRUE;
}

bool CMLGraphics::DrawTextClipOutlined(LPCTSTR szText, int nLen, LPRECT lpRect, const MLColor &clrText, const MLColor &clrBorder, UINT uFormat)
{
	TextOutOutlined(lpRect->left, lpRect->top, szText, nLen, clrText, clrBorder);

	return true;
}

bool CMLGraphics::DrawText(LPCTSTR szText, int nLen, LPRECT lpRect, UINT uFormat)
{
	return TextOut(lpRect->left, lpRect->top, szText, nLen);
}

bool CMLGraphics::GetTextExtentPoint32(LPCTSTR szText, int nLen, LPSIZE pSize)
{
	assert(m_font->GetHandle());
	if (nLen == -1)
		pSize->cx = gdk_string_width(m_font->GetHandle(), szText);
	else
		pSize->cx = gdk_text_width(m_font->GetHandle(), szText, nLen);

	pSize->cy = m_font->GetHeight();

	return TRUE;
}

void CMLGraphics::SetTextColor(const MLColor &color)
{
	SetFgColor(color);
}

void CMLGraphics::SetBkMode(bool bTransparent)
{
	m_bTransparent = bTransparent;
}

void CMLGraphics::SetFgColor(const MLColor &color)
{
	GdkColor	clr;

	MLColorToGdkColor(clr, color, m_pDrawable);

	gdk_gc_set_rgb_fg_color(m_pGc, &clr);
}

void CMLGraphics::SetBgColor(const MLColor &color)
{
	GdkColor	clr;

	MLColorToGdkColor(clr, color, m_pDrawable);

	gdk_gc_set_rgb_bg_color(m_pGc, &clr);
}

void CMLGraphics::SetPen(CMLPen &pen)
{
	gdk_gc_set_line_attributes(m_pGc, pen.m_nWidth, pen.m_LineStyle, pen.m_CapStyle, pen.m_JionStyle);

	SetGdkColorPixel(pen.m_clr, m_pDrawable);

	gdk_gc_set_rgb_fg_color(m_pGc, &pen.m_clr);
}

bool CMLGraphics::GetClipBoundBox(CRect &rc)
{
	GdkRectangle	grc;
	GdkRegion	*pRegion = gdk_drawable_get_clip_region(m_pDrawable);
	gdk_region_get_clipbox(pRegion, &grc);

	rc.left = grc.x;
	rc.top = grc.y;
	rc.right = grc.x + grc.width;
	rc.bottom = grc.y + grc.height;

	return true;
}

bool CMLGraphics::SetClipBoundBox(const CRect &rc)
{
	GdkRectangle	grc;

	if (!m_bRecoverClipBox)
	{
		m_bRecoverClipBox = true;
		GdkRegion	*pRegion = gdk_drawable_get_clip_region(m_pDrawable);
		gdk_region_get_clipbox(pRegion, &m_rcClipBoxOrg);
	}

	grc.x = rc.left;
	grc.y = rc.top;
	grc.width = rc.Width();
	grc.height = rc.Height();

	gdk_gc_set_clip_rectangle(m_pGc, &grc);

	return false;
}

void CMLGraphics::RecoverClipBox()
{
	if (m_bRecoverClipBox)
	{
		m_bRecoverClipBox = false;

		gdk_gc_set_clip_rectangle(m_pGc, &m_rcClipBoxOrg);
	}
}

void CMLGraphics::DrawGraphics(IMLGraphicsBase *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc)
{
	CMLGraphics		*pGtk2Src = (CMLGraphics *)pSrc;
	assert(pGtk2Src->m_pDrawable);
	gdk_draw_drawable(m_pDrawable, m_pGc, pGtk2Src->m_pDrawable, xsrc, ysrc, xdest, ydest, width, height);
}

void CMLGraphics::DrawGraphicsStretch(IMLGraphicsBase *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc, int widthsrc, int heightsrc)
{
	ERRLOG0("Unimplemented fun: CMLGraphics::DrawGraphicsStretch");
// 	assert(pImage->IsValid());
// 	CMLGraphics		*pSrcGraph = (CMLGraphics *)pSrc;
// 
// 	XlibRectStretch(widthsrc, heightsrc, widthdest, heightdest,
// 		xsrc, ysrc, xdest, ydest,
// 		widthdest, heightdest,
// 		pSrcGraph->m_pDrawable,
// 		this->m_pDrawable, m_pGc, m_pGc, pImage->GetDepth());
}

void CMLGraphics::DrawImage(CMLImage *pImage, int xdest, int ydest, int width, int height, int xsrc, int ysrc)
{
	if (!pImage->m_pPixmap)
		assert(pImage->m_pPixmap);
	gdk_draw_drawable(m_pDrawable, m_pGc, pImage->m_pPixmap, xsrc, ysrc, xdest, ydest, width, height);
}

void CMLGraphics::DrawImageStretch(CMLImage *pImage, int xdest, int ydest, int widthdest, int heightdest, int xsrc, int ysrc, int widthsrc, int heightsrc)
{
	assert(pImage->IsValid());
	assert(pImage->m_pPixmap);
	XlibStretchBlt(xdest, ydest, widthdest, heightdest, xsrc, ysrc, widthsrc, heightsrc, 
 		pImage->m_pPixmap,
 		this->m_pDrawable, m_pGc);
// 	XlibRectStretch(widthsrc, heightsrc, widthdest, heightdest,
// 		0, 0, xdest, ydest,
// 		widthdest, heightdest,
// // 		xsrc, ysrc, xdest, ydest,
// // 		widthdest, heightdest,
// 		pImage->m_pPixmap,
// 		this->m_pDrawable, m_pGc, m_pGc, pImage->GetDepth());
	// gdk_draw_drawable(m_pDrawable, m_pGc, pImage->m_pPixmap, xsrc, ysrc, xdest, ydest, width, height);
}


void CMLGraphics::DrawGraphicsAlphaBlend(IMLGraphicsBase *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc, byte nAlpha, bool bPerPixelAlpha)
{
	DrawGraphics(pSrc, xdest, ydest, width, height, xsrc, ysrc);
}

void CMLGraphics::DrawGraphicsStretchAlphaBlend(IMLGraphicsBase *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc, int widthsrc, int heightsrc, byte nAlpha, bool bPerPixelAlpha)
{
	DrawGraphicsStretch(pSrc, xdest, ydest, width, height, xsrc, ysrc, widthsrc, heightsrc);
}

void CMLGraphics::DrawImageAlphaBlend(CMLImage *pImage, int xdest, int ydest, int width, int height, int xsrc, int ysrc, byte nAlpha, bool bPerPixelAlpha)
{
	DrawImage(pImage, xdest, ydest, width, height, xsrc, ysrc);
}

void CMLGraphics::DrawImageStretchAlphaBlend(CMLImage *pImage, int xdest, int ydest, int widthdest, int heightdest, int xsrc, int ysrc, int widthsrc, int heightsrc, byte nAlpha, bool bPerPixelAlpha)
{
	DrawImageStretch(pImage, xdest, ydest, widthdest, heightdest, xsrc, ysrc, widthsrc, heightsrc);
}

void CMLGraphics::SetFont(CMLFont *font)
{
	IMLGraphicsBase::SetFont(font);
	if (!font->GetHandle())
		assert(font->GetHandle());

	gdk_gc_set_font(m_pGc, font->GetHandle());
}

void CMLGraphics::Init(GdkDrawable *pDrawable, GdkGC *pGC)
{
	m_pDrawable = pDrawable;
	gdk_drawable_ref(m_pDrawable);

	if (pGC)
	{
		m_pGc = pGC;
		gdk_gc_ref(m_pGc);
	}
	else
		m_pGc = gdk_gc_new(m_pDrawable);
}

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
}

//////////////////////////////////////////////////////////////////////////
// private section for gtk+ implementation
CGtkContextObj		g_GtkContextObj;

CGtkContextObj::CGtkContextObj()
{
	m_window = NULL;
}
CGtkContextObj::~CGtkContextObj()
{
}

GtkWidget *CGtkContextObj::GetGtkWindow()
{
//	if (m_window)
//		return m_window;
//    m_window = gtk_window_new(GTK_WINDOW_POPUP);
//
//	gtk_widget_show(m_window);
//
	assert(m_window);
	return m_window;
}

void CGtkContextObj::SetGtkWindow(GtkWidget *window)
{
	printf("SetGtkWindow: 0x%X", window);
	if (!m_window)
		m_window = window;
}

